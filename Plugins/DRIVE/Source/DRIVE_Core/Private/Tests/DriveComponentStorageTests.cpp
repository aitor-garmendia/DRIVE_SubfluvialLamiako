#if WITH_DEV_AUTOMATION_TESTS
#include "ECS/DriveComponentStorage.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDriveComponentStorageBasicTest,
	"DRIVE.ECS.ComponentStorage.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FDriveComponentStorageBasicTest::RunTest(const FString& Parameters)
{
	FDriveComponentStorage Storage;

	struct FTestComponent
	{
		int32 Value = 0;
	};

	FDriveEntityId Entity = FDriveEntityId::Make(1, 0);

	auto& Store = Storage.GetOrCreateStore<FTestComponent>();

	Store.Emplace(Entity, FTestComponent{42});

	TestTrue(TEXT("Entity has component"), Store.Has(Entity));
	TestEqual(TEXT("Component value correct"), Store.GetChecked(Entity).Value, 42);

	Store.Remove(Entity);
	TestFalse(TEXT("Component removed"), Store.Has(Entity));

	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS