#include "stdafx.h"
#include "swarmersyncher.h"
#include "mediaswarmer.h"

MediaSyncher::MediaSyncher(void)
{
}

MediaSyncher::~MediaSyncher(void)
{
}

void MediaSyncher::InitParent( MediaSwarmer * parent)
{
	p_parent = parent;

}


//
//
//
void MediaSyncher::MapFinishedChanging(const char * source_ip)
{

	// Read directory here get all files
	p_parent->ReadDestDir();

	vector <TKSyncherMap::TKFile> v_files;

	TKSyncherMap temp_map = RetrieveCurrentMap();
	temp_map.EnumerateAllFiles(v_files);

}