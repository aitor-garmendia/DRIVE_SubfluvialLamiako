#include "Tests/ADriveSessionTestHarness.h"
#include "UWorldSimulationSubsystem.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Logging/DriveLog.h"

ADriveSessionTestHarness::ADriveSessionTestHarness()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADriveSessionTestHarness::BeginPlay()
{
	Super::BeginPlay();

	RunFullTest();
}

void ADriveSessionTestHarness::RunFullTest()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		DRIVE_LOG_TEST(Error, TEXT("[DRIVE TEST] World is null"));
		return;
	}

	UWorldSimulationSubsystem* Subsystem = World->GetSubsystem<UWorldSimulationSubsystem>();
	if (!Subsystem)
	{
		DRIVE_LOG_TEST(Error, TEXT("[DRIVE TEST] Subsystem not found"));
		return;
	}

	DRIVE_LOG_TEST(Log, TEXT("==============================================="));
	DRIVE_LOG_TEST(Log, TEXT("        STARTING DRIVE 2B SESSION TEST        "));
	DRIVE_LOG_TEST(Log, TEXT("==============================================="));

	// 1 Initialize
	DRIVE_LOG_TEST(Log, TEXT("[TEST] InitializeSimulation"));
	Subsystem->BP_InitializeSimulation(GlobalConfig);
	LogState(TEXT("After Initialize"), Subsystem);

	// 2 Load Scenario
	if (ScenarioAsset)
	{
		DRIVE_LOG_TEST(Log, TEXT("[TEST] LoadScenario"));
		bool bLoaded = Subsystem->BP_TryLoadScenario(ScenarioAsset);
		DRIVE_LOG_TEST(Log, TEXT("[TEST] Load result: %s"), bLoaded ? TEXT("SUCCESS") : TEXT("FAIL"));
		LogState(TEXT("After Load"), Subsystem);
	}
	else
	{
		DRIVE_LOG_TEST(Log, TEXT("[TEST] No ScenarioAsset assigned"));
	}

	// 3 Start
	DRIVE_LOG_TEST(Log, TEXT("[TEST] StartSimulation"));
	bool bStarted = Subsystem->BP_TryStartSimulation();
	DRIVE_LOG_TEST(Log, TEXT("[TEST] Start result: %s"), bStarted ? TEXT("SUCCESS") : TEXT("FAIL"));
	LogState(TEXT("After Start"), Subsystem);

	// 4 Pause
	DRIVE_LOG_TEST(Log, TEXT("[TEST] PauseSimulation"));
	Subsystem->BP_PauseSimulation();
	LogState(TEXT("After Pause"), Subsystem);

	// 5 Step once (5 steps)
	DRIVE_LOG_TEST(Log, TEXT("[TEST] StepOnce x5"));
	Subsystem->BP_StepOnce(5);
	LogState(TEXT("After StepOnce"), Subsystem);

	// 6 Resume
	DRIVE_LOG_TEST(Log, TEXT("[TEST] ResumeSimulation"));
	Subsystem->BP_ResumeSimulation();
	LogState(TEXT("After Resume"), Subsystem);

	// 7 Stop
	DRIVE_LOG_TEST(Log, TEXT("[TEST] StopSimulation"));
	Subsystem->BP_StopSimulation();
	LogState(TEXT("After Stop"), Subsystem);

	// 8 Reset + Reload
	DRIVE_LOG_TEST(Log, TEXT("[TEST] ResetSimulation (reload scenario)"));
	bool bReset = Subsystem->BP_ResetSimulation(true);
	DRIVE_LOG_TEST(Log, TEXT("[TEST] Reset result: %s"), bReset ? TEXT("SUCCESS") : TEXT("FAIL"));
	LogState(TEXT("After Reset"), Subsystem);

	DRIVE_LOG_TEST(Log, TEXT("==============================================="));
	DRIVE_LOG_TEST(Log, TEXT("        DRIVE 2B SESSION TEST FINISHED        "));
	DRIVE_LOG_TEST(Log, TEXT("==============================================="));
}

void ADriveSessionTestHarness::LogState(const FString& StepName, UWorldSimulationSubsystem* Subsystem)
{
	const EDriveSimulationState SimState = Subsystem->BP_GetSimulationState();
	const int64 Steps = Subsystem->BP_GetRuntimeFixedStepCount();
	const float SimTime = Subsystem->BP_GetClockSimTimeSeconds();

	DRIVE_LOG_TEST(Log, TEXT("[TEST] %s | State=%d | FixedSteps=%lld | SimTime=%.4f"), *StepName, (int32)SimState, Steps, SimTime);
}