// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
//#include "ConnectionManager.h"
#include "PioletPoisoner.h"

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
//	ConnectionModuleStatusData status_data;
	ConnectionModuleThreadData thread_data;
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
//	m_socket.InitHwnd(hwnd);
	m_socket.InitParent((ConnectionModule*)pParam);
	int num_socket_events=1;
	events[4]=m_socket.ReturnEventHandle();

	DWORD num_events=num_reserved_events+num_socket_events;
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

//	thread_data.p_vendor_counts = m_socket.ReturnVendorCountsPointer();

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
				WSAResetEvent(events[event]);	// reset event
				
				m_socket.TimerHasFired(thread_data.m_hwnd);				

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
				WSAResetEvent(events[event]);	// reset event

				m_socket.OneMinuteTimer(thread_data.m_hwnd);
				singleLock.Unlock();
			}
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
ConnectionModule::ConnectionModule(PioletPoisoner *parent)
{
//	m_header_counter=0;
//	m_max_host=0;
//	m_max_host_cache=0;
//	p_supply=NULL;

	// Create message window so that it will accept messages posted to it
	FTConnectionModule* ftm = new FTConnectionModule();
	ftm->InitParent(this);
	v_ft_connection_modules.push_back(ftm);

	p_parent = parent;
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ConnectionModule()","Error",MB_OK);
	}
	
	m_wnd.InitParent(this);
	p_thread_data = NULL;

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
	vector<FTConnectionModule*>::iterator iter = v_ft_connection_modules.begin();
	while(iter != v_ft_connection_modules.end())
	{
		delete *iter;
		v_ft_connection_modules.erase(iter);
	}
	vector<ListeningSocket*>::iterator iter_list = v_listening_sockets.begin();
	while(iter_list != v_listening_sockets.end())
	{
		delete *iter_list;
		v_listening_sockets.erase(iter_list);
	}
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
void ConnectionModule::InitParent(PioletPoisoner *parent)
{
	p_parent=parent;
	m_main_hwnd = p_parent->m_dlg.GetSafeHwnd();

	//testing
	/*
	ListeningSocket* socket = new ListeningSocket();
	socket->InitParent(this);
	socket->Create(10248,"38.119.64.12",5678912,"ODSK4K345LKJLGJ346509SFKDJH435");
	v_listening_sockets.push_back(socket);
	*/
	
}

//
//
//
void ConnectionModule::TimerHasFired()
{
	for(UINT i=0; i<v_ft_connection_modules.size(); i++)
	{
		v_ft_connection_modules[i]->TimerHasFired(m_main_hwnd);
	}
	::PostMessage(m_main_hwnd, WM_NUM_FT_MODULE, (WPARAM)v_ft_connection_modules.size(),0);
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[2]);	// timer has fired
//		p_manager->ReportVendorCounts(m_thread_data.p_vendor_counts);
	}
	::PostMessage(m_main_hwnd,WM_LISTENING_SOCKETS_STATUS,(WPARAM)v_listening_sockets.size(),0);
}

// Message Window Functions

void ConnectionModule::OneMinuteTimer()
{
	vector<FTConnectionModule*>::iterator iter = v_ft_connection_modules.begin();
	while(iter != v_ft_connection_modules.end())
	{
		(*iter)->OneMinuteTimer();
		if( (*iter)->GetNumIdleSockets() == 60 && (v_ft_connection_modules.size() > 1))
		{
			SendMessage(m_main_hwnd, WM_REMOVE_FT_MODULE, (WPARAM)(*iter), 0);
			delete (*iter);
			v_ft_connection_modules.erase(iter);
		}
		else
			iter++;
	}
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[3]);	// one min timer has fired
//		p_manager->ReportVendorCounts(m_thread_data.p_vendor_counts);
	}

	vector<ListeningSocket*>::iterator iter_list = v_listening_sockets.begin();
	while(iter_list != v_listening_sockets.end())
	{
		(*iter_list)->CheckIdle();
		iter_list++;
	}
}

//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data=(ConnectionModuleThreadData *)lparam;
	p_thread_data->m_hwnd = p_parent->m_dlg.GetSafeHwnd();
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

void ConnectionModule::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	p_parent->Log(text, color, bold, italic);
}

//
//
//
/*
PoisonEntry ConnectionModule::GetPoisonEntry(char* filename)
{
	return p_parent->GetPoisonEntry(filename);
}

*/
//
//
//
void ConnectionModule::ReceivedFileRequest(PoisonEntry* pe)
{
	//TRACE("Accepting connection from TCP port %d\n",pe->m_request_port);
	::SendMessage(m_main_hwnd,WM_FILE_REQUEST,(WPARAM)pe,0);
	
	/*
	bool found_listening_socket=false;
	bool existing_port_in_use=false;

	vector<ListeningSocket*>::iterator iter = v_listening_sockets.begin();
	while(iter != v_listening_sockets.end())
	{
		if((*iter)->m_waiting_for_connection && (*iter)->m_port == port)
		{
			existing_port_in_use=true;
			break;
		}
		iter++;
	}
	if(!existing_port_in_use)
	{
		iter = v_listening_sockets.begin();
		while(iter != v_listening_sockets.end())
		{
			if(!(*iter)->m_waiting_for_connection)
			{
				(*iter)->Create(pe->m_request_port,pe->m_requester_ip.c_str(),pe->m_filesize,pe->m_md5.c_str());
				found_listening_socket=true;
				break;
			}
			iter++;
		}
		if(!found_listening_socket)
		{
			ListeningSocket* socket = new ListeningSocket();
			socket->InitParent(this);
			socket->Create(pe->m_request_port,pe->m_requester_ip.c_str(),pe->m_filesize,pe->m_md5.c_str());
			v_listening_sockets.push_back(socket);
		}
	}
	*/
}

//
//
//
void ConnectionModule::OnReceivedFileRequest(PoisonEntry* pe)
{
	bool found_listening_socket=false;
	bool existing_port_in_use=false;

	vector<ListeningSocket*>::iterator iter = v_listening_sockets.begin();
	while(iter != v_listening_sockets.end())
	{
		if((*iter)->m_waiting_for_connection && (*iter)->m_port == pe->m_request_port)
		{
			existing_port_in_use=true;
			break;
		}
		iter++;
	}
	if(!existing_port_in_use)
	{
		iter = v_listening_sockets.begin();
		while(iter != v_listening_sockets.end())
		{
			if(!(*iter)->m_waiting_for_connection && (*iter)->m_port == pe->m_request_port)
			{
				(*iter)->Create(pe->m_request_port,pe->m_requester_ip.c_str(),pe->m_filesize,pe->m_md5.c_str());
				found_listening_socket=true;
				break;
			}
			iter++;
		}
		if(!found_listening_socket)
		{
			ListeningSocket* socket = new ListeningSocket();
			socket->InitParent(this);
			socket->Create(pe->m_request_port,pe->m_requester_ip.c_str(),pe->m_filesize,pe->m_md5.c_str());
			v_listening_sockets.push_back(socket);
		}
	}
	delete pe;
}
//
//
//
void ConnectionModule::AcceptedConnection(SOCKET hSocket, int port, const char* ip, UINT filesize, const char* md5)
{
	// Need to give this connection to a module with an idle socket
	for(UINT i=0;i<v_ft_connection_modules.size();i++)
	{
		if(v_ft_connection_modules[i]->GetNumIdleSockets()>0)
		{	
			v_ft_connection_modules[i]->AcceptConnection(hSocket,ip,port,filesize,md5);
			return;
		}
	}

	// If we didn't find a module to accept the connection, then create a new one
	FTConnectionModule *mod=new FTConnectionModule();
	v_ft_connection_modules.push_back(mod);

	mod->InitParent(this);
	mod->AcceptConnection(hSocket,ip,port,filesize,md5);

	/*
	bool found_free_socket = false;
	vector<FTConnectionModule*>::iterator iter = v_ft_connection_modules.begin();
	while(iter != v_ft_connection_modules.end())
	{
		if( (*iter)->AcceptConnection(hSocket,ip,port,filesize,md5))
		{
			found_free_socket = true;
			break;
		}
		iter++;
	}
	if(!found_free_socket) //add a new module
	{
		FTConnectionModule* new_ftm = new FTConnectionModule();
		new_ftm->CAcceptConnection(hSocket,ip,port,filesize,md5);
		v_ft_connection_modules.push_back(new_ftm);
	}
	*/
}

//
//
//
void ConnectionModule::KillListeningSocket(ListeningSocket* socket)
{
	vector<ListeningSocket*>::iterator iter_list = v_listening_sockets.begin();
	while(iter_list != v_listening_sockets.end())
	{
		if(*iter_list == socket)
		{
			delete *iter_list;
			v_listening_sockets.erase(iter_list);
			break;
		}
		iter_list++;
	}	
}