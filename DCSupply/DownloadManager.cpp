#include "StdAfx.h"
#include "downloadmanager.h"

DownloadManager::DownloadManager(void)
{
}

DownloadManager::~DownloadManager(void)
{
}
//
//adds download to the download manager
void DownloadManager::AddDownload(DCDownload *download)
{
	//add download to vector
	v_downloads.push_back(download);
}

//
//removes download from download manager
void DownloadManager::RemoveDownloads()
{
	vector<DCDownload*>::iterator iter;
	TRACE("v_downloads.size() %d\n", v_downloads.size());
	if(v_downloads.size()>=1000)
	{
		v_downloads.clear();
	}
	//	return;
	//look for all downloads that have been disconnected by downloader
	/*for(iter = v_downloads.begin();iter != v_downloads.end();iter++)
	{
		if(!(*iter)->GetConnectionStatus())
		{
			v_downloads.erase(iter);
			//v_downloads.erase(iter);
		}
	}*/
	/*for(int counter=0;counter<v_downloads.size();counter++)
	{
		if(!v_downloads[counter].GetConnectionStatus())\
		{
			v_downloads.erase(counter);
		}
	}*/
}