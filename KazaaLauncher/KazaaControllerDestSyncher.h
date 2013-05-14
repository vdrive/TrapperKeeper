#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class KazaaLauncherDll;
class KazaaControllerDestSyncher :
	public TKSyncherInterface
{
public:
	KazaaControllerDestSyncher(void);
	~KazaaControllerDestSyncher(void);
	void InitParent(KazaaLauncherDll *parent);
	void MapFinishedChanging(const char* source_ip);

	KazaaLauncherDll *p_parent;
};
