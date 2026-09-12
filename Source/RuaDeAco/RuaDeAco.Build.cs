using UnrealBuildTool;

public class RuaDeAco : ModuleRules
{
	public RuaDeAco(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicIncludePaths.Add(ModuleDirectory);

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
			"SlateCore",
			"RuaDeAcoRuntime",
			"ImageWrapper"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings",
			"RHI",
			"RenderCore"
		});
	}
}
