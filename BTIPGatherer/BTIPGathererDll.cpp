#include "StdAfx.h"
#include "BTIPGathererDll.h"
#include "BTIPDataHeader.h"


//
//
//
BTIPGathererDll::BTIPGathererDll(void)
{
}

//
//
//
BTIPGathererDll::~BTIPGathererDll(void)
{
}

//
//
//
void BTIPGathererDll::DllInitialize()
{
	//m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());

}

//
//
//
void BTIPGathererDll::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}

}

//
//
//
void BTIPGathererDll::DllStart()
{

	p_com = new BTIPGathererCom();

	p_com->InitParent(this);
	p_com->Register(this,6824);
}

//
//
//
void BTIPGathererDll::DllShowGUI()
{
	//m_dlg.ShowWindow(SW_NORMAL);
	//m_dlg.BringWindowToTop();
}

//
//
//


UINT GatherIPThreadProc(LPVOID pParam)
{

	BTIPThreadData * tempdata = (BTIPThreadData *)pParam;
	if (tempdata == NULL)
	{
		return 1;
	}


	BTIPList ips;
	BTIP tempip;

	TrackerConnection tracker;

	PeerList * peers = tracker.GetPeers(&tempdata->m_data,"");
	int num = peers->GetCount();
	for (int i = 0; i < num; i ++)
	{
		if (peers != NULL)
		{
			strcpy(tempip.m_ip,peers->GetIP().c_str());
			tempip.m_port = atoi(peers->GetPort().c_str());
			strcpy(tempip.m_id,tempdata->m_data.m_hash.c_str());
			tempip.m_torrent_id = tempdata->m_data.m_torrent_id;
			if ((strstr(tempip.m_ip,"38.118.151")==NULL)&&(strstr(tempip.m_ip,"204.9.116")==NULL)&&(strstr(tempip.m_ip,"38.118.154")==NULL)&&(strstr(tempip.m_ip,"38.119.66")==NULL))
			{
				ips.AddIp(tempip);
			}
			peers = peers->GetNext();
		}
	}

	ips.CalculateNumIPs();


	BTIPDataHeader header;

	header.appcode = BTIPDATA_IP_LIST;



	if (num > 0)
	{
		char * data = new char[sizeof(BTIPDataHeader)+ips.GetBufferLength()];
		header.length = ips.GetBufferLength();
		char * ptr = data;
		memcpy(ptr,&header,sizeof(BTIPDataHeader));
		ptr += sizeof(BTIPDataHeader);
		ips.WriteToBuffer(ptr);
		int size = sizeof(BTIPDataHeader)+ips.GetBufferLength();
		char source[32+1];
		strcpy(source,tempdata->source_name.c_str());

		tempdata->p_com->SendReliableData(source,data,size);
		delete [] data;
	}




	delete pParam;
	return 0;
}

BTIPList BTIPGathererDll::GetPeerList(TorrentFileData tdata)
{

	BTIPList ips;
	BTIP tempip;

	PeerList * peers = m_tracker.GetPeers(&tdata,"");
	int num = peers->GetCount();
	for (int i = 0; i < num; i ++)
	{
		if (peers != NULL)
		{
			strcpy(tempip.m_ip,peers->GetIP().c_str());
			tempip.m_port = atoi(peers->GetPort().c_str());
			strcpy(tempip.m_id,tdata.m_hash.c_str());
			tempip.m_torrent_id = tdata.m_torrent_id;
			if ((strstr(tempip.m_ip,"38.118.151")==NULL)&&(strstr(tempip.m_ip,"204.9.116")==NULL)&&(strstr(tempip.m_ip,"38.118.154")==NULL)&&(strstr(tempip.m_ip,"38.119.66")==NULL))
			{
				ips.AddIp(tempip);
			}
			peers = peers->GetNext();
		}
	}

	ips.CalculateNumIPs();

	return ips;

}

/*
void BTIPGathererDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface

	BTIPDataHeader * header = (BTIPDataHeader *)data;
	switch(header->appcode)
	{
		case BTIPDATA_TORRENT:	
		{
			m_source_name = source_name;
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			TorrentFileData tempdata;
			tempdata.ReadFromBuffer(ptr);
			BTIPList iplist = GetPeerList(tempdata);
			iplist.CalculateNumIPs();
			if (iplist.m_num_ips != 0)
			{
				SendIPData(source_name,iplist);
			}
			

			break;
		}

	
	}

}
*/

void BTIPGathererDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface

	BTIPDataHeader * header = (BTIPDataHeader *)data;
	switch(header->appcode)
	{
		case BTIPDATA_TORRENT:	
		{
			m_source_name = source_name;
			char * ptr = (char*)data;
			ptr += sizeof(BTIPDataHeader);
			BTIPThreadData * tempdata = new BTIPThreadData;
			tempdata->m_data.ReadFromBuffer(ptr);
			tempdata->p_com = p_com;
			tempdata->source_name = source_name;
			AfxBeginThread(GatherIPThreadProc,tempdata,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
			break;
		}

	
	}

}

void BTIPGathererDll::SendIPData(char *source_name,BTIPList iplist)
{

	BTIPDataHeader header;

	header.appcode = BTIPDATA_IP_LIST;



	char * data = new char[sizeof(BTIPDataHeader)+iplist.GetBufferLength()];
	header.length = iplist.GetBufferLength();
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	iplist.WriteToBuffer(ptr);
	int size = sizeof(BTIPDataHeader)+iplist.GetBufferLength();

	p_com->SendUnreliableData(source_name,data,size);

	delete [] data;


}


void BTIPGathererDll::SendSourceIPData(BTIPList iplist)
{

	BTIPDataHeader header;

	header.appcode = BTIPDATA_IP_LIST;



	char * data = new char[sizeof(BTIPDataHeader)+iplist.GetBufferLength()];
	header.length = iplist.GetBufferLength();
	char * ptr = data;
	memcpy(ptr,&header,sizeof(BTIPDataHeader));
	ptr += sizeof(BTIPDataHeader);
	iplist.WriteToBuffer(ptr);
	int size = sizeof(BTIPDataHeader)+iplist.GetBufferLength();

//	p_com->SendUnreliableData(m_source_name.c_str(),data,size);

	delete [] data;


}




