// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppPassValueParameterByConstReference
#pragma once

// Include headers for command execution functions
#include "Execute_CreateHero.h"
#include "Execute_DeleteHero.h"
#include "Execute_DeleteHeroById.h"
#include "Execute_ExportHeroes.h"
#include "Execute_ExportHeroesToFile.h"
#include "Execute_ExportLocalizableData.h"
#include "Execute_FindHeroById.h"
#include "Execute_GetT4TemplateGenerator.h"
#include "Execute_BulkCreateHeroes.h"
#include "Execute_BulkDeleteHeroes.h"
#include "Execute_ListItems.h"
#include "Execute_ListReligiousHeroes.h"
#include "Execute_RunT4Template.h"
#include "Execute_UpdateHero.h"
#include "Execute_ValidateGameData.h"

// Include Unreal Engine framework headers
#include "Framework/Commands/Commands.h"
#include "Templates/SharedPointer.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

// Define the custom command class
class FGameDataExtensionCommands: public TCommands< FGameDataExtensionCommands >
{
public:
    // Command instances for various actions
    TSharedPtr<FUICommandInfo> ExportHeroes;
    TSharedPtr<FUICommandInfo> ExportHeroesToFile;
    TSharedPtr<FUICommandInfo> ExportLocalizableData;
    TSharedPtr<FUICommandInfo> BulkCreateHeroes;
    TSharedPtr<FUICommandInfo> BulkDeleteHeroes;
    TSharedPtr<FUICommandInfo> CreateHero;
    TSharedPtr<FUICommandInfo> DeleteHeroById;
    TSharedPtr<FUICommandInfo> DeleteHero;
    TSharedPtr<FUICommandInfo> UpdateHero;
    TSharedPtr<FUICommandInfo> FindHeroById;
    TSharedPtr<FUICommandInfo> ListReligiousHeroes;
    TSharedPtr<FUICommandInfo> ListItems;
    TSharedPtr<FUICommandInfo> ValidateGameData;
    TSharedPtr<FUICommandInfo> GetT4TemplateGenerator;
    TSharedPtr<FUICommandInfo> RunT4Template;
    
    // Constructor to initialize the command set
    FGameDataExtensionCommands()
    : TCommands(
        TEXT("GameDataExtensions"), // Command set name
        INVTEXT("Game Data Extensions"), // Display name
        NAME_None, // Parent command set
        FAppStyle::GetAppStyleSetName()) // Style set
    {
    }
    
    // Register all commands
    virtual void RegisterCommands() override
    {
#define LOCTEXT_NAMESPACE "GameDataExtensionCommands"
        // Define each command with a label, tooltip, and input chord
        UI_COMMAND(ExportHeroes, "Export Heroes", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ExportHeroesToFile, "Export Heroes to File", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ExportLocalizableData, "Export Localizable Data", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(BulkCreateHeroes, "Bulk Create Heroes", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(BulkDeleteHeroes, "Bulk Delete Heroes", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(CreateHero, "Create Hero", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(DeleteHeroById, "Delete Hero By Id", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(DeleteHero, "Delete Hero", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(UpdateHero, "Update Hero", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(FindHeroById, "Find Hero", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ListReligiousHeroes, "List Heroes", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ListItems, "List Items (Including Embedded)", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(ValidateGameData, "Validate Game Data", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(GetT4TemplateGenerator, "Preprocess .tt Template", "", EUserInterfaceActionType::Button, FInputChord());
        UI_COMMAND(RunT4Template, "Transform .tt Template", "", EUserInterfaceActionType::Button, FInputChord());
#undef LOCTEXT_NAMESPACE
    }

    // Create a command list and map commands to their execution functions
    static TSharedPtr<FUICommandList> GetCommandList(const TArray<UObject*>& ContextSensitiveObjects)
    {
        const FGameDataExtensionCommands& GameDataExtensionCommands = Get();
        TSharedPtr<FUICommandList> GameDataUICommandList = MakeShareable(new FUICommandList);

        // Map each command to its corresponding execution function
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ExportHeroes, FExecuteAction::CreateStatic(&Execute_ExportHeroes, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ExportHeroesToFile, FExecuteAction::CreateStatic(&Execute_ExportHeroesToFile, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ExportLocalizableData, FExecuteAction::CreateStatic(&Execute_ExportLocalizableData, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.BulkCreateHeroes, FExecuteAction::CreateStatic(&Execute_BulkCreateHeroes, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.BulkDeleteHeroes, FExecuteAction::CreateStatic(&Execute_BulkDeleteHeroes, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.CreateHero, FExecuteAction::CreateStatic(&Execute_CreateHero, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.DeleteHeroById, FExecuteAction::CreateStatic(&Execute_DeleteHeroById, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.DeleteHero, FExecuteAction::CreateStatic(&Execute_DeleteHero, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.UpdateHero, FExecuteAction::CreateStatic(&Execute_UpdateHero, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.FindHeroById, FExecuteAction::CreateStatic(&Execute_FindHeroById, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ListReligiousHeroes, FExecuteAction::CreateStatic(&Execute_ListReligiousHeroes, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ListItems, FExecuteAction::CreateStatic(&Execute_ListItems, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.ValidateGameData, FExecuteAction::CreateStatic(&Execute_ValidateGameData, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.GetT4TemplateGenerator, FExecuteAction::CreateStatic(&Execute_GetT4TemplateGenerator, ContextSensitiveObjects));
        GameDataUICommandList->MapAction(GameDataExtensionCommands.RunT4Template, FExecuteAction::CreateStatic(&Execute_RunT4Template, ContextSensitiveObjects));

        return GameDataUICommandList;
    }

    // Build the custom menu in the Unreal Engine editor
    static void ConstructGameDataExtensionMenu(FMenuBuilder& MenuBuilder)
    {
        struct Local
        {
            // Add commands to the menu
            static void BuildGameDataExtensionMenu(FMenuBuilder& InMenuBuilder)
            {
                const FGameDataExtensionCommands& GameDataExtensionCommands = FGameDataExtensionCommands::Get();

                // Add commands to the "Test Actions" section
                InMenuBuilder.BeginSection("GameDataMenuTestActions", INVTEXT("Test Actions"));
                {
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ExportHeroes);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ExportHeroesToFile);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ExportLocalizableData);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.BulkCreateHeroes);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.BulkDeleteHeroes);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.CreateHero);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.DeleteHeroById);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.DeleteHero);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.UpdateHero);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.FindHeroById);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ListReligiousHeroes);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ListItems);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.ValidateGameData);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.GetT4TemplateGenerator);
                    InMenuBuilder.AddMenuEntry(GameDataExtensionCommands.RunT4Template);
                }
                InMenuBuilder.EndSection();
            }
        };

        // Add the "Game Data Extensions" submenu to the main menu
        MenuBuilder.BeginSection("GameDataMenuExtensionActions", INVTEXT("Extension Actions"));
        {
            MenuBuilder.AddSubMenu(
                INVTEXT("Game Data Extensions"), 
                INVTEXT("Game data extension menu items."), 
                FNewMenuDelegate::CreateStatic(&Local::BuildGameDataExtensionMenu), 
                false, 
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Link")
            );
        }
        MenuBuilder.EndSection();
    }
};