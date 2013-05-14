#include "stdafx.h"
#include "mediasyncher.h"
#include "mediamanager.h"

MediaSyncher::MediaSyncher(void)
{
}

MediaSyncher::~MediaSyncher(void)
{
}

void MediaSyncher::InitParent( MediaManager * parent)
{
	p_parent = parent;

}


//
//
//
void MediaSyncher::MapFinishedChanging(const char * source_ip)
{
	p_parent->DisplayInfo("Synching Finished, beginning file creation/Deletion");

	vector <TKSyncherMap::TKFile> v_files;

	TKSyncherMap temp_map = RetrieveCurrentMap();
	temp_map.EnumerateAllFiles(v_files);
	v_template_files.clear();
	// Strip out the template files from the list of files
	for (int i = 0; i < (int)v_files.size(); i++)
	{
		if (strstr(v_files[i].m_name.c_str(),"template"))
		{
			v_template_files.push_back(v_files[i]);
		}

	}
	// Compare the templates to see if anything needs to be made
	p_parent->CompareMapsTemplates(v_template_files);

	p_parent->KillAllKazaa();
	p_parent->CheckMapsForDeletion(v_files);
	p_parent->RelaunchAllKazaa();

}