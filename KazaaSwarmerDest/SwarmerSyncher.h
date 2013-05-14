#pragma once
#include "..\tksyncher\tksyncherinterface.h"

class MediaSwarmer;

class MediaSyncher :
	public TKSyncherInterface
{
public:
	MediaSyncher(void);
	~MediaSyncher(void);

	void InitParent(MediaSwarmer * parent);


private:
	void MapFinishedChanging(const char* source_ip);
	void MapHasChanged(const char* source_ip);


	MediaSwarmer * p_parent;
};
