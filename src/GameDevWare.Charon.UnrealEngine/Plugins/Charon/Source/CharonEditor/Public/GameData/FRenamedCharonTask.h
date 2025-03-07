#pragma once
#include "ICharonTask.h"

class FRenamedCharonTask final : public ICharonTask
{
private:
	const TSharedRef<ICharonTask> Inner;
	FText DisplayName;
	
public:
	FRenamedCharonTask(const TSharedRef<ICharonTask>& Inner, const FText& DisplayName) : Inner(Inner), DisplayName(DisplayName) {}
	
	virtual const FText& GetDisplayName() override { return DisplayName; }
	virtual bool Start(ENamedThreads::Type EventDispatchThread = ENamedThreads::AnyThread) override { return Inner->Start(EventDispatchThread); }
	virtual void Stop() override{ Inner->Stop(); }
	virtual FSimpleMulticastDelegate& OnStart() override { return Inner->OnStart(); }
	virtual FSimpleMulticastDelegate& OnSucceed() override { return Inner->OnSucceed(); }
	virtual FSimpleMulticastDelegate& OnFailed() override { return Inner->OnFailed(); }
};
