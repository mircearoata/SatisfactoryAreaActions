// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

public class AreaActions : ModuleRules
{
    public AreaActions(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PrivatePCHHeaderFile = "Public/AreaActions.h";
        
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate", "SlateCore"
		});
        
        PublicDependencyModuleNames.AddRange(new string[] {
	        "FactoryGame",
	        "SML"
        });
    }
}