// Copyright GameDevWare, Denis Zykov 2025

using System.IO;
using UnrealBuildTool;

public class CharonEditor: ModuleRules
{
	public CharonEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "Engine", "UnrealEd", "Charon"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",  "Slate", "SlateCore",
				"InputCore", "ToolWidgets", "AppFramework",
				 "Json", "DesktopPlatform",
				"AssetTools", "AssetRegistry", "RHI",
				"RenderCore", "TextureUtilitiesCommon",
				"PropertyEditor",
				"JsonUtilities", 
				"ClassViewer",
				"EditorStyle",
				"WebBrowser",
				"HTTP", 
				"HotReload", 
				"EditorScriptingUtilities", 
				"GameProjectGeneration", 
				"Projects", 
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
		
		//PublicAdditionalShadowFiles.Add("Path/To/Your/Icon.png");
	}
}
