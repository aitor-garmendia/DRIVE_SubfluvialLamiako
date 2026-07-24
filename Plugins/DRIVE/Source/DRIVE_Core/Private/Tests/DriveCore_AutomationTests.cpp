#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Simulation/FSimulationClock.h"
#include "Messaging/FDriveMessageBus.h"
#include "Scenario/FScenarioRuntime.h"

#include "Systems/DriveSystemContext.h"
#include "Systems/DriveSystemBase.h"
#include "DriveCore_TestTypes.h"


// ------------------------------------------------------------
// CLOCK TESTS
// ------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveClock_BasicInitTest,
	"DRIVE.Core.Clock.BasicInit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveClock_BasicInitTest::RunTest(const FString& Parameters)
{
	FSimulationClock Clock;
	Clock.Initialize(/*FixedDt*/ 1.0f / 60.0f, /*TimeScale*/ 1.0f, /*MaxSteps*/ 8);

	TestTrue(TEXT("Clock starts unpaused"), !Clock.IsPaused());
	TestTrue(TEXT("FixedDeltaTimeSeconds > 0"), Clock.GetFixedDeltaTimeSeconds() > 0.0f);
	TestEqual(TEXT("TimeScale = 1"), Clock.GetTimeScale(), 1.0f);
	TestEqual(TEXT("MaxFixedStepsPerFrame = 8"), Clock.GetMaxFixedStepsPerFrame(), 8);

	TestEqual(TEXT("RealTimeSeconds initially 0"), Clock.GetRealTimeSeconds(), 0.0);
	TestEqual(TEXT("SimTimeSeconds initially 0"), Clock.GetSimTimeSeconds(), 0.0);
	TestEqual(TEXT("Accumulator initially 0"), Clock.GetAccumulatorSeconds(), 0.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveClock_FixedStepAccumulationTest,
	"DRIVE.Core.Clock.FixedStepAccumulation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveClock_FixedStepAccumulationTest::RunTest(const FString& Parameters)
{
	FSimulationClock Clock;
	const float FixedDt = 1.0f / 10.0f; 
	Clock.Initialize(FixedDt, 1.0f, 8);
	Clock.TickRealTime(0.35f);

	const int32 Pending = Clock.GetPendingFixedSteps();
	TestEqual(TEXT("Pending steps after 0.35s at 0.1 fixed dt"), Pending, 3);

	double SimBefore = Clock.GetSimTimeSeconds();
	bool bConsumed1 = Clock.ConsumeFixedStep();
	bool bConsumed2 = Clock.ConsumeFixedStep();
	bool bConsumed3 = Clock.ConsumeFixedStep();

	TestTrue(TEXT("Consume #1 true"), bConsumed1);
	TestTrue(TEXT("Consume #2 true"), bConsumed2);
	TestTrue(TEXT("Consume #3 true"), bConsumed3);

	double SimAfter = Clock.GetSimTimeSeconds();
	TestEqual(TEXT("Sim advanced by 3*FixedDt"), SimAfter - SimBefore, (double)(3.0f * FixedDt));
	TestEqual(TEXT("Pending steps after consuming 3"), Clock.GetPendingFixedSteps(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveClock_PauseStopsSimTest,
	"DRIVE.Core.Clock.PauseStopsSim",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveClock_PauseStopsSimTest::RunTest(const FString& Parameters)
{
	FSimulationClock Clock;
	Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

	Clock.Pause();
	TestTrue(TEXT("Clock is paused"), Clock.IsPaused());

	Clock.TickRealTime(1.0f);
	TestEqual(TEXT("When paused, pending steps should be 0"), Clock.GetPendingFixedSteps(), 0);

	const double SimBefore = Clock.GetSimTimeSeconds();
	const bool bConsumed = Clock.ConsumeFixedStep();
	TestFalse(TEXT("ConsumeFixedStep should fail when no backlog"), bConsumed);
	TestEqual(TEXT("SimTime does not advance while paused"), Clock.GetSimTimeSeconds(), SimBefore);

	return true;
}

// ------------------------------------------------------------
// MESSAGE BUS TESTS
// ------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveBus_PublishSubscribeTest,
	"DRIVE.Core.MessageBus.PublishSubscribe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveBus_PublishSubscribeTest::RunTest(const FString& Parameters)
{
	FDriveMessageBus Bus;

	int32 Received = 0;
	auto Handle = Bus.Subscribe<FDriveTestEvent_Int>([&](const FDriveTestEvent_Int& Ev)
	{
		Received = Ev.Value;
	});

	FDriveTestEvent_Int Ev;
	Ev.Value = 42;

	Bus.Publish(Ev);
	TestEqual(TEXT("Publish delivers immediately"), Received, 42);

	Bus.Unsubscribe(Handle);
	Received = 0;

	Ev.Value = 7;
	Bus.Publish(Ev);
	TestEqual(TEXT("After Unsubscribe, should not receive"), Received, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveBus_EnqueueFlushTest,
	"DRIVE.Core.MessageBus.EnqueueFlush",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveBus_EnqueueFlushTest::RunTest(const FString& Parameters)
{
	FDriveMessageBus Bus;

	int32 CallCount = 0;
	Bus.Subscribe<FDriveTestEvent_Int>([&](const FDriveTestEvent_Int& Ev)
	{
		CallCount++;
	});

	FDriveTestEvent_Int Ev;
	Ev.Value = 1;

	Bus.Enqueue(Ev);
	TestEqual(TEXT("Enqueue does not deliver immediately"), CallCount, 0);

	Bus.Flush();
	TestEqual(TEXT("Flush delivers enqueued events"), CallCount, 1);

	return true;
}

// ------------------------------------------------------------
// RUNTIME TESTS
// ------------------------------------------------------------

class FDriveTestSystem_Dummy final : public FDriveSystemBase
{
public:
	virtual FName GetSystemId() const override { return TEXT("Test.Dummy"); }
	virtual int32 GetInitOrder() const override { return 0; }
	virtual int32 GetFixedTickOrder() const override { return 0; }

	virtual void GetDependencies(TArray<FName>& OutDeps) const override
	{
		OutDeps.Reset();
	}

	virtual bool Initialize(const FDriveSystemContext& Context) override
	{
		const bool bOk = FDriveSystemBase::Initialize(Context);
		return bOk;
	}

	virtual void Shutdown() override
	{
		FDriveSystemBase::Shutdown();
	}

	virtual void TickFixed(float FixedDt) override
	{
		++TickCount;
	}

	virtual void PostTickFixed(float FixedDt) override {}

	int32 GetTickCount() const { return TickCount; }

private:
	int32 TickCount = 0;
};


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveRuntime_StartBlockedWithoutScenarioTest,
	"DRIVE.Core.Runtime.StartBlockedWithoutScenario",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveRuntime_StartBlockedWithoutScenarioTest::RunTest(const FString& Parameters)
{
	FSimulationClock Clock;
	Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

	FDriveMessageBus Bus;
	FScenarioRuntime Runtime;
	Runtime.Initialize(&Clock, &Bus);

	// Si no lo declaramos, Automation lo cuenta como fallo.
	AddExpectedError(TEXT("Start blocked: No scenario loaded"), EAutomationExpectedErrorFlags::Contains, 1);

	// Intentamos arrancar sin haber cargado escenario
	Runtime.Start();

	// No debe estar en running
	TestFalse(TEXT("Runtime must NOT enter Running state if no scenario is loaded"), Runtime.IsRunning());

	// Advance no debe ejecutar ningún fixed step
	const int32 Steps = Runtime.Advance(1.0f);
	TestEqual(TEXT("Advance must execute 0 steps when runtime is not running"), Steps, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveRuntime_BasicRunAdvanceTest,
	"DRIVE.Core.Runtime.BasicRunAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveRuntime_BasicRunAdvanceTest::RunTest(const FString& Parameters)
{
	FSimulationClock Clock;
	Clock.Initialize(/*FixedDt*/ 1.0f / 10.0f, /*TimeScale*/ 1.0f, /*MaxSteps*/ 8);

	FDriveMessageBus Bus;
	FScenarioRuntime Runtime;
	Runtime.Initialize(&Clock, &Bus);

	// Registramos 1 system dummy
	TSharedRef<FDriveTestSystem_Dummy> Sys = MakeShared<FDriveTestSystem_Dummy>();
	Runtime.RegisterSystem(Sys);

	// Construimos context válido
	FDriveSystemContext Ctx(&Runtime, &Clock, &Bus);
	Ctx.EcsWorld = Runtime.GetEcsWorld();
	Ctx.SimState = &Runtime.GetSimState();

	// Build plan + init systems 
	const bool bBuilt = Runtime.BuildExecutionPlanAndInitializeSystems(Ctx);
	TestTrue(TEXT("BuildExecutionPlanAndInitializeSystems should succeed"), bBuilt);

	FString Error;
	const bool bCanStart = Runtime.CanStart(Error);
	TestTrue(TEXT("CanStart should be true after plan+init"), bCanStart);

	Runtime.Start();
	TestTrue(TEXT("Runtime is Running after Start"), Runtime.IsRunning());

	const int32 Steps = Runtime.Advance(0.35f);
	TestEqual(TEXT("Advance should execute 3 fixed steps"), Steps, 3);

	TestEqual(TEXT("Dummy system tick count == steps"), Sys->GetTickCount(), 3);
	TestEqual(TEXT("Runtime fixed step count == steps"), (int32)Runtime.GetFixedStepCount(), 3);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
