# Introduction

Charon is a versatile [plugin](https://www.fab.com/listings/8cdfd7af-e1e9-4c97-b28d-d4b196767824) tailored for Unreal Engine, designed to facilitate data-driven game design
by allowing both developers and game designers to efficiently manage static game data,
like units, items, missions, quests, and other. It provides a user-friendly interface that requires no special skills
for game designers, simplifying the process of data manipulation. For programmers, Charon streamlines
development workflows by generating C++ code to load game data seamlessly into the game.

<img width="800" alt="editor ui" src="https://raw.githubusercontent.com/gamedevware/charon/refs/heads/main/docs/assets/ue_editor_screenshot.png"/>  

# Installation

Prerequisites
---------------
Unreal Engine Editor plugin uses `dotnet charon` tool, which is a .NET Core application built for [.NET 8](https://dotnet.microsoft.com/en-us/download/dotnet/8.0) it runs wherever Unreal Engine Editor can run.

The generated code and data do not require any additional dependencies for your game's runtime.  

Installation from Fab
---------------------------------------

1. Install the [.NET 8 or later](https://dotnet.microsoft.com/en-us/download/dotnet/8.0) for your operating system (Windows, MacOS, Linux).
2. Ensure your Unreal Engine version is 5.3 or later.
3. [Open](https://www.fab.com/listings/8cdfd7af-e1e9-4c97-b28d-d4b196767824) the Fab page for the plugin.
4. Click the **Add To My Library** button in the upper right corner and follow the instructions.
5. Open Unreal Engine and follow the [instruction](https://dev.epicgames.com/documentation/en-us/unreal-engine/working-with-plugins-in-unreal-engine) for plugin installation.

# How to Use

- [Working with the Plugin](https://gamedevware.github.io/charon/unreal_engine/overview.html#working-with-the-plugin)
- [How to Create Game Data File](https://gamedevware.github.io/charon/unreal_engine/creating_game_data.html)

# Documentation

[Charon Documentation](https://gamedevware.github.io/charon/) • [Unreal Engine Plugin Documentation](https://gamedevware.github.io/charon/unreal_engine/overview.html)  

# Examples
- [Example Project](https://github.com/gamedevware/charon-unreal-engine/tree/main/src/GameDevWare.Charon.UnrealEngine)
- [Editor Extensions](https://github.com/gamedevware/charon-unreal-engine/tree/main/src/GameDevWare.Charon.UnrealEngine/Source/CharonPluginGameEditor)
  - [CharonCli Examples](https://github.com/gamedevware/charon-unreal-engine/tree/main/src/GameDevWare.Charon.UnrealEngine/Source/CharonPluginGameEditor/Private/CharonCliExamples)

# Collaboration

[Discord](https://discord.gg/2quB5vXryd) • [Plugin Issues](https://github.com/gamedevware/charon-unreal-engine/issues) • [support@gamedevware.com](mailto:support@gamedevware.com)  


# License

MIT
