// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "EExportMode.generated.h"

UENUM()
/**
 * @enum EExportMode
 * @brief Defines the mode of export for documents during the FCharonCli::Export() operation.
 *
 * This enumeration specifies different modes that control how documents are exported from the game data.
 * Each mode tailors the exported data for different purposes, such as modification, publication, localization,
 * or extraction.
 */
enum class EExportMode
{
	/**
	 * @brief Exports a proper graph of documents with all references kept valid.
	 *
	 * This mode is suitable for purposes where the exported data is intended to be imported back
	 * after modifications. It maintains the integrity and relationships between documents.
	 */
	Normal = 0,

	/**
	 * @brief Exports documents similar to Default but trims any extra data.
	 *
	 * In Publication mode, extra data is removed from the documents to reduce their size and load time.
	 * This mode is ideal for preparing game data for production, where efficiency is key.
	 */
	Publication = 1,

	/**
	 * @brief Strips exported documents of all data except localizable text.
	 *
	 * This mode is used for exporting texts that require localization. It is ideal for situations
	 * where the exported data is sent for translation with the intention of importing the translated
	 * data back into the game.
	 */
	Localization = 2,

	/**
	 * @brief Deprecated option, same as Normal
	 */
	Extraction = 3
};