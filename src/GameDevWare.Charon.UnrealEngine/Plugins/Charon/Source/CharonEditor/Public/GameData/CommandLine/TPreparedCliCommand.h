// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "Async/Async.h"
#include "Misc/MonitoredProcess.h"
#include "../ICharonTask.h"
#include "FValidationReport.h"
#include "FImportReport.h"
#include "JsonObjectConverter.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"


DECLARE_LOG_CATEGORY_EXTERN(LogTCharonCliCommand, Log, All);

/**
 * @class TPreparedCliCommand
 * @brief Represents a prepared command for CLI application, with customizable result type.
 *
 * This class encapsulates a command that can be executed via FCharonCli. It is templated to allow
 * specifying the type of result the command should produce. By default, it uses an int32 to represent
 * the process exit code if no specific result type is provided.
 *
 * @tparam InResultType The type of the result produced by the command. Defaults to int32.
 */
template<typename InResultType = int32>
class TPreparedCliCommand final : public TSharedFromThis<TPreparedCliCommand<InResultType>>, public ICharonTask
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCommandSucceed, InResultType)
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCommandFailed, int32, FString)
private:
	enum struct ERunStatus
	{
		ReadyToRun,
		Running,
		Stopped,
		Succeed,
		Failed,
	};

	FText DisplayName;
	TSharedRef<FMonitoredProcess> Process;
	FString OutputFilePath;
	ENamedThreads::Type EventThread;
	std::atomic<ERunStatus> RunStatus;

	FSimpleMulticastDelegate TaskStart;
	FSimpleMulticastDelegate TaskSucceed;
	FSimpleMulticastDelegate TaskFailed;
	FOnCommandSucceed CommandSucceed;
	FOnCommandFailed CommandFailed;

public:
	/**
	 * Event that fires when the task is started.
	 *
	 * @return A reference to the FSimpleMulticastDelegate that can be used to bind event handlers for task start.
	 */
	virtual FSimpleMulticastDelegate& OnStart() override { return TaskStart; }
	/**
	 * Event that fires when the task succeeds.
	 *
	 * @return A reference to the FSimpleMulticastDelegate that can be used to bind event handlers for task success.
	 */
	virtual FSimpleMulticastDelegate& OnSucceed() override { return TaskSucceed; }
	/**
	 * Event that fires when the task fails.
	 *
	 * @return A reference to the FSimpleMulticastDelegate that can be used to bind event handlers for task failure.
	 */
	virtual FSimpleMulticastDelegate& OnFailed() override { return TaskFailed; }
	/**
	 * Event that fires when the command succeeds.
	 *
	 * @return FOnCommandSucceed event, providing the result of the command of type InResultType.
	 */
	FOnCommandSucceed& OnCommandSucceed() { return CommandSucceed; }
	/**
	 * Event that fires when the command fails.
	 *
	 * @return FOnCommandFailed event, providing the exit code and process's output.
	 */
	FOnCommandFailed& OnCommandFailed() { return CommandFailed; }
	/**
	 * Retrieves the display name of the task, primarily used for logging purposes.
     *
     * @return The display name of the task as FText.
     */
	virtual const FText& GetDisplayName() override { return DisplayName; }

	/**
	 * Constructs a TCharonCliCommand object.
	 *
	 * @param Process A shared reference to FMonitoredProcess, typically an instance of FCharonCliCommandRunner.
	 * @param DisplayName The display name of the command for logging purposes.
	 * @param OutputFilePath Optional file path where Charon.exe process will output results to be read into InResultType.
	 */
	TPreparedCliCommand(const TSharedRef<FMonitoredProcess>& Process, const FText& DisplayName, const FString& OutputFilePath = FString());

	/**
	 * Starts the command execution.
	 * If it returns false, then no events will be fired due to initial misconfiguration of the command, or if the command is already running or finished.
	 * This method should be called only once.
	 * @param EventDispatchThread The thread on which the events should be dispatched. Defaults to ENamedThreads::AnyThread.
	 * @return true if the task was started successfully, false otherwise.
	 */
	virtual bool Start(ENamedThreads::Type EventDispatchThread = ENamedThreads::AnyThread) override;

	/**
	 * Stops the command execution by aborting (SIGTERM) process. 
	 */
	virtual void Stop() override;

private:
	void OnProcessCompleted(int32 ExitCode);
	void OnProcessCancelled();
	void OnProcessOutput(FString Output);

	static bool TryReadResult(const FString& Output, int32 ExitCode, TSharedPtr<FJsonObject>& OutResult);
	static bool TryReadResult(const FString& Output, int32 ExitCode, TArray<TSharedPtr<FJsonValue>>& OutResult);
	static bool TryReadResult(const FString& Output, int32 ExitCode, int32& OutResult);
	static bool TryReadResult(const FString& Output, int32 ExitCode, FString& OutResult);
	static bool TryReadResult(const FString& Output, int32 ExitCode, FValidationReport& OutResult);
	static bool TryReadResult(const FString& Output, int32 ExitCode, FImportReport& OutResult);
};

inline DEFINE_LOG_CATEGORY(LogTCharonCliCommand);

template <typename InResultType>
TPreparedCliCommand<InResultType>::TPreparedCliCommand(
	const TSharedRef<FMonitoredProcess>& Process,
	const FText& DisplayName,
	const FString& OutputFilePath)
	: DisplayName(DisplayName), Process(Process), EventThread(ENamedThreads::AnyThread), RunStatus(ERunStatus::ReadyToRun)
{
	CommandSucceed.AddLambda([this](InResultType _) { TaskSucceed.Broadcast(); });
	CommandFailed.AddLambda([this](int32 _, FString __) { TaskFailed.Broadcast(); });
	this->OutputFilePath = OutputFilePath;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::Start(ENamedThreads::Type EventDispatchThread)
{
	ERunStatus ExpectedReadyToRun = ERunStatus::ReadyToRun;
	if (!RunStatus.compare_exchange_strong(ExpectedReadyToRun, ERunStatus::Running))
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Unable to run command because it is already running or finished."));
		return false;
	}

	const auto WeakThisPtr = this->AsWeak();
	EventThread = EventDispatchThread;
	BroadcastEvent(WeakThisPtr, TaskStart, EventThread);
	
	if (EventThread == ENamedThreads::AnyThread)
	{
		Process->OnCompleted().BindSP(this, &TPreparedCliCommand::OnProcessCompleted);
		Process->OnCanceled().BindSP(this, &TPreparedCliCommand::OnProcessCancelled);
	}
	else
	{
		Process->OnCompleted().BindLambda([WeakThisPtr, EventDispatchThread](int32 ExitCode)
		{
			AsyncTask(EventDispatchThread, [WeakThisPtr, ExitCode]
			{
				if (const auto ThisPtr = WeakThisPtr.Pin())
				{
					ThisPtr->OnProcessCompleted(ExitCode);
				}
			});
		});
		Process->OnCanceled().BindLambda([WeakThisPtr, EventDispatchThread]
		{
			AsyncTask(EventDispatchThread, [WeakThisPtr]
			{
				if (const auto ThisPtr = WeakThisPtr.Pin())
				{
					ThisPtr->OnProcessCancelled();
				}
			});
		});
	}
	Process->OnOutput().BindSP(this, &TPreparedCliCommand::OnProcessOutput);
	
	const bool bRunSuccess = Process->Launch();
	if(!bRunSuccess)
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to launch command because Unreal failed to create In/Out pipes or FPlatformProcess instance."));
		auto _ = Process->OnCanceled().ExecuteIfBound();
	}
	return bRunSuccess;
}

template <typename InResultType>
void TPreparedCliCommand<InResultType>::OnProcessCompleted(int32 ExitCode)
{
	ERunStatus ExpectedRunning = RunStatus.load();
	if (ExpectedRunning != ERunStatus::Running)
	{
		return; // already finished
	}
	
	UE_LOG(LogTCharonCliCommand, Log, TEXT("Command's '%s' process has been finished with exit code %d."), *DisplayName.ToString(), ExitCode);

	FString Output;
	if (!OutputFilePath.IsEmpty() && FPaths::FileExists(OutputFilePath))
	{
		FFileHelper::LoadFileToString(Output, *OutputFilePath);
	}
	
	InResultType Result;
	if (!TryReadResult(Output, ExitCode, Result))
	{
		if (!RunStatus.compare_exchange_strong(ExpectedRunning, ERunStatus::Failed))
		{
			return; // already finished
		}
		
		CommandFailed.Broadcast(ExitCode, Output);
	}
	else
	{
		if (!RunStatus.compare_exchange_strong(ExpectedRunning, ERunStatus::Succeed))
		{
			return; // already finished
		}
		
		CommandSucceed.Broadcast(Result);
	}
}

template <typename InResultType>
void TPreparedCliCommand<InResultType>::OnProcessCancelled()
{
	ERunStatus ExpectedRunning = ERunStatus::Running;
	if (!RunStatus.compare_exchange_strong(ExpectedRunning, ERunStatus::Stopped))
	{
		return; // already finished
	}
	
	UE_LOG(LogTCharonCliCommand, Log, TEXT("Command's '%s' process has been stopped or canceled by other means."), *DisplayName.ToString());

	CommandFailed.Broadcast(INT32_MIN, TEXT("Process launch failed or has been cancelled."));
}

template <typename InResultType>
void TPreparedCliCommand<InResultType>::OnProcessOutput(FString Output)
{
	auto SharedOutputRef = MakeShared<FString>(Output);
	const auto WeakThisPtr = this->AsWeak();
	AsyncTask(ENamedThreads::GameThread, [WeakThisPtr, SharedOutputRef]()
	{
		if (!WeakThisPtr.IsValid()) return;

		if (SharedOutputRef->Contains(TEXT("Error]")) || SharedOutputRef->Contains(TEXT("ERR]")))
		{
			UE_LOG(LogTCharonCliCommand, Error, TEXT("%s"), *SharedOutputRef.Get());
		}
		else if(SharedOutputRef->Contains(TEXT("Warning]")) || SharedOutputRef->Contains(TEXT("WARN]")))
		{
			UE_LOG(LogTCharonCliCommand, Warning, TEXT("%s"), *SharedOutputRef.Get());
		}
		else
		{
			UE_LOG(LogTCharonCliCommand, Log, TEXT("%s"), *SharedOutputRef.Get());
		}
	});
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, TSharedPtr<FJsonObject>& OutResult)
{
	if (ExitCode != 0)
	{
		return false;
	}
	
	const auto JsonReader = TJsonReaderFactory<>::Create(Output);
	TSharedPtr<FJsonValue> OutValue;
	
	if (!FJsonSerializer::Deserialize(JsonReader, OutValue) || !OutValue.IsValid())
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Output is not a valid JSON. Output: "));
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("%s"), *Output);
		return false;
	}
	
	TSharedPtr<FJsonObject>* OutObject;
	if (!OutValue->TryGetObject(OutObject) || !OutObject->IsValid())
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Result value is not a JSON object."));
		return false;
	}

	check(OutObject);
	OutResult = OutObject->ToSharedRef();
	return true;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, TArray<TSharedPtr<FJsonValue>>& OutResult)
{
	if (ExitCode != 0)
	{
		return false;
	}
	
	const auto JsonReader = TJsonReaderFactory<>::Create(Output);
	TSharedPtr<FJsonValue> OutValue;
	
	if (!FJsonSerializer::Deserialize(JsonReader, OutValue) || !OutValue.IsValid())
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Output is not a valid JSON. Output: "));
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("%s"), *Output);
		return false;
	}

	TArray<TSharedPtr<FJsonValue>>* OutArray = nullptr;
	if (!OutValue->TryGetArray(OutArray) || OutArray == nullptr)
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Result value is not a JSON array."));
		return false;
	}

	check(OutArray);
	
	OutResult = *OutArray;
	return true;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, int32& OutResult)
{
	if (ExitCode != 0)
	{
		return false;
	}
	
	OutResult = ExitCode;
	return true;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, FString& OutResult)
{
	if (ExitCode != 0)
	{
		return false;
	}

	OutResult = Output;
	return true;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, FValidationReport& OutResult)
{
	TSharedPtr<FJsonObject> Object;
	if (!TryReadResult(Output, ExitCode, Object))
	{
		return false;
	}
	check(Object.IsValid());
	
	FText FailReason;
	if(!FJsonObjectConverter::JsonObjectToUStruct<FValidationReport>(Object.ToSharedRef(), &OutResult, 0, 0, false, &FailReason))
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Result value is not valid 'FValidationReport' object. Reason: %s."), *FailReason.ToString());
		return false;
	}

	return true;
}

template <typename InResultType>
bool TPreparedCliCommand<InResultType>::TryReadResult(const FString& Output, int32 ExitCode, FImportReport& OutResult)
{
	TSharedPtr<FJsonObject> Object;
	if (!TryReadResult(Output, ExitCode, Object))
	{
		return false;
	}

	FText FailReason;
	if(!FJsonObjectConverter::JsonObjectToUStruct<FImportReport>(Object.ToSharedRef(), &OutResult, 0, 0, false, &FailReason))
	{
		UE_LOG(LogTCharonCliCommand, Warning, TEXT("Failed to read command's output as JSON. Result value is not valid 'FImportReport' object. Reason: %s."), *FailReason.ToString());
		return false;
	}
	return true;
}

template <typename InResultType>
void TPreparedCliCommand<InResultType>::Stop()
{
	Process->Stop();
}