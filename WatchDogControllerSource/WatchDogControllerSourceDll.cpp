/////////////////////////////////////////////////////////////////////////////////////
// Author : Jerry Trinh
// Date   : March | 10 | 2003
// Purpose: With the WDC program is a Rack is down for more than one hour, 
//          the Rack will restart automatically.  
/////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "WatchDogControllerSourceDll.h"
#include "DllInterface.h"
#include "Rack.h"

//
//
//
WatchDogControllerSourceDll::WatchDogControllerSourceDll(void)
{
	m_com_link = NULL;
}

//
//
//
WatchDogControllerSourceDll::~WatchDogControllerSourceDll(void)
{
}

//
//
//
void WatchDogControllerSourceDll::DllInitialize()
{
	// initialization
	m_dlg.Create(IDD_WDCS_DLG, CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_ping_interval = 5*60; //time interval in between pings
}

//
//
//
void WatchDogControllerSourceDll::DllUnInitialize()
{
	// release memory:
	::DestroyWindow(m_dlg.GetSafeHwnd());
	if (m_com_link != NULL)
	{
		// object released:
		delete m_com_link;
	}
}

//
//
//
void WatchDogControllerSourceDll::DllStart()
{
	//Create and register the com interface
	m_com_link = new WatchDogComLink();
	m_com_link->Register(this, DllInterface::GetAppID().m_app_id);
	
	m_name_server.Register(this);
	// call this routine to retrieve all ips from the racks and push into the vector list
	m_name_server.RequestIP("WATCHDOG", v_ip_list);
	
	// set timer in 5 minutes
	m_dlg.SetTimer(1, m_ping_interval*1000, NULL);

	// init m_count in such to keep track of responsding message from destination
	//m_count =0;

	// remove all items from the list:
	m_dlg.DeleteAllRacks();

	//TRACE("WATCHDOGCONTROLLERSOURCE: Getting IP List from the NameServer.\n");		
	// add all items from the vector list into the dialog:
	for (UINT i=0; i<v_ip_list.size(); i++)
	{
	//	//TRACE("WATCHDOGCONTROLLERSOURCE: IP %d is equal to %s.\n",i,v_ip_list[i]);		
		m_dlg.AddDestIP((char*)v_ip_list[i].c_str());		
		v_racks.push_back(v_ip_list[i]);
	}

	m_dlg.TotalWatchRacks();
}

//
//
//
void WatchDogControllerSourceDll::DllShowGUI()
{
	// pop up dialog:
	m_dlg.ShowWindow(SW_NORMAL);
}

//
//
// this function is overrided from Com Interface
void WatchDogControllerSourceDll::DataReceived(char *source_name, void *data, int data_length)
{
	// set data
	WatchDogControllerSourceHeader* header = (WatchDogControllerSourceHeader*) data;	
	switch(header->op)
	{
		case (WatchDogControllerSourceHeader::op_code::pong):
		{				
			AddNewIP(source_name);	
			m_dlg.TimeFromPong(v_racks, source_name);			
			break;
		}
	}
}

//
//
//
bool WatchDogControllerSourceDll::SendControllerMessage(char * dest, WatchDogControllerSourceHeader::op_code new_op_code)
{
	byte buf[sizeof(WatchDogControllerSourceHeader)];
	WatchDogControllerSourceHeader *header = (WatchDogControllerSourceHeader*)buf;
	header->op = new_op_code;
	header->size = 0;
	return m_com_link->SendUnreliableData(dest, buf, sizeof(WatchDogControllerSourceHeader));
}

//
//
// send message to destination with restart signal
bool WatchDogControllerSourceDll::SendMessageToDest(char * ip)
{
	// Be sure we send the right ips:
	char* new_ip;
	for (UINT i=0; i<v_ip_list.size(); i++)
	{
		if (strcmp(v_ip_list[i].c_str(), ip) == 0)
		{
			new_ip = (char*)v_ip_list[i].c_str();
			break;
		}
	}
	//determined the size of header message:
	byte buf[sizeof(WatchDogControllerSourceHeader)];

	WatchDogControllerSourceHeader* header = (WatchDogControllerSourceHeader*)buf;
	header->op = WatchDogControllerSourceHeader::restart;
	header->size = 0;	
	return m_com_link->SendUnreliableData(new_ip, buf, sizeof(WatchDogControllerSourceHeader));
}

//
//
// make sure send a ping message to dest when timer is 5 minutes
void WatchDogControllerSourceDll::OnPingTimer()
{
	//m_count =0;
	// call this function because when timer reset
	//m_dlg.ConnectingStatus();		
	m_dlg.TimeToPing(v_racks);
	SendControllerMessage("WATCHDOG", WatchDogControllerSourceHeader::ping);
	m_dlg.TotalWatchRacks();
	//m_dlg.SetTimer(2, 30*1000, 0);
}

//
//
// This is a not good version for searching but for debug only..I will update by using Binary Search method.
void WatchDogControllerSourceDll::AddNewIP(char * ip)
{
	Rack the_rack;

	bool found = false;
	for (UINT i = 0; i<v_ip_list.size(); i++)
	{
		if (strcmp(v_ip_list[i].c_str(), ip) == 0)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		// update rack ip list
		the_rack.m_rack_name = ip;
		v_racks.push_back(the_rack);
		// add new ip into the list if it doesn't exist:
		v_ip_list.push_back(ip);
		// also add new ip into the dialog:
		m_dlg.AddDestIP(ip);
	}
}

//
//
// using binary search:
void WatchDogControllerSourceDll::AddNewIPS(char * new_ip)
{
	//bool found = binary_search(v_ip_list.begin(), v_ip_list.end(), new_ip);
	//vector<string>::iterator exist = NULL;
	//exist = find(v_temp_ip_list.begin(), v_temp_ip_list.end(); new_ip);

	//if (exist != NULL && exist != v_temp_ip_list.end())
	//{
	//	found = true;
	//	break;
	//}

	//if (!found)
	//{
	//	v_ip_list.push_back(ip);
	//	m_dlg.AddDestIP(ip);
	//}
	
}
