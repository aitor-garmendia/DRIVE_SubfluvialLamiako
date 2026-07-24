#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ECS/DriveEntityRegistry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDriveEntityRegistry_BasicLifecycle,
	"DRIVE.IT3.EntityRegistry.BasicLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDriveEntityRegistry_BasicLifecycle::RunTest(const FString& Parameters)
{
	FDriveEntityRegistry R;

	const FDriveEntityId A = R.Create();
	const FDriveEntityId B = R.Create();

	TestTrue(TEXT("A alive after create"), R.IsAlive(A));
	TestTrue(TEXT("B alive after create"), R.IsAlive(B));
	TestEqual(TEXT("AliveCount == 2"), R.GetAliveCount(), 2);

	TestTrue(TEXT("Destroy A succeeds"), R.Destroy(A));
	TestFalse(TEXT("A alive after destroy"), R.IsAlive(A));
	TestEqual(TEXT("AliveCount == 1"), R.GetAliveCount(), 1);

	// Reutilización: al crear
	const FDriveEntityId C = R.Create();
	TestTrue(TEXT("C alive after create"), R.IsAlive(C));
	TestEqual(TEXT("AliveCount == 2"), R.GetAliveCount(), 2);

	// Si C reutilizó el índice de A, la generación debe ser distinta -> A inválida
	if (C.GetIndex() == A.GetIndex())
	{
		TestTrue(TEXT("Generation changed on reuse"), C.GetGeneration() != A.GetGeneration());
		TestFalse(TEXT("Old handle A must not become alive again"), R.IsAlive(A));
	}

	TestTrue(TEXT("Registry invariants OK"), R.CheckInvariants());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS