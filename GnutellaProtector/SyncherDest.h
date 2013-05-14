#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class GnutellaProtectorDll;
class SyncherDest :
	public TKSyncherInterface
{
public:
	SyncherDest(void);
	~SyncherDest(void);
	void InitParent(GnutellaProtectorDll *parent);
	void MapFinishedChanging(const char* source_ip);

	GnutellaProtectorDll *p_parent;
	int m_id;

};
