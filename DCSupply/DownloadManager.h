#pragma once
#include "DCDownload.h"

//class that takes care of all the downloads for the client
class DownloadManager
{
public:
	DownloadManager(void);
	~DownloadManager(void);
	void AddDownload(DCDownload *download);		//adds download to vector
	void RemoveDownloads();		//checks which downloads need to be removed from vector
private:
	vector<DCDownload*> v_downloads;	//vector of downloads that need to be taken care of

};
