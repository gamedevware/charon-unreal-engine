// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "Misc/MonitoredProcess.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFT4CommandRunner, Log, All);

/**
 * @class FT4CommandRunner
 * @brief Extends FMonitoredProcess with a more convenient interface for executing Charon.exe commands.
 *
 * This class is designed to simplify the execution of Charon.exe by managing command line parameters,
 * environment variables, API keys, and temporary files. It encapsulates the setup and execution logic
 * required to run Charon.exe commands effectively.
 */
class FT4CommandRunner final : public TSharedFromThis<FT4CommandRunner>, public FMonitoredProcess
{
private:
	TArray<FString> FileCleanupList;
	FString RunScriptPath;

public:
	/**
	 * @brief List of environment variables to pass to the running process.
	 *
	 * This map contains key-value pairs representing environment variables that will be made available
	 * to the Charon.exe process upon execution.
	 */
	TMap<FString, FString> EnvironmentVariables;

	/**
	 * Constructs an FT4CommandRunner with specific command line parameters.
	 *
	 * @param InParameters The command line parameters to pass to Charon.exe.
	 */
	explicit FT4CommandRunner(FString InParameters);
	/**
	 * Destructor for FT4CommandRunner.
	 */
	virtual ~FT4CommandRunner() override;

	/**
	 * Launches the Charon.exe process with the specified parameters and environment.
	 *
	 * @return true if the process was successfully launched, false otherwise.
	 */
	virtual bool Launch() override;
	
	/**
	 * Attaches the filename of a temporary file for deletion after the command completes or fails.
	 *
	 * @param InFilePath The path of the temporary file to be managed.
	 */
	void AttachTemporaryFile(const FString& InFilePath);
};
