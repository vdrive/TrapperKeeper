// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"

// For Thread
#include <afxmt.h>	// for CCriticalSection
#include "OvernetUDPSocket.h"

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
	int i;

	// Init message window handle
	ConnectionModule* pConnectionModule = (ConnectionModule*)pParam;
	HWND hwnd=pConnectionModule->m_hwnd;

	CCriticalSection critical_section;
	ConnectionModuleStatusData status_data;
	ConnectionModuleThreadData thread_data;
	vector<ProjectKeywords> keywords;
	vector<ProjectSupplyQuery> project_supply_queries;

	thread_data.p_keywords=&keywords;
	thread_data.p_project_supply_queries=&project_supply_queries;
	thread_data.p_status_data=&status_data;

	WSAEVENT events[64];
	memset(events,0,sizeof(events));

	// Create the reserved events
	int num_reserved_events=4;
	for(i=0;i<num_reserved_events;i++)
	{
		events[i]=WSACreateEvent();
		if(events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
	// Create the Overnet Socket and add it's event handle to the events vector
	OvernetUDPSocket m_socket;
	m_socket.InitHwnd(hwnd);
	m_socket.InitParent((ConnectionModule*)pParam);
	int num_socket_events=1;
	events[4]=m_socket.ReturnEventHandle();

	DWORD num_events=num_reserved_events+num_socket_events;
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

	// Init the message data and send it
	memcpy(&thread_data.m_reserved_events[0],&events[0],sizeof(WSAEVENT)*4);
	PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);

	
	// Seed random number generator
	srand((unsigned)time(NULL));

	while(1)
	{
		event=WSAWaitForMultipleEvents(num_events,&events[0],wait_all,timeout,alertable);
		
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
			WSAResetEvent(events[event]);	// reset event
			break;
		}

		// Process all the items in the process queue and cancel all the transfers in the cancel queue
		if(event==1)
		{

			// Temp variables
			vector<ProjectKeywords> tmp_keywords;
			vector<ProjectSupplyQuery> tmp_project_supply_queries;
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
		}

		// The Timer has fired...tell the mod our latest info.
		if(event==2)
		{
			// Extract the status data from the sockets as temp data
			ConnectionModuleStatusData tmp_status_data=pConnectionModule->ReportStatus();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(events[event]);	// reset event
				
				m_socket.OneSecondTick();				
				status_data=tmp_status_data;	 // copy the temp data
				singleLock.Unlock();
			}
			PostMessage(hwnd,WM_STATUS_READY,0,0);
		}

		// Event 3
		if(event==3)
		{
//			vector<VendorCount> *tmp_vendor_counts=sockets.ReturnVendorCounts();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(events[event]);	// reset event

				// Do nothing else

				singleLock.Unlock();
			}

//			PostMessage(hwnd,WM_VENDOR_COUNTS_READY,(WPARAM)tmp_vendor_counts,(LPARAM)0);
		}
	
		// Else it is a socket event
		if(event>3)
		{
			WSAResetEvent(events[event]);	// reset event

			int index=event-num_reserved_events;

			WSANETWORKEVENTS network_events=m_socket.ReturnNetworkEvents();
			
			if(network_events.lNetworkEvents & FD_CONNECT)
			{
				m_socket.OnConnect(network_events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(network_events.lNetworkEvents & FD_READ)
			{
				m_socket.OnReceive(network_events.iErrorCode[FD_READ_BIT]);
			}
			if(network_events.lNetworkEvents & FD_WRITE)
			{
				m_socket.OnSend(network_events.iErrorCode[FD_WRITE_BIT]);
			}
			if(network_events.lNetworkEvents & FD_CLOSE)
			{
				m_socket.OnClose(network_events.iErrorCode[FD_CLOSE_BIT]);
			}
			if(network_events.lNetworkEvents & FD_ACCEPT)
			{
				m_socket.OnAccept(network_events.iErrorCode[FD_ACCEPT_BIT]);
			}
		}
	}

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

	// Start the worker thread...passing it the handle to the message window
	m_hwnd=m_wnd.GetSafeHwnd();
	m_thread = AfxBeginThread(ConnectionModuleThreadProc,(LPVOID)this,THREAD_PRIORITY_NORMAL);
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
void ConnectionModule::InitParent(ConnectionManager *manager)
{
	p_manager=manager;
}

//
//
//
void ConnectionModule::TimerHasFired()
{
	if(p_critical_section!=NULL)
	{
		WSASetEvent(m_thread_data.m_reserved_events[2]);	// timer has fired
	}
}

//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	m_thread_data=*((ConnectionModuleThreadData *)lparam);
	KeywordsUpdated();
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
		//UINT i,j;
	
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
			if(psq_iter->m_project_id == v_keywords[i].m_id)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&v_keywords[i]);
				//psq_iter->m_project_status_index=i;
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
void ConnectionModule::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	if(p_critical_section==NULL)
	{
		return;
	}

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		m_thread_data.p_project_supply_queries->push_back(psq);
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data

		// Remove any existing queries for this project so that all returns from the new query will not count the returns for any old queries
		vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
		while(psq_iter!=v_project_supply_queries.end())
		{
			if(psq_iter->m_project_id == psq.m_project_id && psq_iter->m_track == psq.m_track)
			{
				v_project_supply_queries.erase(psq_iter);
				break;
			}
			psq_iter++;
		}

		// Save this query to the list of project supply queries that we've done 
		v_project_supply_queries.push_back(psq);
		singleLock.Unlock();
	}

}

//
//
//
ConnectionModuleStatusData ConnectionModule::ReportStatus()
{
	ConnectionModuleStatusData ret=m_status_data;
	m_status_data.Clear();
	return ret;
}

//
//
//
void ConnectionModule::StatusReady()
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
	p_manager->StatusReady(status);
}

//
//
//
void ConnectionModule::Log(const char* log)
{
	p_manager->Log(log);
}

//
//
//
ProjectSupplyQuery ConnectionModule::GetPSQ(const byte* hash)
{
	ProjectSupplyQuery psq;
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		for(int i=v_project_supply_queries.size()-1; i>=0; i--)
		{
			if(memcmp(hash, v_project_supply_queries[i].m_keyword_hash, 16)==0)
			{
				if( (i >= (v_project_supply_queries.size()-5))||(v_project_supply_queries.size()<=5))
					return v_project_supply_queries[i];
				else
				{
					psq = v_project_supply_queries[i];
					v_project_supply_queries.erase(v_project_supply_queries.begin()+i);
					v_project_supply_queries.push_back(psq);
					return psq;

				}
			}
		}
		singleLock.Unlock();
	}
	return psq;
}
