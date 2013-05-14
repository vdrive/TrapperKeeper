#include "StdAfx.h"
#include "BTInflatorDll.h"
#include "BTIPDataHeader.h"
#include "Torrent.h"
#include "TorrentFileData.h"

//
//
//
BTInflatorDll::BTInflatorDll(void)
{

}

//
//
//
BTInflatorDll::~BTInflatorDll(void)
{
}

//
//
//
void BTInflatorDll::DllInitialize()
{
	m_dlg.Create(IDD_Inflation_DLG,CWnd::GetDesktopWindow());
	m_next_port = 6982;
}

//
//
//
void BTInflatorDll::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}

}

//
//
//
void BTInflatorDll::DllStart()
{

	p_com = new BTInflatorCom();

	p_com->InitParent(this);
	p_com->Register(this,6824);

	CTime t = CTime::GetCurrentTime();
	srand((unsigned)int(t.GetTime()));

	m_dlg.InitParent(this);
	m_dlg.SetTimer(1,1*60*1000,NULL);
	m_dlg.SetTimer(2,5*60*1000,NULL);
	m_dlg.SetTimer(3,20*60*1000,NULL);

	m_manager.InitParent(this);

}

//
//
//
void BTInflatorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	//m_dlg.RefreshData();
}

//
//
//


void BTInflatorDll::TimerFired(UINT nIDEvent)
{

	switch(nIDEvent)
	{
		case 1:
		{

			for (int i = 0; i < (int)v_torrents.size(); i++)
			{
				if (v_torrents[i].IsReady())
				{
					//m_dlg.KillTimer(1);  // Temporary for debugging
					ServiceTorrent(&v_torrents[i]);

				}
			}
			
			break;
		}
		case 2:
		{
			
			m_dlg.RefreshData();
			for (int i = 0; i < (int)v_torrents.size(); i++)
			{
				m_dlg.UpdateTorrentData(v_torrents[i]);
			}
			break;
		}
		case 3:
		{
			m_manager.ClearStaleSockets();
		}
	}
}

void BTInflatorDll::ServiceTorrent(Torrent * the_torrent)
{
	the_torrent->SetAttemptTime();
	m_manager.MakeTorrentConnections(the_torrent);

}

/*
void BTInflatorDll::ServiceNewTorrent(Torrent * the_torrent)
{
	the_torrent->SetAttemptTime();
	m_manager.MakeTorrentConnections(the_torrent);

}
*/

void BTInflatorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	BTIPDataHeader * header = (BTIPDataHeader *)data;
	switch(header->appcode)
	{
		case BTIPDATA_PING:
		{
			SendPong(source_name);
			break;
		}

		case BTIPDATA_LOW_SEED_TORRENT:	
		{
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			TorrentFileData data;
			data.ReadFromBuffer(ptr);

			Torrent temp;
			int numseeds = 5;
			int numpeers = 2;
			temp.InitTorrent(data.m_name,data.m_announce_URL,data.m_hash,numseeds,numpeers,m_next_port);
			//temp.m_announce_url = data.m_announce_URL;
			//temp.m_info_hash = data.m_hash;

			bool found = false;
			for (int i = 0; i < (int)v_torrents.size(); i++)
			{

				if (v_torrents[i] == temp)
				{
					found = true;
					break;
				}
					//v_torrents[i].GetTorrentInfo(
				
			}


			if (found == false)
			{
				int torsize = sizeof(Torrent);
				int datasize = sizeof(TorrentFileData);
				v_torrents.push_back(temp);
				m_next_port+= (numseeds + numpeers);

			}

			break;
		}

		case BTIPDATA_STOP_SINGLE_INFLATION:
		{
			
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			TorrentFileData data;
			data.ReadFromBuffer(ptr);


			Torrent temp;
			temp.InitTorrent(data.m_name,data.m_announce_URL,data.m_hash,10,2,m_next_port);

			for (int i = 0; i < (int)v_torrents.size(); i++)
			{

				if (v_torrents[i] == temp)
				{
					v_torrents.erase(v_torrents.begin() + i );
					break;
				}
					//v_torrents[i].GetTorrentInfo(
				
			}
		
			break;
		}

	}

	
 }

void BTInflatorDll::TorrentFailure(string info_hash,string announce_url,string peer_id)
{

	Torrent temptorrent;
	temptorrent.InitTorrent(" ",announce_url,info_hash,0,0,0);

	for (int i = 0; i < (int)v_torrents.size(); i++)
	{
		if (v_torrents[i] == temptorrent)
		{
 			v_torrents[i].UpdatePeerFailure(peer_id);
		}
	}



}

void BTInflatorDll::TorrentSuccess(string info_hash,string announce_url,string peer_id)
{
	Torrent temptorrent;
	temptorrent.InitTorrent(" ",announce_url,info_hash,0,0,0);

	for (int i = 0; i < (int)v_torrents.size(); i++)
	{
		if (v_torrents[i] == temptorrent)
		{
			v_torrents[i].UpdatePeerConnect(peer_id);
		}
	}


}

void BTInflatorDll::SendPong(char *source_name)
{

	char dest[128+1];
	BTIPDataHeader header;

	header.appcode = BTIPDATA_PONG;
	char * data = new char[sizeof(BTIPDataHeader)];
	header.length = 0;
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	int size = sizeof(BTIPDataHeader);

	strcpy(dest,source_name);
	p_com->SendReliableData(dest,data,size);

	delete [] data;

}
