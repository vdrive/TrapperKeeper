#pragma once
#include "..\tksyncher\tksyncherinterface.h"

class MediaManager;

class MediaSyncher :
	public TKSyncherInterface
{
public:
	MediaSyncher(void);
	~MediaSyncher(void);

	void InitParent(MediaManager * parent);


private:
	void MapFinishedChanging(const char* source_ip);

	vector <TKSyncherMap::TKFile> v_template_files;
	MediaManager * p_parent;
};
