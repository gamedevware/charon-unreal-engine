// Copyright GameDevWare, Denis Zykov 2025

using System.IO;
using System.Text.RegularExpressions;
using UnrealBuildTool;

public class Charon : ModuleRules
{
	public Charon(ReadOnlyTargetRules Target) : base(Target)
	{
		this.PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		this.DefinePluginVersion();

		this.PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		this.PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		this.PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"JsonUtilities", 
				"Json",
				// ... add other public dependencies that you statically link with here ...
			}
			);

		this.PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				// ... add private dependencies that you statically link with here ...	
			}
			);

		this.DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
	private void DefinePluginVersion()
	{
		var pluginFilePath = Path.Combine(this.ModuleDirectory, "../../Charon.uplugin");
		if (File.Exists(pluginFilePath))
		{
			var pluginFileContent = File.ReadAllText(pluginFilePath);
            
			// Extract VersionName using regex
			// Matches: "VersionName": "1.5.3" or "VersionName":"1.5.3"
			var versionMatch = Regex.Match(pluginFileContent, 
				@"""VersionName""\s*:\s*""([^""]+)""");
            
			var version = versionMatch.Success ? versionMatch.Groups[1].Value : "0.0.0";
            
			var parts = version.Split('.');
			var major = parts.Length > 0 && int.TryParse(parts[0], out var maj) ? maj : 0;
			var minor = parts.Length > 1 && int.TryParse(parts[1], out var min) ? min : 0;
			var patch = parts.Length > 2 && int.TryParse(parts[2], out var pat) ? pat : 0;

			this.PublicDefinitions.Add($"CHARON_PLUGIN_MAJOR_VERSION={major}");
			this.PublicDefinitions.Add($"CHARON_PLUGIN_MINOR_VERSION={minor}");
			this.PublicDefinitions.Add($"CHARON_PLUGIN_PATCH_VERSION={patch}");
			this.PublicDefinitions.Add($"CHARON_PLUGIN_VERSION_STRING=\"{version}\"");
            
			// Optional: Add version comparison macro
			this.PublicDefinitions.Add("CHARON_PLUGIN_VERSION_AT_LEAST(Major, Minor) " +
				"((CHARON_PLUGIN_MAJOR_VERSION > Major) || " +
				"(CHARON_PLUGIN_MAJOR_VERSION == Major && CHARON_PLUGIN_MINOR_VERSION >= Minor))");
		}
		else
		{
			// Fallback if .uplugin file not found
			this.PublicDefinitions.Add("CHARON_PLUGIN_MAJOR_VERSION=0");
			this.PublicDefinitions.Add("CHARON_PLUGIN_MINOR_VERSION=0");
			this.PublicDefinitions.Add("CHARON_PLUGIN_PATCH_VERSION=0");
			this.PublicDefinitions.Add("CHARON_PLUGIN_VERSION_STRING=\"0.0.0\"");
		}
	}
}
