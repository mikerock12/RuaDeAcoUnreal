using UnrealBuildTool;

public class RuaDeAcoRuntime : ModuleRules
{
	public RuaDeAcoRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		bEnableExceptions = true;

		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Public"));

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});
	}
}
