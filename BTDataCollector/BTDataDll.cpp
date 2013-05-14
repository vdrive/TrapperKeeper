#include "StdAfx.h"
#include "BTDataDll.h"
#include "BTDataHeader.h"
#include "NameServerInterface.h"

//
//
//
BTDataDll::BTDataDll(void)
{
}

//
//
//
BTDataDll::~BTDataDll(void)
{
}

//
//
//
void BTDataDll::DllInitialize()
{
	m_dlg.Create(IDD_COLLECTOR_DLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTDataDll::DllUnInitialize()
{

}

//
//
//
void BTDataDll::DllStart()
{

	p_com = new BTCollectorComInterface();

	p_com->InitParent(this);
	p_com->Register(this,235);

	m_dlg.InitParent(this);


	m_dlg.SetTimer(1,10*60*1000,NULL);
	m_dlg.SetTimer(2,6*60*1000,NULL);


}

//
//
//
void BTDataDll::DllShowGUI()
{	
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTDataDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	char * ptr = (char *)data;
	BTDataHeader * bheader = (BTDataHeader *)data;
	ptr += sizeof(BTDataHeader);

	if (bheader->m_appid == 235)
	{
		if (bheader->m_opcode == BTDATAHEADER_RETURN)
		{
			ParseClientData(source_name,ptr,data_length - sizeof(BTDataHeader));
			return;
		}
		else
		{
			return;
		}
	}
}

void BTDataDll::TimerFired(UINT nIDEvent)
{

	switch(nIDEvent)
	{
		case 1:	
		{
			SendDataRequests();
			break;
		}
		case 2:
		{
			CalculateData();
		}
	}
}

void BTDataDll::SendDataRequests()
{

	NameServerInterface ns;
	vector<string> v_send_list;

	ns.RequestIP("BTData",v_send_list);
	
	BTDataHeader btheader;

	btheader.m_appid = 235;
	btheader.m_opcode = BTDATAHEADER_REQUEST;

	int size = sizeof(btheader);
	char * data = new char[size];
	memcpy(data,&btheader,size);

	char dest[128];

	for (int i = 0; i < (int)v_send_list.size();i++)
	{
		strcpy(dest,v_send_list[i].c_str());
		p_com->SendUnreliableData(dest,data,size);
	}

	delete [] data;
}

void BTDataDll::CalculateData()
{

	ClientData tempdata;
	vector<ClientData> v_temp_data;

	double timediff = 0;

	int tempsize = (int)v_client_data.size();

	for (int i = 0; i < (int)v_client_data.size(); i++)
	{
		vector<ClientData>::iterator client_iter = v_client_data[i].begin();
		while (client_iter < v_client_data[i].end())
		{
			/*
			tempdata.m_bytes = client_iter->m_bytes;
			strcpy(tempdata.m_ip,client_iter->m_ip);
			strcpy(tempdata.m_torrentname,client_iter->m_torrentname);
			*/
			tempdata = *client_iter;
			int found = 0;
			for (int k = 0; k<(int)v_temp_data.size(); k++)
			{
			
				if (strcmp(tempdata.m_ip,v_temp_data[k].m_ip) == 0)
				{
					found = 1;
					v_temp_data[k].m_bytes += tempdata.m_bytes;
					v_temp_data[k].m_bytes_sent += tempdata.m_bytes_sent;

					timediff = difftime((time_t)tempdata.m_last_time.GetTime(),(time_t)tempdata.m_first_time.GetTime());
					v_time[k] += timediff;
					break;
				}
			}

			if (found == 0)
			{
				timediff = difftime((time_t)tempdata.m_last_time.GetTime(),(time_t)tempdata.m_first_time.GetTime());
				v_temp_data.push_back(tempdata);
				v_time.push_back(timediff);
			}	
			client_iter++;
		}
	}
	m_dlg.DisplayData(v_temp_data, v_time);
}




void BTDataDll::ParseClientData(char * source_name,void * data, int data_length)
{

	int count = (data_length/sizeof(ClientData));
	vector<ClientData> v_temp_clients;
	char * ptr = (char*)data;

	for (int i = 0; i < count; i++)
	{
		ClientData * tempdata = (ClientData *)ptr;
		v_temp_clients.push_back(*tempdata);
		ptr += sizeof(ClientData);
	}

	int found = 0;
	int index = 0;
	if (count > 0)
	{
		for (int j = 0; j < (int)v_data_map.size(); j++)
		{
			char tempip[16+1];
			strcpy(tempip,v_data_map[j].c_str());
			if (strcmp(tempip,source_name) == 0)
			{
				found = 1;
				index = j;
				break;
			}
		}
	}
	if (found == 1)
	{
//		delete (&(v_client_data.begin() + index));
		(v_client_data.begin()+index)->clear();
//		delete *(v_data_map.begin() + index);
		v_data_map.erase(v_data_map.begin() + index);
		v_client_data.erase(v_client_data.begin() + index);
	}

	if (count > 0)
	{
		v_client_data.push_back(v_temp_clients);
		v_data_map.push_back(source_name);
	}

}