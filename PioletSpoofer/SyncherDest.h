#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class PioletSpooferDll;
class SyncherDest :
	public TKSyncherInterface
{
public:
	SyncherDest(void);
	~SyncherDest(void);
	void InitParent(PioletSpooferDll *parent);
	void MapFinishedChanging(const char* source_ip);

	PioletSpooferDll *p_parent;
	int m_id;

};
