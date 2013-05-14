#pragma once
#include "DCDownload.h"
#include "DB.h"
#define DOWNLOADS 300		//simultaneous downloads
//class that takes care of all the downloads for the client
class DownloadManager
{
public:
	DownloadManager(void);
	~DownloadManager(void);
	void AddDownload(char *host,unsigned int port,char* data);		//adds download to vector
	int AddPassiveDownload();	//adds a passive download to the vector
	void GetDBConnection(DB *db);	//get a pointer to the ONE database connection initiated in dialog
	void SendJunk();
	void IncrementDownloadPosition();
	void DisconnectAll();
private:
	//vector<DCDownload*> v_downloads;	//vector of downloads that need to be taken care of
	DCDownload* download;
	bool m_initialized;
	int position;
	DB * p_db;
};
