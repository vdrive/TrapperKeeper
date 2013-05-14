#pragma once
#include "tksyncherinterface.h"

class ExecutableSource :
	public TKSyncherInterface
{
public:
	ExecutableSource(void);
	~ExecutableSource(void);

	string GetExtension(const char* file_name);
	void MapFinishedChanging(const char* source_ip); //override this to handle notification that the map has finished changing.
	void EnumerateDllsInDirectory(const char* directory,vector <string> &v_files);
};
