#pragma once

#include "CoreMinimal.h"
#include "Logging/DriveLog.h"

struct FSimulationClock
{
public:
	// Metricas y diagnostico
	struct FFrameMetrics
	{
		// Input del frame
		double RealDeltaSeconds = 0.0;
		double ScaledDeltaSeconds = 0.0;

		// Estado
		bool bPaused = false;
		float TimeScale = 1.0f;

		// Fixed-step 
		int32 RawPendingSteps = 0;
		int32 ExecutedSteps = 0;
		int32 DroppedSteps = 0;
		bool bBacklogClamped = false;

		// Backlog
		double AccumulatorSeconds = 0.0;
		double FixedDeltaSeconds = 0.0;
		int32 MaxFixedStepsPerFrame = 0;
	};

	struct FDiagnostics
	{
		FFrameMetrics LastFrame;

		int64 TotalFrames = 0;
		int64 TotalPausedFrames = 0;

		int64 TotalFixedStepsConsumed = 0;
		int64 TotalDroppedSteps = 0;
		int64 TotalClampedFrames = 0;
	};

	const FDiagnostics& GetDiagnostics() const { return Diagnostics; }
	void ResetDiagnostics() { Diagnostics = FDiagnostics(); }

	void SetDiagnosticsVerboseLogsEnabled(bool bEnabled) { bVerboseDiagnosticsLogs = bEnabled; }
	bool AreDiagnosticsVerboseLogsEnabled() const { return bVerboseDiagnosticsLogs; }


	// API de configuración
	void Initialize(float InFixedDeltaTimeSeconds, float InDefaultTimeScale, int32 InMaxFixedStepsPerFrame)
	{
		DefaultFixedDeltaTimeSeconds = FMath::Max(0.0001f, InFixedDeltaTimeSeconds);
		DefaultTimeScale = FMath::Max(0.0f, InDefaultTimeScale);

		FixedDeltaTimeSeconds = DefaultFixedDeltaTimeSeconds;
		TimeScale = DefaultTimeScale;
		MaxFixedStepsPerFrame = FMath::Max(1, InMaxFixedStepsPerFrame);

		ResetTemporalState(/*bResetScaleToDefault*/true);
		ResetDiagnostics();
	}

	void ResetTemporalState(bool bResetScaleToDefault)
	{
		RealTimeSeconds = 0.0;
		LastRealDeltaSeconds = 0.0;

		SimTimeSeconds = 0.0;
		LastSimDeltaSeconds = 0.0;

		AccumulatedRealSeconds = 0.0;

		FixedStepCount = 0;
		bPaused = false;

		if (bResetScaleToDefault)
		{
			TimeScale = DefaultTimeScale;
		}
	}

	void Reset()
	{
		ResetTemporalState(/*bResetScaleToDefault*/false);
	}

	void SetPaused(bool bInPaused)
	{
		if (bPaused == bInPaused)
		{
			return;
		}

		bPaused = bInPaused;
		
		DRIVE_LOG_CLOCK(Log, TEXT("[DRIVE][Clock] Paused -> %s"), bPaused ? TEXT("true") : TEXT("false"));

		if (bPaused)
		{
			LastSimDeltaSeconds = 0.0;
		}
	}

	bool IsPaused() const { return bPaused; }

	void Pause()
	{
		SetPaused(true);
	}

	void Resume()
	{
		SetPaused(false);
	}

	void SetTimeScale(float InTimeScale)
	{
		const float NewScale = FMath::Max(0.0f, InTimeScale);
		if (FMath::IsNearlyEqual(TimeScale, NewScale, KINDA_SMALL_NUMBER))
		{
			return;
		}

		TimeScale = NewScale;
		DRIVE_LOG_CLOCK(Log, TEXT("[DRIVE][Clock] TimeScale -> %.4f"), TimeScale);
	}

	float GetTimeScale() const { return TimeScale; }

	float GetFixedDeltaTimeSeconds() const { return FixedDeltaTimeSeconds; }
	int32 GetMaxFixedStepsPerFrame() const { return MaxFixedStepsPerFrame; }

	double GetRealTimeSeconds() const { return RealTimeSeconds; }
	double GetSimTimeSeconds() const { return SimTimeSeconds; }
	double GetAccumulatedRealSeconds() const { return AccumulatedRealSeconds; }

	float GetLastRealDeltaSeconds() const { return static_cast<float>(LastRealDeltaSeconds); }
	float GetLastSimDeltaSeconds() const { return static_cast<float>(LastSimDeltaSeconds); }

	uint64 GetFixedStepCount() const { return static_cast<uint64>(FixedStepCount); }

	void TickRealTime(float InDeltaRealSeconds)
	{
		Diagnostics.TotalFrames++;

		const double RealDt = static_cast<double>(FMath::Max(0.0f, InDeltaRealSeconds));
		LastRealDeltaSeconds = RealDt;
		RealTimeSeconds += RealDt;

		LastScaledDelta = RealDt * static_cast<double>(TimeScale);

		// Preparar métricas del frame
		FFrameMetrics Frame;
		Frame.RealDeltaSeconds = RealDt;
		Frame.ScaledDeltaSeconds = LastScaledDelta;
		Frame.bPaused = bPaused;
		Frame.TimeScale = TimeScale;
		Frame.FixedDeltaSeconds = FixedDeltaTimeSeconds;
		Frame.MaxFixedStepsPerFrame = MaxFixedStepsPerFrame;

		if (bPaused)
		{
			Diagnostics.TotalPausedFrames++;

			LastSimDeltaSeconds = 0.0;
			LastRawSteps = 0;
			LastStepsExecuted = 0;
			LastDroppedSteps = 0;

			Frame.RawPendingSteps = 0;
			Frame.ExecutedSteps = 0;
			Frame.DroppedSteps = 0;
			Frame.bBacklogClamped = false;
			Frame.AccumulatorSeconds = AccumulatedRealSeconds;

			Diagnostics.LastFrame = Frame;

			if (bVerboseDiagnosticsLogs)
			{
				DRIVE_LOG_CLOCK(VeryVerbose, TEXT("[DRIVE][Clock] Tick paused | RealDt=%.6f Acc=%.6f"), RealDt, AccumulatedRealSeconds);
			}
			return;
		}

		AccumulatedRealSeconds += RealDt;

		const double FixedDt = static_cast<double>(FixedDeltaTimeSeconds);
		const double MaxBacklog = static_cast<double>(MaxFixedStepsPerFrame) * FixedDt;

		const double Epsilon = 1e-9;
		const int32 RawSteps = (FixedDt > 0.0) ? static_cast<int32>(FMath::FloorToDouble((AccumulatedRealSeconds + Epsilon) / FixedDt)): 0;

		const bool bWasBacklogClamped = (AccumulatedRealSeconds > MaxBacklog);
		if (bWasBacklogClamped)
		{
			AccumulatedRealSeconds = MaxBacklog;
		}

		const int32 StepsAfterClamp = (FixedDt > 0.0) ? static_cast<int32>(FMath::FloorToDouble((AccumulatedRealSeconds + Epsilon) / FixedDt)): 0;

		LastRawSteps = RawSteps;
		LastStepsExecuted = FMath::Clamp(StepsAfterClamp, 0, MaxFixedStepsPerFrame);
		LastDroppedSteps = FMath::Max(0, RawSteps - LastStepsExecuted);

		// Métricas del frame
		Frame.RawPendingSteps = LastRawSteps;
		Frame.ExecutedSteps = LastStepsExecuted;
		Frame.DroppedSteps = LastDroppedSteps;
		Frame.bBacklogClamped = bWasBacklogClamped;
		Frame.AccumulatorSeconds = AccumulatedRealSeconds;

		Diagnostics.LastFrame = Frame;

		// Contadores acumulados + hitch/clamp
		if (bWasBacklogClamped || LastDroppedSteps > 0)
		{
			Diagnostics.TotalClampedFrames++;
			Diagnostics.TotalDroppedSteps += LastDroppedSteps;

			const double Now = FPlatformTime::Seconds();
			const double Cooldown = 1.0;

			if ((Now - LastHitchLogWallSeconds) >= Cooldown)
			{
				LastHitchLogWallSeconds = Now;

				DRIVE_LOG_CLOCK(Log, TEXT("[DRIVE][Clock] Hitch/Clamp | Raw=%d Exec=%d Dropped=%d Paused=%d TimeScale=%.3f Acc=%.6f FixedDt=%.6f MaxSteps=%d"),
					LastRawSteps, LastStepsExecuted, LastDroppedSteps, bPaused ? 1 : 0, TimeScale, AccumulatedRealSeconds, FixedDeltaTimeSeconds, MaxFixedStepsPerFrame);
			}

		}
		else if (bVerboseDiagnosticsLogs && (LastStepsExecuted > 0))
		{
			DRIVE_LOG_CLOCK(Log, TEXT("[DRIVE][Clock] Tick | Raw=%d Exec=%d Acc=%.6f FixedDt=%.6f TimeScale=%.3f"),
				LastRawSteps, LastStepsExecuted, AccumulatedRealSeconds, FixedDeltaTimeSeconds, TimeScale);
		}
	}

	int32 GetPendingFixedSteps() const
	{
		const double FixedDt = static_cast<double>(FixedDeltaTimeSeconds);
		if (FixedDt <= 0.0)
		{
			return 0;
		}

		const double Epsilon = 1e-9;
		const int32 Steps = static_cast<int32>(FMath::FloorToDouble((AccumulatedRealSeconds + Epsilon) / FixedDt));
		return FMath::Clamp(Steps, 0, MaxFixedStepsPerFrame);
	}

	bool ConsumeFixedStep()
	{
		const double FixedDt = static_cast<double>(FixedDeltaTimeSeconds);
		if (FixedDt <= 0.0)
		{
			return false;
		}

		if (AccumulatedRealSeconds + 1e-9 < FixedDt)
		{
			return false;
		}

		AccumulatedRealSeconds -= FixedDt;
		if (AccumulatedRealSeconds < 0.0)
		{
			AccumulatedRealSeconds = 0.0;
		}

		++FixedStepCount;

		const double SimStepDt = FixedDt * static_cast<double>(TimeScale);
		LastSimDeltaSeconds = SimStepDt;
		SimTimeSeconds += SimStepDt;

		Diagnostics.TotalFixedStepsConsumed++;
		return true;
	}

	float GetInterpolationAlpha() const
	{
		if (FixedDeltaTimeSeconds <= 0.0f) return 0.0f;
		return FMath::Clamp(static_cast<float>(AccumulatedRealSeconds / static_cast<double>(FixedDeltaTimeSeconds)), 0.0f, 1.0f);
	}

	int32 LastRawSteps = 0;        
	int32 LastStepsExecuted = 0;  
	int32 LastDroppedSteps = 0;    
	double LastScaledDelta = 0.0;  

	double GetAccumulatorSeconds() const { return AccumulatedRealSeconds; }
	int32 GetLastRawSteps() const { return LastRawSteps; }
	int32 GetLastStepsExecuted() const { return LastStepsExecuted; }
	int32 GetLastDroppedSteps() const { return LastDroppedSteps; }
	double GetLastScaledDelta() const { return LastScaledDelta; }


private:
	// Config
	float FixedDeltaTimeSeconds = 1.0f / 60.0f;
	float TimeScale = 1.0f;
	int32 MaxFixedStepsPerFrame = 8;

	// Defaults
	float DefaultFixedDeltaTimeSeconds = 1.0f / 60.0f;
	float DefaultTimeScale = 1.0f;

	bool bPaused = false;

	// Runtime
	int64 FixedStepCount = 0;

	// Tiempo real
	double RealTimeSeconds = 0.0;
	double LastRealDeltaSeconds = 0.0;

	// Tiempo simulado
	double SimTimeSeconds = 0.0;
	double LastSimDeltaSeconds = 0.0;

	// Backlog para scheduling
	double AccumulatedRealSeconds = 0.0;

	FDiagnostics Diagnostics;
	bool bVerboseDiagnosticsLogs = false;

	double LastHitchLogWallSeconds = 0.0;
	double MaxRealDeltaSeconds = 0.05; 
};