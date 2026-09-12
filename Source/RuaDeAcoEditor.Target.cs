using UnrealBuildTool;

public class RuaDeAcoEditorTarget : TargetRules
{
	public RuaDeAcoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;
		ExtraModuleNames.Add("RuaDeAco");
	}
}
