#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tksyncher\tksyncherinterface.h"

class AutoUpdaterDll;

class AutoUpdaterSyncher : public TKSyncherInterface
{
public:
	AutoUpdaterSyncher();
	~AutoUpdaterSyncher(void);

	void InitParent(AutoUpdaterDll *parent);
	void MapFinishedChanging(const char* source_ip);

	AutoUpdaterDll *p_parent;
};
