// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "Misc/MonitoredProcess.h"

class FChmodProcess : public FMonitoredProcess
{
public:
	FChmodProcess(const FString& FilePath, const FString& Permissions): FMonitoredProcess("", "", true, true)
	{
		URL = TEXT("/usr/bin/env");
		Params = FString::Format(TEXT(" -- \"chmod\" \"{1}\" \"{2}\""), { FilePath, Permissions });
	}
};
