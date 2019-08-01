// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Main_Battle_TankEditorTarget : TargetRules
{
	public Main_Battle_TankEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Main_Battle_Tank" } );
	}
}
