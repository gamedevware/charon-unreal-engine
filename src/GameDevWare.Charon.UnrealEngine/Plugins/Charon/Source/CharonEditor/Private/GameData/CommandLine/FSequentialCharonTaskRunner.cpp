// Copyright GameDevWare, Denis Zykov 2025

#include "FSequentialCharonTaskRunner.h"

#include "Async/Async.h"

DEFINE_LOG_CATEGORY(LogFSequentialCharonTaskRunner);


FSequentialCharonTaskRunner::FSequentialCharonTaskRunner(const TArray<TSharedRef<ICharonTask>>& Tasks, ETaskFailureHandling FailureHandling)
	: EventThread(ENamedThreads::AnyThread), TaskList(Tasks), FailureHandlingMode(FailureHandling), CurrentTaskIndex(-1) 
{
	DisplayName = FText::Format(INVTEXT("Tasks [{0}]"), TaskList.Num());
}

bool FSequentialCharonTaskRunner::Start(ENamedThreads::Type EventDispatchThread)
{
	int32 Expected = -1;
	if (!CurrentTaskIndex.compare_exchange_strong(Expected, 0))
	{
		UE_LOG(LogFSequentialCharonTaskRunner, Warning, TEXT("Unable to run sequential tasks because FSequentialCharonTaskRunner is already running or finished."));
		return false;
	}

	EventThread = EventDispatchThread;
	BroadcastEvent(AsWeak(), TaskStart, EventThread);
	
	RunTask(0);
	return true;
}

void FSequentialCharonTaskRunner::RunTask(int32 TasksIndex)
{
	if (TasksIndex >= TaskList.Num()) // it was last task
	{
		BroadcastEvent(AsWeak(), TaskSucceed, EventThread);
		return;
	}
	
	const auto Task = TaskList[TasksIndex];
	const auto WeakThisPtr = this->AsWeak();
	const auto NextTaskFn = [WeakThisPtr, this](bool Success)
	{
		const auto ThisPtr = WeakThisPtr.Pin();
		if (!ThisPtr.IsValid()) { return; }

		if (!Success && FailureHandlingMode == ETaskFailureHandling::FailOnFirstError)
		{
			const int32 CurrentIndex = CurrentTaskIndex.exchange(INT32_MIN);
			if (CurrentIndex < 0 || CurrentIndex >= TaskList.Num())
			{
				return; // already done
			}
			
			BroadcastEvent(AsWeak(), TaskFailed, EventThread);
			return;
		}
		
		const int32 NextIndex = CurrentTaskIndex.fetch_add(1) + 1;
		if (NextIndex < 0)
		{
			return; // Stopped
		}
		RunTask(NextIndex);
	};

	Task->OnSucceed().AddLambda(NextTaskFn, true);
	Task->OnFailed().AddLambda(NextTaskFn, false);
	
	if (!Task->Start(EventThread))
	{
		UE_LOG(LogFSequentialCharonTaskRunner, Warning, TEXT("Sequential task #%d '%s' has failed to start."), TasksIndex, *Task->GetDisplayName().ToString());
		NextTaskFn(/*Success*/false);
	}
	else
	{
		UE_LOG(LogFSequentialCharonTaskRunner, Verbose, TEXT("Running sequential task #%d '%s'..."), TasksIndex, *Task->GetDisplayName().ToString());
	}
}


void FSequentialCharonTaskRunner::Stop()
{
	const int32 CurrentIndex = CurrentTaskIndex.exchange(INT32_MIN);
	if (CurrentIndex < 0 || CurrentIndex >= TaskList.Num())
	{
		return; // already done
	}
	
	TaskList[CurrentIndex]->Stop();
	BroadcastEvent(AsWeak(), TaskFailed, EventThread);
}

TSharedRef<ICharonTask> ICharonTask::AsSequentialRunner(const TArray<TSharedRef<ICharonTask>>& Tasks, ETaskFailureHandling FailureHandling)
{
	return MakeShared<FSequentialCharonTaskRunner>(Tasks, FailureHandling);
}
