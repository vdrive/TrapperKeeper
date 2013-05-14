#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class OvernetSpooferDll;
class SyncherDest :
	public TKSyncherInterface
{
public:
	SyncherDest(void);
	~SyncherDest(void);
	void InitParent(OvernetSpooferDll *parent);
	void MapFinishedChanging(const char* source_ip);

	OvernetSpooferDll *p_parent;
	int m_id;

};
