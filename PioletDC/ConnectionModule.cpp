// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"

// For Thread
#include <afxmt.h>	// for CCriticalSection
#include "BlubsterSocket.h"

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
	vector<SupplyProject> supply;
	vector<ProjectSupplyQuery> project_supply_queries;
	UINT max_host = 0;
	UINT max_host_cache = 0;

/*
	vector<GnutellaHost> connect_hosts;
	CompressedQRPTable compressed_qrp_table;
*/

	thread_data.p_status_data=&status_data;
	thread_data.p_keywords=&keywords;
	thread_data.p_supply=&supply;
	thread_data.p_project_supply_queries=&project_supply_queries;
	thread_data.p_max_host=&max_host;
	thread_data.p_max_host_cache=&max_host_cache;

/*
	thread_data.p_connect_hosts=&connect_hosts;
	thread_data.p_compressed_qrp_table=&compressed_qrp_table;
*/
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
	// Create the Blubster Socket and add it's event handle to the events vector
	BlubsterSocket m_socket;
	m_socket.InitHwnd(hwnd);
	m_socket.InitParent((ConnectionModule*)pParam);
	int num_socket_events=1;
	events[4]=m_socket.ReturnEventHandle();

	DWORD num_events=num_reserved_events+num_socket_events;
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

	thread_data.p_vendor_counts = m_socket.ReturnVendorCountsPointer();

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
			vector<SupplyProject> tmp_supply;
			vector<ProjectSupplyQuery> tmp_project_supply_queries;
			UINT tmp_max_host = 0;
			UINT tmp_max_host_cache = 0;

			/*
			vector<GnutellaHost> tmp_connect_hosts;
			CompressedQRPTable tmp_compressed_qrp_table;
			*/
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
				if(supply.size()>0)
				{
					tmp_supply=supply;
					supply.clear();
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
				/*
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
				*/

				singleLock.Unlock();
			}

			// Call the functions, passing the temp data
			
			if(tmp_keywords.size()>0)
			{
				m_socket.UpdateProjectKeywords(tmp_keywords);
			}
			if(tmp_supply.size()>0)
			{
				m_socket.UpdateSupply(tmp_supply);
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
			/*
			if(tmp_connect_hosts.size()>0)
			{
				sockets.ConnectToHosts(tmp_connect_hosts);
			}
			if(tmp_compressed_qrp_table.GetLen()>0)
			{
				sockets.UpdateCompressedQRPTable(tmp_compressed_qrp_table);
			}
			*/
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
				
				status_data=tmp_status_data;	 // copy the temp data
				m_socket.TimerHasFired();				

				singleLock.Unlock();
			}

			PostMessage(hwnd,WM_DEMAND_SUPPLY_DATA_READY,0,0);
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
	m_header_counter=0;
	m_max_host=0;
	m_max_host_cache=0;

	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ConnectionModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	p_critical_section=NULL;

	// Start the worker thread...passing it the handle to the message window
	m_hwnd=m_wnd.GetSafeHwnd();
	AfxBeginThread(ConnectionModuleThreadProc,(LPVOID)this,THREAD_PRIORITY_BELOW_NORMAL);
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
		p_manager->ReportVendorCounts(m_thread_data.p_vendor_counts);
	}
}

// Message Window Functions

//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	m_thread_data=*((ConnectionModuleThreadData *)lparam);
	KeywordsUpdated();
	ProjectSupplyUpdated(NULL);

}

//
//
//
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
/*	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].TimerHasFired();
	}

	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_status_data.m_connected_socket_count++;
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}

			// Get the ip of the connection and the connection status of the socket
			m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}
*/
	ConnectionModuleStatusData ret=m_status_data;
	
	m_status_data.ClearCounters();
	
	return ret;
}

//
//
//
void ConnectionModule::DemandSupplyDataReady()
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
/*
	// Init mod-specific data
	status.m_mod=m_mod;
	status.p_mod=this;

	// Store the latest vector of sorted connected host ips
	v_connected_host_ips=status.v_connected_host_ips;

	// sort the list of connected ips
	sort(status.v_connected_host_ips.begin(),status.v_connected_host_ips.end());
*/
	// Report the status to the parent
	p_manager->DemandSupplyDataReady(status);
}

//
//
//
void ConnectionModule::ProjectSupplyUpdated(char* project)
{
	if(p_critical_section==NULL)
	{
		return;
	}

	v_supply = p_manager->ReturnSupplyProjects();

	// Check to see if I need to update all of the projects
	if(project==NULL)
	{
		// Tell the thread that the keywords need to be updated
//		p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 1 - Before LOCK\n",0,false,true);	// black italic
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			(*m_thread_data.p_supply)=v_supply;
	
			WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
			singleLock.Unlock();
		}
//		p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 1 - After LOCK\n",0,false,true);	// black italic
	
		return;	
	}
/*
	// Else I gotta find which project it is that is updated

	// Find this project in the supply vector
	for(i=0;i<(int)v_supply.size();i++)
	{
		if(strcmp(v_supply[i].m_name.c_str(),project)==0)
		{
			vector<SupplyProject> supply_projects;
			supply_projects.push_back(v_supply[i]);
			supply_projects[0].m_name+=UPDATE_STRING;	// indicate it is an update when "(update)" is appended to the project name
	
			// Tell the thread that the keywords need to be updated
//			p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 2 - Before LOCK\n",0,false,true);	// black italic
			CSingleLock singleLock(p_critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				(*m_thread_data.p_supply)=supply_projects;
	
				WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
				singleLock.Unlock();
			}			
//			p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::ProjectSupplyUpdated() - 2 - After LOCK\n",0,false,true);	// black italic

			return;
		}
	}

	// Check to see if it could possibly be an update
	if(supply.size()==1)
	{
		if(strstr(supply[0].m_name.c_str(),UPDATE_STRING)!=NULL)
		{
			char *project=new char[supply[0].m_name.size()+1];
			strcpy(project,supply[0].m_name.c_str());
			*strstr(project,UPDATE_STRING)='\0';	// remove the update string from the project name
			supply[0].m_name=project;
			delete [] project;

			// Find the project in the supply, and update it
			for(i=0;i<v_supply.size();i++)
			{
				if(strcmp(v_supply[i].m_name.c_str(),supply[0].m_name.c_str())==0)
				{
					v_supply[i]=supply[0];
					break;
				}
			}

			return;
		}
	}

	// Else it's not an update, so just replace the local vector
	v_supply=supply;
*/

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

	psq.m_header_counter=++m_header_counter;
	// Tell the thread to connect to these hosts
	//p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::PerformProjectSupplyQuery() - Before LOCK\n",0,false,true);	// black italic

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		m_thread_data.p_project_supply_queries->push_back(psq);
		WSASetEvent(m_thread_data.m_reserved_events[1]);	// vector data
		singleLock.Unlock();
	}
	//p_manager->p_parent->m_log_window_manager.Log("ConnectionModule::PerformProjectSupplyQuery() - After LOCK\n",0,false,true);	// black italic

	// Remove any existing queries for this project so that all returns from the new query will not count the returns for any old queries
	// Make sure that if this query is a searcher query, that we only remove searcher queries...same with supply queries
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		if((strcmp(psq_iter->m_project.c_str(),psq.m_project.c_str())==0) /* =>_<= && (psq_iter->m_is_searcher_query==psq.m_is_searcher_query)*/)
		{
			v_project_supply_queries.erase(psq_iter);
			break;
		}
		psq_iter++;
	}

	// Save this query to the list of project supply queries that we've done 
	v_project_supply_queries.push_back(psq);
}

//
//
//
unsigned int ConnectionModule::GetHeaderCounter()
{
	return ++m_header_counter;
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
