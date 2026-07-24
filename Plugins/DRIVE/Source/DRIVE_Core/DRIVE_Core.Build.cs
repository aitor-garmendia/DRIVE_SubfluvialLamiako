using UnrealBuildTool;

public class DRIVE_Core : ModuleRules
{
	public DRIVE_Core(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
		PrivateDependencyModuleNames.AddRange(new string[] {});
	}
}
