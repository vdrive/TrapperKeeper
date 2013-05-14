#include "StdAfx.h"
#include "BTTorrentSourceDll.h"
#include "BTIpDataHeader.h"
//
//
//
BTTorrentSourceDll::BTTorrentSourceDll(void)
{
}

//
//
//
BTTorrentSourceDll::~BTTorrentSourceDll(void)
{
}

//
//
//
void BTTorrentSourceDll::DllInitialize()
{
	m_dlg.Create(IDD_BT_TORRENT_SRC_DLG,CWnd::GetDesktopWindow());

}

//
//
//
void BTTorrentSourceDll::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}

}

//
//
//
void BTTorrentSourceDll::DllStart()
{

	p_com = new BTTorrentSourceCom();

	p_com->InitParent(this);
	p_com->Register(this,6824);

	m_dlg.InitParent(this);

	m_dlg.SetTimer(1,10*60*1000,NULL);  //real
	//m_dlg.SetTimer(1,1*60*1000,NULL);
	m_dlg.SetTimer(2,60*60*1000,NULL);
	PingAllDestinations();
}

//
//
//
void BTTorrentSourceDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	/*
	vector<TorrentFileData> v_test_list;
	m_db.GetFullTorrentInfo(&v_test_list);
	*/
	SendHashIDList();
}

//
//
//
void BTTorrentSourceDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	BTIPDataHeader * header = (BTIPDataHeader *)data;
	switch(header->appcode)
	{
		case BTIPDATA_PONG:	
		{
			string source = source_name;
			v_send_list.push_back(source);

			break;
		}

		case BTIPDATA_IP_LIST:	
		{
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			BTIPList iplist;
			iplist.ReadFromBuffer(ptr);
			m_db.InsertIPs(iplist);

			break;
		}
		case BTIPDATA_TORRENT_REQUEST:	
		{
			
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			int numdata = header->length/sizeof(HashIdPair);
			HashIdPair * temphash;
			vector<HashIdPair> v_hashes;
			int i = 0, j = 0, k = 0;

			for (i = 0; i < numdata; i++)
			{
				temphash = (HashIdPair *)ptr;
				v_hashes.push_back(*temphash);
				ptr+= sizeof(HashIdPair);

			}

			vector<TorrentFileData> vt_data;
			vector<BTIPList> vt_ips;
			m_db.GetTorrentInfoSubset(&vt_data,v_hashes,&vt_ips);

			for (j = 0; j < (int)vt_data.size(); j++)
			{
				SendRequestedTorrentData(vt_data[j],source_name);
			}
			/*
			for (k = 0; k < (int)vt_ips.size(); j++)
			{
				SendRequestedTorrentIPs(vt_ips[j],source_name);
			}
			*/
			v_hashes.clear();
			vt_ips.clear();
			vt_data.clear();

			break;
		}
		case BTIPDATA_IP_REQUEST:
		{
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			int numdata = header->length/sizeof(HashIdPair);
			HashIdPair * temphash;
			vector<HashIdPair> v_hashes;
			int i = 0;

			for (i = 0; i < numdata; i++)
			{
				temphash = (HashIdPair *)ptr;
				v_hashes.push_back(*temphash);
				ptr+= sizeof(HashIdPair);

			}
			vector<BTIPList> vt_ips;
			m_db.GetIPListSubset(v_hashes,&vt_ips);

			for (i = 0; i < (int)vt_ips.size(); i++)
			{
				SendRequestedTorrentIPs(vt_ips[i],source_name);
			}
			vt_ips.clear();
			v_hashes.clear();


		}
	}
}

void BTTorrentSourceDll::SendTorrentData()
{
	vector<string> v_send_list;

	m_name_server.RequestIP("BTTorrentData",v_send_list);

	char dest[128+1];


	BTIPDataHeader header;

	header.appcode = BTIPDATA_TORRENT;

	TorrentFileData tempdata;
	for (int j = 0; j < (int)m_torrent_data.size(); j ++)
	{

		tempdata.m_hash = m_torrent_data[j].m_hash;
		tempdata.m_announce_URL = m_torrent_data[j].m_announce_URL;
		tempdata.m_torrent_id = m_torrent_data[j].m_torrent_id;
		char * data = new char[sizeof(BTIPDataHeader)+tempdata.GetBufferLength()];
		header.length = tempdata.GetBufferLength();
		char * ptr = data;
		memcpy(ptr,&header,sizeof(BTIPDataHeader));
		ptr += sizeof(BTIPDataHeader);
		tempdata.WriteToBuffer(ptr);
		int size = sizeof(BTIPDataHeader)+tempdata.GetBufferLength();
		
		for (int i = 0; i < (int)v_send_list.size();i++)
		{
			strcpy(dest,v_send_list[i].c_str());
			p_com->SendReliableData(dest,data,size);
		}
		delete [] data;

	}


}

void BTTorrentSourceDll::TimerFired(UINT nIDEvent)
{

	switch(nIDEvent)
	{
		case 1:	
		{
			// IP Collection
/*			
			m_db.GetIPInfo(&m_torrent_data);
			SendTorrentData();
			m_torrent_data.clear();
	*/

			// SEED Inflation
			
			vector<TorrentFileData>  torrent_data;
			m_low_seed_data.clear();
			m_db.GetLowSeedTorrentInfo(&m_low_seed_data);

			SendLowSeedTorrentData(m_low_seed_data);


			m_dlg.RefreshList(m_low_seed_data);
			//SendHashIDList();
			break;
		}

		case 2:
		{

			PingAllDestinations();

			break;

		}

	
	}


}

void BTTorrentSourceDll::AddNewLowSeedTorrent(TorrentFileData tdata)
{
	m_low_seed_data.push_back(tdata);
	m_db.AddLowSeedTorrent(tdata);
	m_dlg.RefreshList(m_low_seed_data);
}


void BTTorrentSourceDll::SendRequestedTorrentData(TorrentFileData tdata, char * dest_name)
{



	BTIPDataHeader header;

	header.appcode = BTIPDATA_TORRENT;


	char * data = new char[sizeof(BTIPDataHeader)+tdata.GetBufferLength()];
	header.length = tdata.GetBufferLength();
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	tdata.WriteToBuffer(ptr);
	int size = sizeof(BTIPDataHeader)+tdata.GetBufferLength();

	p_com->SendReliableData(dest_name,data,size);

	delete [] data;

}

void BTTorrentSourceDll::SendRequestedTorrentIPs(BTIPList vips, char * dest_name)
{

	BTIPDataHeader header;

	header.appcode = BTIPDATA_IP_LIST;
	int size = sizeof(BTIPDataHeader)+vips.GetBufferLength();
	char * data = new char[size];
	header.length = vips.GetBufferLength();
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	vips.WriteToBuffer(ptr);

	p_com->SendReliableData(dest_name,data,size);

	delete [] data;

}

void BTTorrentSourceDll::SendHashIDList()
{

//	vector<string> v_send_list;

//	m_name_server.RequestIP("BTInterdictor",v_send_list);

	char dest[128+1];


	BTIPDataHeader header;

	header.appcode = BTIPDATA_HASH_LIST;

	vector<HashIdPair> v_hashes;

	m_db.GetHashList(&v_hashes);
	header.length = (int)v_hashes.size() * sizeof(HashIdPair);
	char * data = new char[sizeof(BTIPDataHeader) + header.length];
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	for (int j = 0; j < (int)v_hashes.size(); j++)
	{
		memcpy (ptr,&v_hashes[j],sizeof(HashIdPair));
		ptr += sizeof(HashIdPair);
		
	}



	int size = sizeof(BTIPDataHeader) + header.length;
		
	for (int i = 0; i < (int)v_send_list.size();i++)
	{
		strcpy(dest,v_send_list[i].c_str());
		p_com->SendReliableData(dest,data,size);
	}
	delete [] data;


}

void BTTorrentSourceDll::SendLowSeedTorrentData(vector<TorrentFileData>  torrent_data)
{

//	vector<string> v_send_list;

//	m_name_server.RequestIP("BTSeedData",v_send_list);

	char dest[128+1];
	int listsize = (int)v_send_list.size();


	BTIPDataHeader header;

	header.appcode = BTIPDATA_LOW_SEED_TORRENT;

	TorrentFileData tempdata;
	for (int j = 0; j < (int)torrent_data.size(); j ++)
	{

		tempdata.m_hash = torrent_data[j].m_hash;
		tempdata.m_announce_URL = torrent_data[j].m_announce_URL;
		tempdata.m_torrent_id = torrent_data[j].m_torrent_id;
		tempdata.m_name = torrent_data[j].m_name;
		char * data = new char[sizeof(BTIPDataHeader)+tempdata.GetBufferLength()];
		header.length = tempdata.GetBufferLength();
		char * ptr = data;
		memcpy(ptr,&header,sizeof(BTIPDataHeader));
		ptr += sizeof(BTIPDataHeader);
		tempdata.WriteToBuffer(ptr);
		int size = sizeof(BTIPDataHeader)+tempdata.GetBufferLength();
		
		for (int i = 0; i < (int)v_send_list.size();i++)
		{
			strcpy(dest,v_send_list[i].c_str());
			if((p_com->SendReliableData(dest,data,size))==false)
			{
				bool retry = p_com->SendReliableData(dest,data,size);
			}	
		}
		delete [] data;

	}


}

void BTTorrentSourceDll::RemoveInflatedTorrent(unsigned int torrent_id)
{

	TorrentFileData remove;
	for (int i = 0; i < (int)m_low_seed_data.size(); i ++)
	{

		if (torrent_id == m_low_seed_data[i].m_torrent_id)
		{
			remove = m_low_seed_data[i];
			m_low_seed_data.erase(m_low_seed_data.begin()+i);
			SendStopTorrent(remove);
			m_db.RemoveLowSeedTorrent(remove);
		}

	}

	//SendStopTorrentID(torrent_id);

	m_dlg.RefreshList(m_low_seed_data);
}

void BTTorrentSourceDll::SendStopTorrentID(int torrent_id)
{
//	vector<string> v_send_list;

//	m_name_server.RequestIP("BTSeedData",v_send_list);

	char dest[128+1];


	BTIPDataHeader header;

	header.appcode = BTIPDATA_STOP_SINGLE_INFLATION;

	int size = sizeof(BTIPDataHeader)+sizeof(int);

	char * data = new char[size];
	header.length = sizeof(int);
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	memcpy(ptr,&torrent_id,sizeof(int));

		
	for (int i = 0; i < (int)v_send_list.size();i++)
	{
		strcpy(dest,v_send_list[i].c_str());
		p_com->SendReliableData(dest,data,size);
	}
	delete [] data;

}

void BTTorrentSourceDll::SendStopTorrent(TorrentFileData  torrent_data)
{

	//vector<string> v_send_list;

	//m_name_server.RequestIP("BTSeedData",v_send_list);

	char dest[128+1];


	BTIPDataHeader header;

	header.appcode = BTIPDATA_STOP_SINGLE_INFLATION;


	char * data = new char[sizeof(BTIPDataHeader)+torrent_data.GetBufferLength()];
	header.length = torrent_data.GetBufferLength();
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	torrent_data.WriteToBuffer(ptr);
	int size = sizeof(BTIPDataHeader)+torrent_data.GetBufferLength();
		
	for (int i = 0; i < (int)v_send_list.size();i++)
	{
		strcpy(dest,v_send_list[i].c_str());
		if((p_com->SendReliableData(dest,data,size))==false)
		{
			bool retry = p_com->SendReliableData(dest,data,size);
		}
	}
	delete [] data;


}

void BTTorrentSourceDll::PingAllDestinations()
{
	vector<string> v_ping_list;
	v_send_list.clear();

	m_name_server.RequestIP("BTSeedData",v_ping_list);


	char dest[128+1];


	BTIPDataHeader header;

	header.appcode = BTIPDATA_PING;


	char * data = new char[sizeof(BTIPDataHeader)];
	header.length = 0;
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	int size = sizeof(BTIPDataHeader);
		
	for (int i = 0; i < (int)v_ping_list.size();i++)
	{
		strcpy(dest,v_ping_list[i].c_str());
		p_com->SendReliableData(dest,data,size);
	}
	delete [] data;

}