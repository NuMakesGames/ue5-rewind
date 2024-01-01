// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Rewind : ModuleRules
{
	public Rewind(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
