// ConnectionModule.cpp

#include "stdafx.h"
#include "FTConnectionModule.h"
#include "ConnectionSockets.h"
#include "PioletPoisonerDlg.h"
// For Thread
#include <afxmt.h>	// for CCriticalSection
#include "ConnectionModule.h"

//
//
//
UINT FTConnectionModuleThreadProc(LPVOID pParam)
{
//	int i;

	// Init message window handle
	FTConnectionModule* pConnectionModule = (FTConnectionModule*)pParam;
	HWND hwnd=pConnectionModule->m_hwnd;

	CCriticalSection critical_section;
//	ConnectionModuleStatusData status_data;
	vector<SOCKET> incoming_connection_socket_handles;
	FTConnectionModuleThreadData thread_data;
//	vector<ProjectKeywords> keywords;
	//vector<SupplyProject> supply;
//	vector<ProjectSupplyQuery> project_supply_queries;
//	UINT max_host = 0;
//	UINT max_host_cache = 0;

//	thread_data.p_status_data=&status_data;
//	thread_data.p_keywords=&keywords;
//	thread_data.p_project_supply_queries=&project_supply_queries;
//	thread_data.p_max_host=&max_host;
//	thread_data.p_max_host_cache=&max_host_cache;

	thread_data.p_incoming_connection_socket_handles=&incoming_connection_socket_handles;
	ConnectionSockets m_socket;
	memcpy(&thread_data.m_reserved_events[0],&m_socket.m_events[0],sizeof(WSAEVENT)*4);


	//	m_socket.InitHwnd(hwnd);
	//m_socket.InitParent((FTConnectionModule*)pParam);

	WSANETWORKEVENTS events;	// what the fired event data was

	DWORD num_events=m_socket.ReturnNumberOfEvents();	// 64
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

//	thread_data.p_vendor_counts = m_socket.ReturnVendorCountsPointer();
	int num_reserved_events=m_socket.ReturnNumberOfReservedEvents();


	// Init the message data and send it
	LRESULT ret = ::SendMessage(hwnd,WM_FT_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);

	// Seed random number generator
	srand((unsigned)time(NULL));

	while(1)
	{
		event=WSAWaitForMultipleEvents(num_events,&m_socket.m_events[0],wait_all,timeout,alertable);
		
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
			WSAResetEvent(m_socket.m_events[event]);	// reset event
			break;
		}

		// Process all the items in the process queue and cancel all the transfers in the cancel queue
		if(event==1)
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(m_socket.m_events[event]);
				char ip[32];
				strcpy(ip, thread_data.m_ip.c_str());
				//m_socket.ConnectToHost(ip, thread_data.m_port, thread_data.m_file_length, thread_data.m_md5.c_str());
				//m_socket.AcceptConnection(thread_data.m_socket,ip, thread_data.m_port, thread_data.m_file_length, thread_data.m_md5.c_str());
				// Incoming connection queue
				if(incoming_connection_socket_handles.size()>0)
				{
					for(UINT i=0;i<incoming_connection_socket_handles.size();i++)
					{
						m_socket.AcceptConnection(incoming_connection_socket_handles[i],ip, thread_data.m_port, thread_data.m_file_length, thread_data.m_md5.c_str());
					}

					incoming_connection_socket_handles.clear();
				}
				singleLock.Unlock();
			}

			/*
			// Temp variables
			vector<ProjectKeywords> tmp_keywords;
			//vector<SupplyProject> tmp_supply;
			vector<ProjectSupplyQuery> tmp_project_supply_queries;
			UINT tmp_max_host = 0;
			UINT tmp_max_host_cache = 0;

			// Copy the critical data to the temp variables
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(events[event]);	// reset event
				if(keywords.size()>0)
				{
					tmp_keywords=keywords;
					keywords.clear();
				}
				if(project_supply_queries.size()>0)
				{
					tmp_project_supply_queries=project_supply_queries;
					project_supply_queries.clear();
				}
				if(max_host > 0)
				{
					tmp_max_host = max_host;
					max_host = 0;
				}
				if(max_host_cache > 0)
				{
					tmp_max_host_cache = max_host_cache;
					max_host_cache = 0;
				}
				singleLock.Unlock();
			}

			// Call the functions, passing the temp data
			
			if(tmp_keywords.size()>0)
			{
				m_socket.UpdateProjectKeywords(tmp_keywords);
			}
			if(tmp_project_supply_queries.size()>0)
			{
				for(i=0;i<(int)tmp_project_supply_queries.size();i++)
				{
					m_socket.PerformProjectSupplyQuery(tmp_project_supply_queries[i]);
				}
			}
			if(tmp_max_host > 0)
			{
				m_socket.UpdateHostLimit(tmp_max_host);
			}
			if(tmp_max_host_cache > 0)
			{
				m_socket.UpdateHostCacheLimit(tmp_max_host_cache);
			}
			*/
		}

		// The Timer has fired...tell the mod our latest info.
		if(event==2)
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(m_socket.m_events[event]);
				
				thread_data.m_num_idle_sockets=m_socket.GetNumIdleSockets();

				singleLock.Unlock();
			}
			/*
			// Extract the status data from the sockets as temp data
			ConnectionModuleStatusData tmp_status_data=pConnectionModule->ReportStatus();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(events[event]);	// reset event
				
				status_data=tmp_status_data;	 // copy the temp data
				m_socket.TimerHasFired();				

				singleLock.Unlock();
			}

			PostMessage(hwnd,WM_SPOOF_DATA_READY,0,0);
			*/
		}

		// Event 3
		if(event==3)
		{
			
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(m_socket.m_events[event]);	// reset event

				m_socket.CloseIdleSockets();

				singleLock.Unlock();
			}
			
		}
	
		// Else it is a socket event
		if(event>3)
		{
			WSAResetEvent(m_socket.m_events[event]);	// reset event

			int index=event-num_reserved_events;

			events=m_socket.m_sockets[index].ReturnNetworkEvents();
			
			if(events.lNetworkEvents & FD_CONNECT)
			{
				m_socket.m_sockets[index].OnConnect(events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(events.lNetworkEvents & FD_READ)
			{
				m_socket.m_sockets[index].OnReceive(events.iErrorCode[FD_READ_BIT]);
			}
			if(events.lNetworkEvents & FD_WRITE)
			{
				m_socket.m_sockets[index].OnSend(events.iErrorCode[FD_WRITE_BIT]);
			}
			if(events.lNetworkEvents & FD_CLOSE)
			{
				m_socket.m_sockets[index].OnClose(events.iErrorCode[FD_CLOSE_BIT]);
			}
			if(events.lNetworkEvents & FD_ACCEPT)
			{
				m_socket.m_sockets[index].OnAccept(events.iErrorCode[FD_ACCEPT_BIT]);
			}
		}
	}

	/*
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
	*/

	return 0;	// exit the thread
}

//
//
//
FTConnectionModule::FTConnectionModule()
{
//	m_header_counter=0;
//	m_max_host=0;
//	m_max_host_cache=0;
//	p_supply=NULL;

	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ConnectionModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	p_critical_section=NULL;
	p_thread_data=NULL;

	// Start the worker thread...passing it the handle to the message window
	m_hwnd=m_wnd.GetSafeHwnd();
	AfxBeginThread(FTConnectionModuleThreadProc,(LPVOID)this,THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
FTConnectionModule::~FTConnectionModule()
{
	// Kill the thread
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[0]);
	}

	// Destroy message window
	m_wnd.DestroyWindow();
}

//
//
//
void FTConnectionModule::InitParent(ConnectionModule *parent)
{
	p_parent=parent;
}

//
//
//
void FTConnectionModule::TimerHasFired(HWND hwnd)
{
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[2]);	// timer has fired
//		p_manager->ReportVendorCounts(m_thread_data.p_vendor_counts);
		::PostMessage(hwnd, WM_FT_SOCKETS_STATUS, (WPARAM)p_thread_data->m_num_idle_sockets,(LPARAM)this);
	}
}

// Message Window Functions

//
//
//
void FTConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data=(FTConnectionModuleThreadData *)lparam;
	for(UINT i=0;i<v_cached_incoming_connection_socket_handles.size();i++)
	{
		p_thread_data->p_incoming_connection_socket_handles->push_back(v_cached_incoming_connection_socket_handles[i]);	
	}
	v_cached_incoming_connection_socket_handles.clear();

//	KeywordsUpdated();
//	ProjectSupplyUpdated(NULL);

}

//
//
//
/*
void ConnectionModule::StatusReady(WPARAM wparam,LPARAM lparam)
{
	char *ptr=(char *)wparam;
	unsigned int *counts=(unsigned int *)lparam;

	p_manager->ReportStatus(ptr,counts);
	if(ptr!=NULL)
	{
		delete [] ptr;
	}
	if(counts!=NULL)
	{
		delete [] counts;
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

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		v_keywords = p_manager->ReturnProjectKeywords();
		UINT i,j;
	
		// Make sure that the project status data is still valid since the keywords have been updated...basically the indexes for each project in
		// the keywords vector have to be the same as the indexes for the same project in the project status vector.
		vector<ProjectStatus> old_project_status=m_status_data.v_project_status;
		m_status_data.v_project_status.clear();

		for(i=0;i<v_keywords.size();i++)
		{
			bool found=false;
			for(j=0;j<old_project_status.size();j++)
			{
				if(strcmp(old_project_status[j].ProjectName(),v_keywords[i].m_project_name.c_str())==0)
				{
					found=true;
					m_status_data.v_project_status.push_back(old_project_status[j]);
					break;
				}
			}

			// If not found, create a new project status object (this must be a new project, then)
			if(!found)
			{
				ProjectStatus ps;
				ps.ProjectName((char *)v_keywords[i].m_project_name.c_str());
				m_status_data.v_project_status.push_back(ps);
			}
		}
		
		(*m_thread_data.p_keywords)=v_keywords;
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data		
		singleLock.Unlock();
	}
	
	// Make sure all of the project keyword data of the project supply queries are still correct
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		bool found=false;
		for(UINT i=0;i<v_keywords.size();i++)
		{
			if(strcmp(psq_iter->m_project.c_str(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&v_keywords[i]);
				psq_iter->m_project_status_index=i;
				break;
			}
		}

		// If not found, then this project no longer exists, so remove the query from the vector
		if(!found)
		{
			v_project_supply_queries.erase(psq_iter);
			psq_iter=v_project_supply_queries.begin();
		}
		else
		{
			psq_iter++;
		}
	}
}

//
//
//
ConnectionModuleStatusData ConnectionModule::ReportStatus()
{

	ConnectionModuleStatusData ret=m_status_data;
	
	m_status_data.ClearCounters();
	
	return ret;
}

//
//
//
void ConnectionModule::SpoofDataReady()
{
	// Get the status
	ConnectionModuleStatusData status;
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		// Copy the status data and reset
		status=*m_thread_data.p_status_data;
		m_thread_data.p_status_data->Clear();
		singleLock.Unlock();
	}

	// Report the status to the parent
	p_manager->SpoofDataReady(status);
}

//
//
//
void ConnectionModule::UpdateHostLimits(UINT max_host, UINT max_host_cache)
{
	m_max_host = max_host;
	m_max_host_cache = max_host_cache;

	if(p_critical_section==NULL)
	{
		return;
	}
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		(*m_thread_data.p_max_host) = m_max_host;
		(*m_thread_data.p_max_host_cache) = m_max_host_cache;
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
}

//
//
//
void ConnectionModule::AddDC(CString dc)
{
	bool found = false;
	for(int i=0; i<(int)v_piolet_dcs.size();i++)
	{
		if(dc.CompareNoCase(v_piolet_dcs[i]) == 0)
		{
			found = true;
			break;
		}
	}
	if(!found)
		v_piolet_dcs.push_back(dc);
}
*/
/*
void ConnectionModule::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	p_parent->Log(text, color, bold, italic);
}
*/

//
//
//
bool FTConnectionModule::ConnectToHost(char* ip, int port, UINT file_length, const char* md5)
{
	if(p_critical_section==NULL)
	{
		return false;
	}

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();

	bool connecting = false;
	if(singleLock.IsLocked())
	{
		if(p_thread_data->m_num_idle_sockets > 0)
		{
			p_thread_data->m_ip = ip;
			p_thread_data->m_port = port;
			p_thread_data->m_file_length = file_length;
			p_thread_data->m_md5 = md5;
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
			connecting = true;
		}
		singleLock.Unlock();
	}
	return connecting;
}

//
//
//
bool FTConnectionModule::AcceptConnection(SOCKET hSocket, const char* ip, int port, UINT file_length, const char* md5)
{
	if(p_critical_section==NULL)
	{
		v_cached_incoming_connection_socket_handles.push_back(hSocket);
		return false;
	}

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();

	bool connecting = false;
	if(singleLock.IsLocked())
	{
		if(p_thread_data->m_num_idle_sockets > 0)
		{
			p_thread_data->m_ip = ip;
			p_thread_data->m_port = port;
			p_thread_data->m_file_length = file_length;
			p_thread_data->m_md5 = md5;
			//p_thread_data->m_socket = hSocket;
			p_thread_data->p_incoming_connection_socket_handles->push_back(hSocket);
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
			connecting = true;
		}
		singleLock.Unlock();
	}
	return connecting;
}

//
//
//
int FTConnectionModule::GetNumIdleSockets()
{
	if(p_thread_data != NULL)
		return p_thread_data->m_num_idle_sockets;
	else
		return -1;
}

//
//
//
void FTConnectionModule::OneMinuteTimer()
{
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[3]);	// one minute timer has fired
//		p_manager->ReportVendorCounts(m_thread_data.p_vendor_counts);
	}
}


//
//
//
/*
void FTConnectionModule::StatusReady(WPARAM wparam,LPARAM lparam)
{
	
	int i;
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

	//p_manager->ReportStatus(status);

	m_wnd.SetTimer(0,1*1000,0);
}
*/