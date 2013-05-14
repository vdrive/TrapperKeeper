#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class GnutellaDecoyerDll;
class SyncherDest :
	public TKSyncherInterface
{
public:
	SyncherDest(void);
	~SyncherDest(void);
	void InitParent(GnutellaDecoyerDll *parent,int id);
	void MapFinishedChanging(const char* source_ip);

	GnutellaDecoyerDll *p_parent;
	int m_id;

};
