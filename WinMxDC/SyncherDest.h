#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class WinMxDcDll;
class SyncherDest :
	public TKSyncherInterface
{
public:
	SyncherDest(void);
	~SyncherDest(void);
	void InitParent(WinMxDcDll *parent);
	void MapFinishedChanging(const char* source_ip);

	WinMxDcDll *p_parent;
	int m_id;

};
