#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Simulation/FSimulationClock.h"
#include "Scenario/FScenarioRuntime.h"
#include "Messaging/FDriveMessageBus.h"

// ------------------------------------------------------------
//  DRIVE.Hardening.Runtime.OrderAndClamp
//  - Start sin scenario: bloquea sin crashear y sigue Stopped
//  - Hitch: el clock hace clamp de steps
// ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveHardening_RuntimeOrderAndClamp,
    "DRIVE.Hardening.Runtime.OrderAndClamp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveHardening_RuntimeOrderAndClamp::RunTest(const FString& Parameters)
{
    FSimulationClock Clock;
    Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

    FDriveMessageBus Bus;
    FScenarioRuntime Runtime;
    Runtime.Initialize(&Clock, &Bus);

    // Start SIN scenario
    {
        FString Error;
        const bool bCanStart = Runtime.CanStart(Error);
        TestFalse(TEXT("CanStart should be false without scenario/systems"), bCanStart);
        TestTrue(TEXT("Error should not be empty"), !Error.IsEmpty());

        Runtime.Start();
        TestTrue(TEXT("State should remain Stopped"), Runtime.GetState() == EScenarioRuntimeState::Stopped);
        TestFalse(TEXT("Should not be running"), Runtime.IsRunning());
    }

    // Hitch
    {
        Clock.TickRealTime(1.0f); 
        const int32 Pending = Clock.GetPendingFixedSteps();
        TestTrue(TEXT("Pending steps should be clamped to MaxFixedStepsPerFrame"),
            Pending <= Clock.GetMaxFixedStepsPerFrame());

        TestTrue(TEXT("Clock should report last raw steps >= executed steps"),
            Clock.GetLastRawSteps() >= Clock.GetLastStepsExecuted());
    }

    return true;
}

// ------------------------------------------------------------
//  DRIVE.Hardening.Runtime.DoubleStartBlocked
//  - Start() dos veces sin scenario: sigue Stopped (idempotencia)
// ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveHardening_RuntimeDoubleStartBlocked,
    "DRIVE.Hardening.Runtime.DoubleStartBlocked",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveHardening_RuntimeDoubleStartBlocked::RunTest(const FString& Parameters)
{
    FSimulationClock Clock;
    Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

    FDriveMessageBus Bus;
    FScenarioRuntime Runtime;
    Runtime.Initialize(&Clock, &Bus);

    Runtime.Start();
    TestTrue(TEXT("After first Start without scenario, state must remain Stopped"),
        Runtime.GetState() == EScenarioRuntimeState::Stopped);

    Runtime.Start();
    TestTrue(TEXT("After second Start without scenario, state must remain Stopped"),
        Runtime.GetState() == EScenarioRuntimeState::Stopped);

    TestFalse(TEXT("Should not be running"), Runtime.IsRunning());
    return true;
}

// ------------------------------------------------------------
//  DRIVE.Hardening.Runtime.PauseStopOrder
//  - Pause estando Stopped: no cambia nada
//  - Stop estando Stopped: no cambia nada
// ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveHardening_RuntimePauseStopOrder,
    "DRIVE.Hardening.Runtime.PauseStopOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveHardening_RuntimePauseStopOrder::RunTest(const FString& Parameters)
{
    FSimulationClock Clock;
    Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

    FDriveMessageBus Bus;
    FScenarioRuntime Runtime;
    Runtime.Initialize(&Clock, &Bus);

    TestTrue(TEXT("Initial state should be Stopped"), Runtime.GetState() == EScenarioRuntimeState::Stopped);

    // Pause en Stopped
    Runtime.Pause();
    TestTrue(TEXT("Pause in Stopped should keep Stopped"), Runtime.GetState() == EScenarioRuntimeState::Stopped);
    TestFalse(TEXT("Should not be paused"), Runtime.IsPaused());

    // Stop en Stopped
    Runtime.Stop();
    TestTrue(TEXT("Stop in Stopped should keep Stopped"), Runtime.GetState() == EScenarioRuntimeState::Stopped);

    return true;
}

// ------------------------------------------------------------
//  DRIVE.Hardening.Runtime.ResetClearsState
//  - Reset debe dejar todo en estado limpio
// ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveHardening_RuntimeResetClearsState,
    "DRIVE.Hardening.Runtime.ResetClearsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveHardening_RuntimeResetClearsState::RunTest(const FString& Parameters)
{
    FSimulationClock Clock;
    Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

    FDriveMessageBus Bus;
    FScenarioRuntime Runtime;
    Runtime.Initialize(&Clock, &Bus);
    Runtime.Start();
    Runtime.Pause();
    Runtime.Stop();
    Runtime.Reset();

    TestTrue(TEXT("After Reset, state should be Stopped"), Runtime.GetState() == EScenarioRuntimeState::Stopped);
    TestFalse(TEXT("After Reset, should not be running"), Runtime.IsRunning());
    TestFalse(TEXT("After Reset, should not be paused"), Runtime.IsPaused());

    TestFalse(TEXT("After Reset, scenario should not be loaded"), Runtime.HasScenarioLoaded());
    TestFalse(TEXT("After Reset, scenario should be unlocked"), Runtime.IsScenarioLocked());

    TestTrue(TEXT("After Reset, active agents should be 0"), Runtime.GetActiveAgentCount() == 0);

    return true;
}

// ------------------------------------------------------------
//  DRIVE.Hardening.Runtime.AdvanceWhenNotRunning
//  - Advance() cuando no está Running debe devolver 0
// ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDriveHardening_RuntimeAdvanceWhenNotRunning,
    "DRIVE.Hardening.Runtime.AdvanceWhenNotRunning",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDriveHardening_RuntimeAdvanceWhenNotRunning::RunTest(const FString& Parameters)
{
    FSimulationClock Clock;
    Clock.Initialize(1.0f / 60.0f, 1.0f, 8);

    FDriveMessageBus Bus;
    FScenarioRuntime Runtime;
    Runtime.Initialize(&Clock, &Bus);

    const int32 Steps1 = Runtime.Advance(0.016f);
    TestTrue(TEXT("Advance in Stopped should return 0"), Steps1 == 0);

    Runtime.Start();
    const int32 Steps2 = Runtime.Advance(0.5f);
    TestTrue(TEXT("Advance when start is blocked should still return 0"), Steps2 == 0);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS