#include "StdAfx.h"
#include "downloadmanager.h"

DownloadManager::DownloadManager(void)
{
	download = new DCDownload[DOWNLOADS];
	position = 0;
	m_initialized = false;
}

DownloadManager::~DownloadManager(void)
{
	this->DisconnectAll();
	delete [] download;
}
//
//adds download to the download manager
void DownloadManager::AddDownload(char* host,unsigned port,char* data)
{
	if(position>=DOWNLOADS-1)
	{
		m_initialized = true;
		position = 0;
	}
	//DCDownload *download = new DCDownload;
	download[position].Disconnect();
	download[position].GetLoginData(data);
	download[position].GetDBConnection(p_db);
	download[position].DownloadConnect(host,port,m_initialized);
	/*for(int x=0;x<50;x++)
	{
		download[x].SendJunk();
	}
	if(m_initialized)
	{
		v_downloads.push_back(download);
	}*/
	//downloads[position].Close();
	//int status = downloads[position].DownloadConnect(host,port);
	//downloads[position].GetLoginData(data);
	//add download to vector
	//v_downloads.push_back(downloads[position]);
	position++;
}

//
//adds a passive download to the vector
int DownloadManager::AddPassiveDownload()
{
	int listening_port;
	if(position>=DOWNLOADS-1)
	{
		m_initialized = true;
		position = 0;
	}
	download[position].Disconnect();
	download[position].GetDBConnection(p_db);
	listening_port = download[position].CreatePort();
	while(listening_port!=0)
	{
		listening_port = download[position].CreatePort();
	}

	/*if(m_initialized)
	{
		v_downloads.push_back(download);
	}*/

	return download[position].GetPort();
}
//
//increments the download position
void DownloadManager::IncrementDownloadPosition()
{
	if(position>=DOWNLOADS-1)
	{
		m_initialized = true;
		position = 0;
	}
	else
	{
		position++;
	}
}
//

void DownloadManager::GetDBConnection(DB *db)
{
	p_db = db;
}

void DownloadManager::SendJunk()
{
	for(int x=0;x<DOWNLOADS;x++)
	{
		download[x].SendJunk();
	}
}

void DownloadManager::DisconnectAll()
{
	for(int x=0;x<DOWNLOADS;x++)
	{
		download[x].Close();
	}
}