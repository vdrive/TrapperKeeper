// NoiseModule.cpp

#include "stdafx.h"
#include "NoiseModule.h"
#include "NoiseManager.h"
#include "NoiseModuleThreadData.h"
#include <afxmt.h>	// for CCriticalSection
#include "NoiseSockets.h"
#include "NoiseModuleThreadStatusData.h"
//
//
//
UINT NoiseModuleThreadProc(LPVOID pParam)
{
	UINT i;

	// Init message window handle
	NoiseModule *mod=(NoiseModule *)pParam;
	HWND hwnd=mod->m_hwnd;
	CCriticalSection *connection_data_critical_section=mod->p_connection_data_critical_section;
	//vector<ConnectionData> *connection_data=mod->p_connection_data;

	// Init the message data structure and send it
	CCriticalSection critical_section;

	vector<SOCKET> incoming_connection_socket_handles;
	vector<NoiseModuleThreadStatusData> status_data;

	NoiseModuleThreadData thread_data;
	thread_data.p_incoming_connection_socket_handles=&incoming_connection_socket_handles;
	thread_data.p_status_data=&status_data;

	// Create sockets for this module
	NoiseSockets sockets(hwnd,connection_data_critical_section,/*connection_data,*/mod->p_fs_manager);

	memcpy(&thread_data.m_reserved_events[0],&sockets.m_events[0],sizeof(WSAEVENT)*4);
	
	::SendMessage(hwnd,WM_INIT_NOISE_MODULE_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);

	int num_reserved_events=sockets.ReturnNumberOfReservedEvents();

	WSANETWORKEVENTS events;	// what the fired event data was

	DWORD num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired
	
	// Seed random number generator
	srand((unsigned)time(NULL));

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
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				// Incoming connection queue
				if(incoming_connection_socket_handles.size()>0)
				{
					for(i=0;i<incoming_connection_socket_handles.size();i++)
					{
						sockets.AcceptConnection(incoming_connection_socket_handles[i]);
					}

					incoming_connection_socket_handles.clear();
				}
/*				
				// Connection queue
				if(connection_data.size()>0)
				{
					vector<ConnectionData>::iterator data_iter=connection_data.begin();
					while(data_iter!=connection_data.end())
					{
						sockets.Connect(data_iter->m_ip,data_iter->m_port);
						data_iter++;
					}
				
					connection_data.clear();
				}

				// Disconnection queue
				if(disconnection_data.size()>0)
				{
					vector<string>::iterator str_iter=disconnection_data.begin();
					while(str_iter!=disconnection_data.end())
					{
						sockets.DisConnect(*str_iter);
						str_iter++;
					}
				
					disconnection_data.clear();
				}

				// Projects
				if(projects.size()>0)
				{
					sockets.SetProjects(projects);
					projects.clear();
				}

				// Identifying GUIDs
				if(identifying_guids.size()>0)
				{
					sockets.SetIdentifyingGUIDs(identifying_guids);
					identifying_guids.clear();
				}

				// Search GUID Lists
				if(search_guid_lists.size()>0)
				{
					sockets.SetSearchGUIDLists(search_guid_lists);
					search_guid_lists.clear();
				}
				
				// Searcher GUID Lists
				if(searcher_guid_lists.size()>0)
				{
					sockets.SetSearcherGUIDLists(searcher_guid_lists);
					searcher_guid_lists.clear();
				}

				// Search GUID Data
				if(search_guid_data.size()>0)
				{
					sockets.PerformQueries(search_guid_data);
					search_guid_data.clear();
				}

				// GUID Interdiction Connection Data
				if(guid_interdiction_data.size()>0)
				{
					// See if there are any connection items, and not reporting items
					vector<GUIDInterdictionData> connection_data;
					vector<GUIDInterdictionData> reporting_data;
					for(int i=0;i<guid_interdiction_data.size();i++)
					{
						if(guid_interdiction_data[i].m_reporting==false)
						{
							connection_data.push_back(guid_interdiction_data[i]);
						}
						else
						{
							reporting_data.push_back(guid_interdiction_data[i]);
						}
					}

					sockets.PerformGUIDInterdiction(connection_data);

					guid_interdiction_data.clear();
					guid_interdiction_data=reporting_data;	// leave the reporting data in the vector
				}

				// Banned GUIDs
				if(banned_guids.size()>0)
				{
					sockets.SetBannedGUIDs(banned_guids);
				}
*/
				singleLock.Unlock();
			}
		}

		// The Timer has fired...tell the mod our latest info.
		if(event==2)
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if (singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event
				
				status_data=sockets.ReportStatus();

				::PostMessage(hwnd,WM_NOISE_MODULE_STATUS_READY,0,0);

				singleLock.Unlock();
			}
		}

		// Event 3 - Reset the banned list determination values
		if(event==3)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event
		
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if (singleLock.IsLocked())
			{
//				sockets.SetBannedGUIDDeterminationValues(banned_guid_threshold,banned_guid_timer_length);
				singleLock.Unlock();
			}
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

	return 0;	// exit the thread
}

//
//
//
NoiseModule::NoiseModule(CCriticalSection *connection_data_critical_section,/*vector<ConnectionData> *connection_data,*/NoiseManager *manager, FileSharingManager* fsm)
{
	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in NoiseModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	m_idle_socket_count=60;

	p_critical_section=NULL;
	//p_connection_data_critical_section=connection_data_critical_section;
	//p_connection_data=connection_data;
	p_manager=manager;
	p_fs_manager = fsm;

	// Start the worker thread...passing it the handle to the message window
	m_hwnd=m_wnd.GetSafeHwnd();
	m_thread = AfxBeginThread(NoiseModuleThreadProc,(LPVOID)this,THREAD_PRIORITY_BELOW_NORMAL);

	m_wnd.SetTimer(0,1*1000,0);	// 1 sec
}

//
//
//
NoiseModule::~NoiseModule()
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
void NoiseModule::InitParent(NoiseManager *manager, FileSharingManager* fsm)
{
	p_manager=manager;
	p_fs_manager = fsm;
}

//
//
//
void NoiseModule::InitIndex(unsigned int index)
{
	m_index=index;
}

//
//
//
unsigned int NoiseModule::IdleSocketCount()
{
	return m_idle_socket_count;
}

//
//
//
void NoiseModule::AcceptConnection(SOCKET hSocket)
{
	m_idle_socket_count--;

	if(p_critical_section==NULL)
	{
		v_cached_incoming_connection_socket_handles.push_back(hSocket);
	}
	else
	{
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			p_thread_data->p_incoming_connection_socket_handles->push_back(hSocket);
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// new vector item data
			singleLock.Unlock();
		}
	}
}

//
//
//
void NoiseModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	UINT i;
	
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data=(NoiseModuleThreadData *)lparam;

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		for(i=0;i<v_cached_incoming_connection_socket_handles.size();i++)
		{
			p_thread_data->p_incoming_connection_socket_handles->push_back(v_cached_incoming_connection_socket_handles[i]);	
		}
		v_cached_incoming_connection_socket_handles.clear();

		WSASetEvent(p_thread_data->m_reserved_events[1]);	// new vector item data
		singleLock.Unlock();
	}
}

//
//
//
void NoiseModule::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 0:
		{
			if(p_critical_section!=NULL)
			{
				CSingleLock singleLock(p_critical_section);
				singleLock.Lock();
				if(singleLock.IsLocked())
				{
					WSASetEvent(p_thread_data->m_reserved_events[2]);	// timer has fired
					singleLock.Unlock();
				}
				m_wnd.KillTimer(0);
			}
			break;
		}
	}
}

//
//
//
void NoiseModule::StatusReady(WPARAM wparam,LPARAM lparam)
{
	UINT i;
	vector<NoiseModuleThreadStatusData> status;

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		status=*p_thread_data->p_status_data;
		p_thread_data->p_status_data->clear();
		singleLock.Unlock();
	}

	// Set the module index, and upate the number of idle sockets
	m_idle_socket_count=0;
	for(i=0;i<status.size();i++)
	{
		status[i].m_mod=m_index;

		if(status[i].m_is_idle)
		{
			m_idle_socket_count++;
		}
	}

	p_manager->ReportStatus(status);

	m_wnd.SetTimer(0,1*1000,0);
}