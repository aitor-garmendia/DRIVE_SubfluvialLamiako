// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DRIVE_Template : ModuleRules
{
	public DRIVE_Template(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"InputCore",
			"EnhancedInput",
			"DRIVE_TemplateRuntime",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"GraphEditor",
			"PropertyEditor",
			"EditorStyle",
			"AnimGraph",
			"BlueprintGraph",
			"KismetCompiler"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
