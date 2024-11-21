// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_MP_Shooter : ModuleRules
{
	public UE_MP_Shooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
