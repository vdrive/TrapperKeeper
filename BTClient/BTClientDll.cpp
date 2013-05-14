#include "StdAfx.h"
#include "BTClientDll.h"
#include <io.h>
#include <iostream>
#include "BTDataHeader.h"

//
//
//
BTClientDll::BTClientDll(void)
{
}

//
//
//
BTClientDll::~BTClientDll(void)
{
}

//
//
//

void BTClientDll::TimerFired(UINT nIDEvent)
{

	switch(nIDEvent)
	{
		case 1:	
		{
			WriteStatsToFile();

			break;
		}

		case 2:
		{
			m_dlg.DisplayClientData(v_client_data);
			m_dlg.DisplayModuleData(v_mod_data);
			break;
		}
	
	}
}
void BTClientDll::DllInitialize()
{
	m_dlg.Create(IDD_BTCLIENT_DLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTClientDll::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}

	if (p_data_com != NULL)
	{
		delete p_data_com;
	}

}

//
//
//
void BTClientDll::DllStart()
{

	p_com = new BTClientComInterface();
	p_data_com = new BTClientComInterface();

	p_com->InitParent(this);
	p_data_com->InitParent(this);

	p_data_com->Register(this,235);// BTCollectorid
	p_com->Register(this,891);// BTScraper id


	m_dlg.InitParent(this);

	m_manager.InitParent(this);
	

	//10 minute timer for the stats file.
	m_dlg.SetTimer(1,10*60*1000,NULL);

	//30 second timer for the stats dialog refresh
	m_dlg.SetTimer(2,2*60*1000,NULL);

	m_manager.ReadConfig();

}

//
//
//
void BTClientDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	DisplayInfo("Dialog Displayed");



	//m_dlg.SetTimer(1,10*60*1000,NULL);

	//Testing Area
	//ReadTorrents();
}

//
//
//

void BTClientDll::SpawnProjectSession(TorrentFile * torrent)
{

	m_manager.StartNewTorrent(torrent);
	// Delete this when you close the connections
	/*
	CClientConnection *connection = new CClientConnection;
	connection->StartConnections(torrent);
	v_connections.push_back(connection);
	*/

}

void BTClientDll::DataReceived(char *source_ip, void *data, UINT data_length)
{

	BTDataHeader * bheader = (BTDataHeader *)data;

	if (bheader->m_appid == 235)
	{
		if (bheader->m_opcode == BTDATAHEADER_REQUEST)
		{
			StatusRequestReceived(source_ip);
			return;
		}
		else
		{
			return;
		}
	}

	DisplayInfo("Data Received");
	TorrentFile thetorrent(data,data_length);
	PeerList * peers = thetorrent.GetPeers();
	int numpeers = peers->GetCount();

	char  msg[1024];
	sprintf(msg,"Got Data for torrent %s with %d peers",thetorrent.GetName().c_str(),numpeers);
	m_dlg.DisplayInfo(msg);

	if (numpeers > 0)
	{
		SpawnProjectSession(&thetorrent);
	}


}

void BTClientDll::StatusRequestReceived(char * source_name)
{

	int size = ((int)v_client_data.size() * sizeof(ClientData) + sizeof(BTDataHeader));
	char * data = new char[size];
	char * ptr = data;
	BTDataHeader header;
	header.m_appid = 235;
	header.m_opcode = BTDATAHEADER_RETURN;
	memcpy(ptr,&header,sizeof(BTDataHeader));
	ptr += sizeof(BTDataHeader);
	for (int i = 0; i < (int)v_client_data.size(); i++)
	{
		ClientData tempdata = v_client_data[i];
		memcpy(ptr,&tempdata,sizeof(ClientData));
		ptr += sizeof(ClientData);
	}

	p_data_com->SendReliableData(source_name,data,size);
	delete [] data;

}

void BTClientDll::DisplayInfo(char * info)
{
	m_dlg.DisplayInfo(info);
}


void BTClientDll::ReadTorrents()
{
	vector<string> filenames;
	TorrentFile tfile;
	ExploreFolder("F:/torrenttest/",&filenames);
	char fname[256];

	for (int i = 0; i < (int)filenames.size(); i++)
	{
		strcpy(fname,filenames[i].c_str());
		tfile.ReadInTorrentFile(fname);
		SpawnProjectSession(&tfile);

	}
}

void BTClientDll::ExploreFolder(char * folder, vector<string> * filenames)
{
	char buf[4096];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	string tempname;
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			tempname = folder;
			tempname +=data.name;
			tempname += "/";

			ExploreFolder((char *)tempname.c_str(),filenames);
			data.size;

		}
	}
	else
	{
		tempname = folder;
		tempname += data.name;

		filenames->push_back(tempname);
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				tempname = folder;
				tempname +=data.name;
				tempname += "/";

				ExploreFolder((char * )tempname.c_str(),filenames);
				data.size;

			}
		}

		else
		{
			tempname = folder;
			tempname += data.name;

			filenames->push_back(tempname);
		}
	}
	_findclose(handle);
}

void BTClientDll::ClientDataReported(ClientData *cdata)
{
	ClientData tempdata;
	/*
	tempdata.m_bytes = cdata->m_bytes;
	strcpy(tempdata.m_ip,cdata->m_ip);
	strcpy(tempdata.m_torrentname,cdata->m_torrentname);
	*/
	tempdata = *cdata;
	int found = 0;
	for (int i = 0; i<(int)v_client_data.size(); i++)
	{
		if (strcmp(tempdata.m_ip,v_client_data[i].m_ip) == 0)
		{
			found = 1;
			v_client_data[i].m_bytes += tempdata.m_bytes;
			v_client_data[i].m_bytes_sent += tempdata.m_bytes_sent;
			v_client_data[i].m_last_time = CTime::GetCurrentTime();
			break;
		}
	}

	if (found == 0)
	{
		tempdata.m_first_time = CTime::GetCurrentTime();
		tempdata.m_last_time = CTime::GetCurrentTime();
		v_client_data.push_back(tempdata);
	}	
	
	//m_dlg.DisplayClientData(v_client_data);

}

void BTClientDll::ModuleDataReported(ModuleData *cdata)
{

	ModuleData tempdata;
	tempdata = *cdata;
	int found = 0;
	for (int i = 0; i<(int)v_mod_data.size(); i++)
	{
		if (tempdata.modnum == v_mod_data[i].modnum)
		{
			found = 1;
			v_mod_data[i].v_ips.clear();
			for (int j = 0; j< (int)tempdata.v_ips.size(); j++)
			{
				v_mod_data[i].v_ips.push_back(tempdata.v_ips[j]);
			}
			break;
		}
	}

	if (found == 0)
	{
		v_mod_data.push_back(tempdata);
	}	



}

void BTClientDll::WriteStatsToFile()
{

	CFile logfile;
	
	if (logfile.Open("BTDownloadLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)!= TRUE)
	{
	 
		return;

	}
	logfile.SeekToEnd();
	char msg[512+1];
	for (int i = 0; i<(int)v_client_data.size(); i++)
	{
		sprintf(msg,"Received a total of %d bytes from ip %s \n",v_client_data[i].m_bytes,v_client_data[i].m_ip);
		logfile.Write(msg,(int)strlen(msg));
	}
	logfile.Close();

}

void BTClientDll::RemoveModule(int modnum)
{

	for(int i=0;i<(int) v_mod_data.size();i++)
	{
		int killmodnum =  v_mod_data[i].modnum;
		if (killmodnum == modnum)
		{
			v_mod_data.erase(v_mod_data.begin()+i);
			return;
		}

	}
}