using UnrealBuildTool;

public class DRIVE_UE : ModuleRules
{
	public DRIVE_UE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "DeveloperSettings", "DRIVE_Core", "UMG"});
		PrivateDependencyModuleNames.AddRange(new string[] {"Slate", "SlateCore"});
	}
}