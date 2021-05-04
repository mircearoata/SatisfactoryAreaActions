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

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate", "SlateCore"
		});
        
        PublicDependencyModuleNames.AddRange(new string[] {"FactoryGame", "SML"});

        var processStartInfo = new ProcessStartInfo("git", "diff --quiet")
        {
	        UseShellExecute = false,
	        WorkingDirectory = Target.ProjectFile.Directory.FullName,
        };
        var gitDiffQuiet = Process.Start(processStartInfo);
        gitDiffQuiet.WaitForExit();
        var gitDirty = gitDiffQuiet.ExitCode == 1;
        Console.Out.WriteLine("Area Actions plugin is {0}", gitDirty ? "dirty" : "clean");

        PublicDefinitions.Add(gitDirty ? "AA_DEBUG=1" : "AA_DEBUG=0");
    }
}