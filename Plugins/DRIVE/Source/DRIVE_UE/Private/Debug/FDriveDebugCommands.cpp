#include "Debug/FDriveDebugCommands.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UWorldSimulationSubsystem.h"
#include "Logging/DriveLog.h"

DEFINE_LOG_CATEGORY_STATIC(LogDriveDebugCmd, Log, All);

static UWorld* GetBestDriveWorld()
{
	if (!GEngine) return nullptr;

	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (Ctx.WorldType == EWorldType::PIE && Ctx.World())
		{
			return Ctx.World();
		}
	}

	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (Ctx.WorldType == EWorldType::Game && Ctx.World())
		{
			return Ctx.World();
		}
	}

	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (Ctx.World())
		{
			return Ctx.World();
		}
	}

	return nullptr;
}

static UWorldSimulationSubsystem* GetDriveSubsystem()
{
	UWorld* World = GetBestDriveWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<UWorldSimulationSubsystem>();
}

static bool GDriveDebugRegistered = false;

static IConsoleObject* GCmdHelp = nullptr;
static IConsoleObject* GCmdClock = nullptr;
static IConsoleObject* GCmdRuntime = nullptr;
static IConsoleObject* GCmdState = nullptr;
static IConsoleObject* GCmdAll = nullptr;

void FDriveDebugCommands::Register()
{
	if (GDriveDebugRegistered)
	{
		return;
	}

	IConsoleManager& CM = IConsoleManager::Get();

	GCmdHelp = CM.RegisterConsoleCommand(
		TEXT("drive.debug.help"),
		TEXT("DRIVE debug help. Usage: drive.debug.help"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FDriveDebugCommands::Cmd_Help),
		ECVF_Default
	);

	GCmdClock = CM.RegisterConsoleCommand(
		TEXT("drive.debug.clock"),
		TEXT("Print DRIVE Clock state. Usage: drive.debug.clock"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FDriveDebugCommands::Cmd_Clock),
		ECVF_Default
	);

	GCmdRuntime = CM.RegisterConsoleCommand(
		TEXT("drive.debug.runtime"),
		TEXT("Print DRIVE Runtime state. Usage: drive.debug.runtime"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FDriveDebugCommands::Cmd_Runtime),
		ECVF_Default
	);

	GCmdState = CM.RegisterConsoleCommand(
		TEXT("drive.debug.state"),
		TEXT("Print DRIVE SimState summary. Usage: drive.debug.state"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FDriveDebugCommands::Cmd_State),
		ECVF_Default
	);

	GCmdAll = CM.RegisterConsoleCommand(
		TEXT("drive.debug.all"),
		TEXT("Print all DRIVE debug info. Usage: drive.debug.all"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FDriveDebugCommands::Cmd_All),
		ECVF_Default
	);

	GDriveDebugRegistered = true;

	DRIVE_LOG_DEBUG(Log, TEXT("[DRIVE][DebugCmd] Registered console commands (drive.debug.*)."));
}

void FDriveDebugCommands::Unregister()
{
	if (!GDriveDebugRegistered)
	{
		return;
	}

	IConsoleManager& CM = IConsoleManager::Get();

	if (GCmdHelp)   { CM.UnregisterConsoleObject(GCmdHelp);   GCmdHelp = nullptr; }
	if (GCmdClock)  { CM.UnregisterConsoleObject(GCmdClock);  GCmdClock = nullptr; }
	if (GCmdRuntime){ CM.UnregisterConsoleObject(GCmdRuntime);GCmdRuntime = nullptr; }
	if (GCmdState)  { CM.UnregisterConsoleObject(GCmdState);  GCmdState = nullptr; }
	if (GCmdAll)    { CM.UnregisterConsoleObject(GCmdAll);    GCmdAll = nullptr; }

	GDriveDebugRegistered = false;

	DRIVE_LOG_DEBUG(Log, TEXT("[DRIVE][DebugCmd] Unregistered console commands."));
}

void FDriveDebugCommands::Cmd_Help(const TArray<FString>& Args)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("DRIVE: help command executed"));
    }
    DRIVE_LOG_DEBUG(Log, TEXT("DRIVE: help command executed (log test)"));

	DRIVE_LOG_DEBUG(Display, TEXT("=== DRIVE Debug Commands ==="));
	DRIVE_LOG_DEBUG(Display, TEXT("  drive.debug.help"));
	DRIVE_LOG_DEBUG(Display, TEXT("  drive.debug.clock"));
	DRIVE_LOG_DEBUG(Display, TEXT("  drive.debug.runtime"));
	DRIVE_LOG_DEBUG(Display, TEXT("  drive.debug.state"));
	DRIVE_LOG_DEBUG(Display, TEXT("  drive.debug.all"));
}

void FDriveDebugCommands::Cmd_Clock(const TArray<FString>& Args)
{
	UWorldSimulationSubsystem* Sub = GetDriveSubsystem();
	if (!Sub)
	{
		DRIVE_LOG_DEBUG(Log, TEXT("[DRIVE][DebugCmd] No UWorldSimulationSubsystem found (are you in PIE?)."));
		return;
	}

	const FSimulationClock& C = Sub->GetClock();

	DRIVE_LOG_DEBUG(Display, TEXT("=== DRIVE Clock ==="));
	DRIVE_LOG_DEBUG(Display, TEXT("FixedDt=%.6f  TimeScale=%.3f  MaxSteps=%d  Paused=%d"),
		C.GetFixedDeltaTimeSeconds(), C.GetTimeScale(), C.GetMaxFixedStepsPerFrame(), C.IsPaused() ? 1 : 0);

	DRIVE_LOG_DEBUG(Display, TEXT("RealTime=%.6f  SimTime=%.6f  Acc=%.6f  Alpha=%.3f"),
		C.GetRealTimeSeconds(), C.GetSimTimeSeconds(), C.GetAccumulatorSeconds(), C.GetInterpolationAlpha());

	DRIVE_LOG_DEBUG(Display, TEXT("PendingSteps=%d  LastRaw=%d  LastExec=%d  LastDropped=%d  LastScaledDelta=%.6f"),
		C.GetPendingFixedSteps(), C.GetLastRawSteps(), C.GetLastStepsExecuted(), C.GetLastDroppedSteps(), C.GetLastScaledDelta());
}

void FDriveDebugCommands::Cmd_Runtime(const TArray<FString>& Args)
{
	UWorldSimulationSubsystem* Sub = GetDriveSubsystem();
	if (!Sub)
	{
		DRIVE_LOG_DEBUG(Log, TEXT("[DRIVE][DebugCmd] No UWorldSimulationSubsystem found (are you in PIE?)."));
		return;
	}

	const FScenarioRuntime& R = Sub->GetRuntime();

	DRIVE_LOG_DEBUG(Display, TEXT("=== DRIVE Runtime ==="));
	DRIVE_LOG_DEBUG(Display, TEXT("Initialized=%d  State=%d  FixedSteps=%llu"),
		R.IsInitialized() ? 1 : 0, (int32)R.GetState(), (unsigned long long)R.GetFixedStepCount());

	DRIVE_LOG_DEBUG(Display, TEXT("ScenarioLoaded=%d  ScenarioLocked=%d  HasInitialState=%d"),
		R.HasScenarioLoaded() ? 1 : 0, R.IsScenarioLocked() ? 1 : 0, R.HasInitialState() ? 1 : 0);

	DRIVE_LOG_DEBUG(Display, TEXT("Systems=%d  FixedTickPlan=%d  Agents=%d  ExecHash=0x%08x"),
		R.GetRegisteredSystemCount(), R.GetExecutionPlanCount(), R.GetActiveAgentCount(), R.GetExecutionOrderHash());
}

void FDriveDebugCommands::Cmd_State(const TArray<FString>& Args)
{
	UWorldSimulationSubsystem* Sub = GetDriveSubsystem();
	if (!Sub)
	{
		DRIVE_LOG_DEBUG(Log, TEXT("[DRIVE][DebugCmd] No UWorldSimulationSubsystem found (are you in PIE?)."));
		return;
	}

	const FDriveSimState& S = Sub->GetRuntime().GetSimState();

	DRIVE_LOG_DEBUG(Display, TEXT("=== DRIVE SimState ==="));
	DRIVE_LOG_DEBUG(Display, TEXT("AliveEntities=%d  ComponentTypes=%d  TotalComponents=%d"),
		S.GetAliveCount(), S.GetStorageTypeCount(), S.GetTotalComponentCount());
}

void FDriveDebugCommands::Cmd_All(const TArray<FString>& Args)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("DRIVE: all command executed"));
    }
    DRIVE_LOG_DEBUG(Log, TEXT("DRIVE: all command executed (log test)"));

	Cmd_Clock(Args);
	Cmd_Runtime(Args);
	Cmd_State(Args);
}