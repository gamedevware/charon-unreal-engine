// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "GameData/ICharonTask.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFSequentialCharonTaskRunner, Log, All);

class CHARONEDITOR_API FSequentialCharonTaskRunner : public TSharedFromThis<FSequentialCharonTaskRunner>, public ICharonTask
{
private:
	FText DisplayName;
	FSimpleMulticastDelegate TaskStart;
	FSimpleMulticastDelegate TaskSucceed;
	FSimpleMulticastDelegate TaskFailed;
	ENamedThreads::Type EventThread;
	const TArray<TSharedRef<ICharonTask>> TaskList;
	ETaskFailureHandling FailureHandlingMode;
	std::atomic<int32> CurrentTaskIndex;

	void RunTask(int32 TasksIndex);
public:
	virtual const FText& GetDisplayName() override { return DisplayName; }
	
	explicit FSequentialCharonTaskRunner(const TArray<TSharedRef<ICharonTask>>& Tasks, ETaskFailureHandling FailureHandling);

	virtual bool Start(ENamedThreads::Type EventDispatchThread = ENamedThreads::AnyThread) override;
	virtual void Stop() override;

	virtual FSimpleMulticastDelegate& OnStart() override { return TaskStart; }
	virtual FSimpleMulticastDelegate& OnSucceed() override { return TaskSucceed; }
	virtual FSimpleMulticastDelegate& OnFailed() override { return TaskFailed; }
};
