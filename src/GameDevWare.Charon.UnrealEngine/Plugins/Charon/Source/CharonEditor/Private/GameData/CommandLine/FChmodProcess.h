// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "Misc/MonitoredProcess.h"

class CHARONEDITOR_API FChmodProcess : public FMonitoredProcess
{
public:
	FChmodProcess(const FString& FilePath, const FString& Permissions): FMonitoredProcess("", "", true, false)
	{
		URL = TEXT("/usr/bin/env");
		Params = FString::Format(TEXT(" -- \"chmod\" \"{1}\" \"{0}\""), { FilePath, Permissions });
	}
};
