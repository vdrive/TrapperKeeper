// ConnectionModule.cpp
#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"


#include <afxmt.h>	// for CCriticalSection

// For thread
#include "ConnectionSockets.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include "ProjectSupplyQuery.h"

#include "FastTrackGiftDll.h"	// temp kludge for logging

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
	UINT i;
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	ConnectionModuleThreadData thread_data;
	ConnectionModuleStatusData status_data;
	vector<SupernodeHost> connect_hosts;
	vector<ProjectSupplyQuery> project_supply_queries;
	vector<ProjectKeywords> keywords;
	//vector<SupplyProject> supply;

	// Create the sockets for this module
	ConnectionSockets sockets;
	sockets.m_dlg_hwnd = hwnd;
	memcpy(&thread_data.m_reserved_events[0],&sockets.m_events[0],sizeof(WSAEVENT)*5);
	thread_data.p_status_data=&status_data;
	thread_data.p_connect_hosts=&connect_hosts;
	thread_data.p_project_supply_queries=&project_supply_queries;
	thread_data.p_keywords=&keywords;
	//thread_data.p_supply=&supply;
	SendMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);
	sockets.p_file_sharing_manager = thread_data.p_file_sharing_manager;

	int num_reserved_events=sockets.ReturnNumberOfReservedEvents();

	WSANETWORKEVENTS events;	// what the fired event data was

	DWORD num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired
	
	// Seed random number generator
	srand((unsigned)time(NULL));

/*
#ifdef SHARING_ENABLED
		sockets.CreateSpoofShares();
#endif
*/
	while(1)
	{
		event=WSAWaitForMultipleEvents(num_events,&sockets.m_events[0],wait_all,timeout,alertable);
		
		char buf[16];
		_itoa(event,buf,10);
		
		if(event==WSA_WAIT_FAILED)
		{
			::MessageBox(NULL,"Wait Failed.","Error",MB_OK);
			break;
		}

		// Check to see if this is the kill thread event (reserved event 0)
		if(event==0)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event
			break;
		}

		// Process all the items in the process queue and cancel all the transfers in the cancel queue
		if(event==1)
		{
			// Temp variables
			vector<SupernodeHost> tmp_connect_hosts;
			vector<ProjectSupplyQuery> tmp_project_supply_queries;
			vector<ProjectKeywords> tmp_keywords;
			//vector<SupplyProject> tmp_supply;
			
			// Copy the critical data to the temp variables
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				if(connect_hosts.size()>0)
				{
					tmp_connect_hosts=connect_hosts;
					connect_hosts.clear();
				}
				if(project_supply_queries.size()>0)
				{
					tmp_project_supply_queries=project_supply_queries;
					project_supply_queries.clear();
				}
				if(keywords.size()>0)
				{
					tmp_keywords=keywords;
					keywords.clear();
				}
				/*
				if(supply.size()>0)
				{
					tmp_supply=supply;
					supply.clear();
				}
				*/

				singleLock.Unlock();
			}

			// Call the functions, passing the temp data
			if(tmp_connect_hosts.size()>0)
			{
				if(!thread_data.m_disconnect_to_hosts)
					sockets.ConnectToHosts(tmp_connect_hosts);
			}
			if(tmp_project_supply_queries.size()>0)
			{
				for(i=0;i<(int)tmp_project_supply_queries.size();i++)
				{
					sockets.PerformProjectSupplyQuery(tmp_project_supply_queries[i]);
				}
			}
			
			if(tmp_keywords.size()>0)
			{
				sockets.UpdateKeywords(tmp_keywords);
			}
			/*
			if(tmp_supply.size()>0)
			{
				sockets.UpdateSupply(tmp_supply);
			}
			*/
			if(thread_data.m_reconnect_to_hosts)
			{
				thread_data.m_reconnect_to_hosts = false;
				sockets.ReConnectAll();
			}
			if(thread_data.m_disconnect_to_hosts)
			{
				sockets.ReConnectAll();
			}
		}

		// The Timer has fired...tell the mod our latest info.
		if(event==2)
		{
			// Extract the status data from the sockets as temp data
			//ConnectionModuleStatusData tmp_status_data=sockets.ReportStatus();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event
				
				//status_data=tmp_status_data;	 // copy the temp data
				sockets.ReportStatus(status_data);

				singleLock.Unlock();
			}
			
			PostMessage(hwnd,WM_STATUS_READY,0,0);
		}

		// Event 3
		if(event==3)
		{
			vector<VendorCount> *tmp_vendor_counts=sockets.ReturnVendorCounts();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				// Do nothing else

				singleLock.Unlock();
			}

			PostMessage(hwnd,WM_VENDOR_COUNTS_READY,(WPARAM)tmp_vendor_counts,(LPARAM)0);
		}
/*	
		if(event==4) //searching
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				// Do nothing else
				sockets.Search(thread_data.m_search_string);

				singleLock.Unlock();
			}
		}
*/
		// Else it is a socket event
		if(event>3)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event

			int index=event-num_reserved_events;
			
			events=sockets.m_sockets[index].ReturnNetworkEvents();
			
			if(events.lNetworkEvents & FD_CONNECT)
			{
				sockets.m_sockets[index].OnConnect(events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(events.lNetworkEvents & FD_READ)
			{
				sockets.m_sockets[index].OnReceive(events.iErrorCode[FD_READ_BIT]);
			}
			if(events.lNetworkEvents & FD_WRITE)
			{
				sockets.m_sockets[index].OnSend(events.iErrorCode[FD_WRITE_BIT]);
			}
			if(events.lNetworkEvents & FD_CLOSE)
			{
				sockets.m_sockets[index].OnClose(events.iErrorCode[FD_CLOSE_BIT]);
			}
			if(events.lNetworkEvents & FD_ACCEPT)
			{
				sockets.m_sockets[index].OnAccept(events.iErrorCode[FD_ACCEPT_BIT]);
			}
		}
	}
	TRACE("Terminating ConnectionModuleThreadProc\n");
	return 0;	// exit the thread
}

//
//
//
ConnectionModule::ConnectionModule()
{
	p_thread_data = NULL;
	UINT timer_ret=1;
	
	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ConnectionModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	p_critical_section=NULL;
	m_mod=0;

	// Start the worker thread...passing it the handle to the message window
	m_hwnd=m_wnd.GetSafeHwnd();
	m_thread = AfxBeginThread(ConnectionModuleThreadProc,(LPVOID)m_hwnd,THREAD_PRIORITY_BELOW_NORMAL);

	// Start 5 second timer
	SetTimer(m_hwnd,6,5*1000,0);
}

//
//
//
ConnectionModule::~ConnectionModule()
{
	// Kill the thread
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[0]);
	}
	if(m_thread!=NULL)
	{
		if(WaitForSingleObject(m_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}
	// Destroy message window
	m_wnd.DestroyWindow();

}

//
//
//
void ConnectionModule::InitParent(ConnectionManager *manager,unsigned int mod)
{
	p_manager=manager;
	m_mod=mod;
}

//
//
//
void ConnectionModule::ConnectToHosts(vector<SupernodeHost> &hosts)
{
	
	if(p_critical_section==NULL)
	{
		return;
	}

	int i;

	// Tell the thread to connect to these hosts
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		*p_thread_data->p_connect_hosts=hosts;
		WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}

	// Keep track of the connect to hosts for when we are checking to see if we can add an ip to the cache
	v_connect_to_ips.clear();
	for(i=0;i<(int)hosts.size();i++)
	{
		v_connect_to_ips.push_back(hosts[i].m_ip);
	}
	
	sort(v_connect_to_ips.begin(),v_connect_to_ips.end());

	// Add these hosts to the connected hosts vector, just so that another mod won't connect to them since i am connecting to them
//	for(i=0;i<hosts.size();i++)
//	{
//		v_connected_host_ips.push_back(hosts[i].IP());
//	}

}

//
//
//
unsigned int ConnectionModule::GetModNumber()
{
	return m_mod;
}


//
//
//
bool ConnectionModule::IsConnected(unsigned int ip)
{
	// If this ip is in either vector of ips
	if(binary_search(v_connected_host_ips.begin(),v_connected_host_ips.end(),ip) || binary_search(v_connect_to_ips.begin(),v_connect_to_ips.end(),ip))
	{
		return true;
	}
	else
	{
		return false;
	}

	return false;
}


//
//
//
void ConnectionModule::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	if(p_critical_section==NULL)
	{
		return;
	}

	// Tell the thread to connect to these hosts
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		p_thread_data->p_project_supply_queries->push_back(psq);
		WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
}


//
//
//

void ConnectionModule::KeywordsUpdated()
{
	if(p_critical_section==NULL)
	{
		return;
	}

	vector<ProjectKeywords> *keywords=p_manager->ReturnProjectKeywordsPointer();

	// Tell the thread that the keywords need to be updated
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		*p_thread_data->p_keywords = *keywords;
		if(keywords->size() == 0)
			p_thread_data->m_disconnect_to_hosts = true;
		else
		{
			if(p_thread_data->m_disconnect_to_hosts==true)
				p_thread_data->m_reconnect_to_hosts=true;
			p_thread_data->m_disconnect_to_hosts = false;
		}
#ifdef KAZAA_KILLER_ENABLED
		p_thread_data->m_disconnect_to_hosts = false;
#endif
		WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
}

//
//
//
/*
void ConnectionModule::ProjectSupplyUpdated()
{
	if(p_critical_section==NULL)
	{
		return;
	}

	vector<SupplyProject> *supply=p_manager->ReturnProjectSupplyPointer();

		// Tell the thread that the keywords need to be updated
//		p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 1 - Before LOCK\n",0,false,true);	// black italic
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			*p_thread_data->p_supply = *supply;
	
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
			singleLock.Unlock();
		}
//		p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 1 - After LOCK\n",0,false,true);	// black italic
	
}
*/
//
// Message Window Functions
//
//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data=(ConnectionModuleThreadData *)lparam;
	p_thread_data->p_file_sharing_manager = p_manager->p_file_sharing_manager;

	KeywordsUpdated();	// read in the keywords now that we've gotten the thread data
	//ProjectSupplyUpdated();	// read in all of the projects supply data
}

//
//
//
void ConnectionModule::OnTimer(UINT nIDEvent)
{
	UINT timer_ret=1;

	KillTimer(m_hwnd,nIDEvent);

	switch(nIDEvent)
	{
		case 6:	// 1 second timer
		{
			// Request status, tell the thread that the timer has fired
			if(p_critical_section!=NULL && p_thread_data != NULL)
			{
//				p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::OnTimer() - Before LOCK\n",0,false,true);	// black italic

					WSASetEvent(p_thread_data->m_reserved_events[2]);	// timer has fired

//				p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::OnTimer() - After LOCK\n",0,false,true);	// black italic
			}
			else
			{
				// Restart the timer
				SetTimer(m_hwnd,nIDEvent,5*1000,0);
			}

			break;
		}
	}
}

//
//
//
void ConnectionModule::StatusReady(WPARAM wparam,LPARAM lparam)
{
	UINT timer_ret=1;

	if(p_critical_section==NULL)
	{
		return;
	}
	
	// Get the status
	//ConnectionModuleStatusData status;
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		// Init mod-specific data
		p_thread_data->p_status_data->m_mod=m_mod;
		p_thread_data->p_status_data->p_mod=this;


		// sort the list of connected ips
		sort(p_thread_data->p_status_data->v_connected_host_ips.begin(),p_thread_data->p_status_data->v_connected_host_ips.end());

		// Store the latest vector of sorted connected host ips
		v_connected_host_ips=p_thread_data->p_status_data->v_connected_host_ips;

		// Report the status to the parent
		p_manager->ReportStatus(*(p_thread_data->p_status_data));

		// Copy the status data and reset
		//status=*p_thread_data->p_status_data;
		p_thread_data->p_status_data->Clear();

		singleLock.Unlock();
	}
/*
	// Init mod-specific data
	status.m_mod=m_mod;
	status.p_mod=this;


	// sort the list of connected ips
	sort(status.v_connected_host_ips.begin(),status.v_connected_host_ips.end());

	// Store the latest vector of sorted connected host ips
	v_connected_host_ips=status.v_connected_host_ips;

	// Report the status to the parent
	p_manager->ReportStatus(status);
*/
	// Reset the timer
	SetTimer(m_hwnd,6,5*1000,0);
}

//
//
//
void ConnectionModule::ReportVendorCounts()
{
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[3]);	// report vendor data
	}
}

//
//
//
void ConnectionModule::VendorCountsReady(WPARAM wparam,LPARAM lparam)
{
	p_manager->VendorCountsReady((vector<VendorCount> *)wparam);
}

//
//
//
void ConnectionModule::LogMsg(WPARAM wparam,LPARAM lparam)
{
	p_manager->LogMsg((const char*)wparam);
}

//
//
//
/*
void ConnectionModule::Search(const char* search)
{
	if(p_critical_section!=NULL)
	{
		p_thread_data->m_search_string = search;
		WSASetEvent(p_thread_data->m_reserved_events[4]);	// report vendor data
	}
}
*/

//
//
//
void ConnectionModule::ReConnectAll()
{
	if(p_critical_section==NULL)
	{
		return;
	}

    v_connected_host_ips.clear();
	v_connect_to_ips.clear();
	// Tell the thread to reconnect to supernodes
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		p_thread_data->m_reconnect_to_hosts = true;
		WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
}

//
//
//
bool ConnectionModule::MostlyConnected()
{
	if(v_connected_host_ips.size() >= 45)
		return true;
	else
		return false;
}