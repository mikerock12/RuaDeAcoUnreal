using UnrealBuildTool;

public class RuaDeAco : ModuleRules
{
	public RuaDeAco(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory,
			System.IO.Path.Combine(ModuleDirectory, "..", "RuaDeAcoRuntime", "Public")
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Paper2D",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings"
		});
	}
}
