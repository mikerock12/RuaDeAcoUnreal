using UnrealBuildTool;

public class RuaDeAcoTarget : TargetRules
{
	public RuaDeAcoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;
		ExtraModuleNames.Add("RuaDeAcoRuntime");
		ExtraModuleNames.Add("RuaDeAco");
	}
}
