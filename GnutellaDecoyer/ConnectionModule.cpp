// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"
#include <afxmt.h>	// for CCriticalSection

// For thread
#include "ConnectionSockets.h"
#include "ConnectionModuleStatusData.h"
#include "CompressedQRPTable.h"
#include "VendorCount.h"

#include "GnutellaDecoyerDll.h"	// temp kludge for logging

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
//	int i;

	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	ConnectionModuleThreadData thread_data;
	ConnectionModuleStatusData status_data;
	vector<GnutellaHost> connect_hosts;
	CompressedQRPTable compressed_qrp_table;
	vector<QueryHitResult> share_files;

	// Create the sockets for this module
	ConnectionSockets sockets;
	memcpy(&thread_data.m_reserved_events[0],&sockets.m_events[0],sizeof(WSAEVENT)*4);
	thread_data.p_status_data=&status_data;
	thread_data.p_connect_hosts=&connect_hosts;
	thread_data.p_compressed_qrp_table=&compressed_qrp_table;
	thread_data.vp_share_files=&share_files;

	PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);

	int num_reserved_events=sockets.ReturnNumberOfReservedEvents();

	WSANETWORKEVENTS events;	// what the fired event data was

	DWORD num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired
	
	// Seed random number generator
	srand((unsigned)time(NULL));

	sockets.p_connection_manager = thread_data.p_connection_manager;

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
			sockets.p_connection_manager = thread_data.p_connection_manager;
			// Temp variables
			vector<GnutellaHost> tmp_connect_hosts;
			CompressedQRPTable tmp_compressed_qrp_table;
			vector<QueryHitResult> tmp_share_files;

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
				if(compressed_qrp_table.GetLen()>0)
				{
					tmp_compressed_qrp_table=compressed_qrp_table;
					compressed_qrp_table.Clear();
				}
				if(share_files.size())
				{
					tmp_share_files=share_files;
					share_files.clear();
				}

				singleLock.Unlock();
			}

			// Call the functions, passing the temp data
			if(tmp_connect_hosts.size()>0)
			{
				sockets.ConnectToHosts(tmp_connect_hosts);
			}
			if(tmp_compressed_qrp_table.GetLen()>0)
			{
				sockets.UpdateCompressedQRPTable(tmp_compressed_qrp_table);
			}
			if(tmp_share_files.size())
			{
				sockets.UpdateShareList(tmp_share_files);
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
				sockets.ReportStatus(status_data);
				WSAResetEvent(sockets.m_events[event]);	// reset event
				//status_data=tmp_status_data;	 // copy the temp data
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

			PostMessage(hwnd,WM_VENDOR_COUNTS_READY,(WPARAM)tmp_vendor_counts,0);
		}
	
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
	m_thread = NULL;
}

//
//
//
ConnectionModule::~ConnectionModule()
{
	// Kill the thread
	if(p_critical_section!=NULL)
	{
		WSASetEvent(m_thread_data.m_reserved_events[0]);
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
	UINT timer_ret=1;
	
	m_thread = AfxBeginThread(ConnectionModuleThreadProc,(LPVOID)m_hwnd,THREAD_PRIORITY_BELOW_NORMAL);

	// Start 1 second timer
	SetTimer(m_hwnd,6,5*1000,0);
}

//
//
//
void ConnectionModule::ConnectToHosts(vector<GnutellaHost> &hosts)
{
	if(p_critical_section==NULL)
	{
		return;
	}

	int i;

	// Tell the thread to connect to these hosts
//	p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ConnectToHosts() - Before LOCK\n",0,false,true);	// black italic
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		*m_thread_data.p_connect_hosts=hosts;
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
//	p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ConnectToHosts() - After LOCK\n",0,false,true);	// black italic

	// Keep track of the connect to hosts for when we are checking to see if we can add an ip to the cache
	v_connect_to_ips.clear();
	for(i=0;i<(int)hosts.size();i++)
	{
		v_connect_to_ips.push_back(hosts[i].IP());
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
}


//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	m_thread_data=*((ConnectionModuleThreadData *)lparam);
	((ConnectionModuleThreadData *)lparam)->p_connection_manager = p_manager;
	m_thread_data.p_connection_manager = p_manager;
	NewShareFileList(p_manager->GetShareFiles());
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
			if(p_critical_section!=NULL)
			{
//				p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::OnTimer() - Before LOCK\n",0,false,true);	// black italic
/*
				CSingleLock singleLock(p_critical_section);
				singleLock.Lock();
				if(singleLock.IsLocked())
				{
*/
					WSASetEvent(m_thread_data.m_reserved_events[2]);	// timer has fired
/*					singleLock.Unlock();
				}
*/
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

//	p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::StatusReady() - Before LOCK\n",0,false,true);	// black italic
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		// Copy the status data and reset
		//status=*m_thread_data.p_status_data;

		m_thread_data.p_status_data->m_mod=m_mod;
		m_thread_data.p_status_data->p_mod=this;

		// sort the list of connected ips
		sort(m_thread_data.p_status_data->v_connected_host_ips.begin(),m_thread_data.p_status_data->v_connected_host_ips.end());

		// Store the latest vector of sorted connected host ips
		v_connected_host_ips=m_thread_data.p_status_data->v_connected_host_ips;

		// Report the status to the parent
		p_manager->ReportStatus(*(m_thread_data.p_status_data));

		m_thread_data.p_status_data->Clear();

		singleLock.Unlock();
	}
//	p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::StatusReady() - After LOCK\n",0,false,true);	// black italic

	// Init mod-specific data
	/*
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
		WSASetEvent(m_thread_data.m_reserved_events[3]);	// report vendor data
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
UINT ConnectionModule::GetConnectedNum()
{
	return v_connected_host_ips.size();
}

//
//
//
void ConnectionModule::NewShareFileList(vector<QueryHitResult>& share_files)
{
	if(p_critical_section==NULL)
	{
		return;
	}

	vector<QueryHitResult> *shares=&share_files;

	// Tell the thread that the keywords need to be updated
//	p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::KeywordsUpdated() - Before LOCK\n",0,false,true);	// black italic
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		(*m_thread_data.vp_share_files)=*shares;
		m_thread_data.p_compressed_qrp_table->SetData(p_manager->p_compressed_qrp_table,p_manager->m_compressed_qrp_table_len);
	
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
}
