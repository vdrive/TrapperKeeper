#pragma once
#include "../TkSyncher/TKSyncherInterface.h"

class KazaaControllerDestSyncher :
	public TKSyncherInterface
{
public:
	KazaaControllerDestSyncher(void);
	~KazaaControllerDestSyncher(void);
	void MapFinishedChanging(const char* source_ip);
};
