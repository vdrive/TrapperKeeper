// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"
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

	thread_data.p_keywords=&keywords;
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
				singleLock.Unlock();
			}

			// Call the functions, passing the temp data
			
			if(tmp_keywords.size()>0)
			{
				m_socket.UpdateProjectKeywords(tmp_keywords);
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
	m_num_supplies=0;
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
	status.m_num_supplies=m_num_supplies;
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
void ConnectionModule::ReadInSupplyEntries()
{
	Log("Loading supply entries from harddrive started");
#ifdef SKYCAT
	for(UINT j=0;j<100;j++)
	{
		SupplyData data;
		data.m_file_type="mp3";
		data.m_bitrate=128;
		data.m_filename="skycat rocks.mp3";
		data.m_filesize=3456789;
		for(UINT i=0;i<16;i++)
			data.m_hash[i]=rand()%256;
		data.m_media_len==200;
		data.m_project_id=666666;
		data.m_track=1;
		SupplyDataSet set(data.m_project_id);
		pair<hash_set<SupplyDataSet>::iterator, bool > pr;
		pr=hs_supply_data.insert(set);
		if(data.m_track > (int)(pr.first->v_supply_data_by_track.size()-1))
		{
			while((pr.first->v_supply_data_by_track.size()-1) != data.m_track)
			{
				SupplyDataByTrack supply_track;
				pr.first->v_supply_data_by_track.push_back(supply_track);
			}
		}
		data.CreateOvernetPacket();
		pr.first->v_supply_data_by_track[data.m_track].v_supply_data.push_back(data);	
	}
#else
	CSingleLock singleLock(&m_supplies_lock);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		CFile file;
		if(file.Open("c:\\syncher\\rcv\\OvernetSupplies\\overnet_supplies.dat",
			CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)!=0)
		{
			if(file.GetLength())
			{
				byte* buf = new byte[(UINT)file.GetLength()];
				file.Read(buf,(UINT)file.GetLength());
				UINT count= *((UINT*)&buf[0]);
				char* ptr = (char*)&buf[sizeof(UINT)];
				if(count)
					hs_supply_data.clear();
				for(UINT i=0;i<count;i++)
				{
					bool protection_on=false;
					SupplyData data;
					ptr += data.ReadFromBuffer(ptr);

					for(UINT j=0;j<v_keywords.size();j++)
					{
						if(data.m_project_id==v_keywords[j].m_id)
						{
							if(v_keywords[j].m_supply_keywords.v_keywords.size())
							{
								for(UINT k=0;k<v_keywords[j].m_supply_keywords.v_keywords.size();k++)
								{
									if(data.m_track==v_keywords[j].m_supply_keywords.v_keywords[k].m_track)
									{
										if(v_keywords[j].m_supply_keywords.v_keywords[k].m_single)
										{
											SupplyDataSet set(data.m_project_id);
											pair<hash_set<SupplyDataSet>::iterator, bool > pr;
											pr=hs_supply_data.insert(set);

											if(data.m_track > (int)(pr.first->v_supply_data_by_track.size()-1))
											{
												while((pr.first->v_supply_data_by_track.size()-1) != data.m_track)
												{
													SupplyDataByTrack supply_track;
													pr.first->v_supply_data_by_track.push_back(supply_track);
												}
											}
											data.CreateOvernetPacket();
											pr.first->v_supply_data_by_track[data.m_track].v_supply_data.push_back(data);
										}
										protection_on=true;
										break;
									}
								}
							}
							else
							{
								SupplyDataSet set(data.m_project_id);
								pair<hash_set<SupplyDataSet>::iterator, bool > pr;
								pr=hs_supply_data.insert(set);
								if(pr.second)
								{
									SupplyDataByTrack supply_track;
									pr.first->v_supply_data_by_track.push_back(supply_track);	
								}
								data.CreateOvernetPacket();
								pr.first->v_supply_data_by_track[0].v_supply_data.push_back(data);
								break;
							}
							if(protection_on)
								break;
						}
					}
				}
				delete [] buf;
			}
			file.Close();
		}
		singleLock.Unlock();
	}
#endif
	Log("Loading supply entries from harddrive ended");
	m_num_supplies = GetNumSupplies();
}

//
//
//
void ConnectionModule::SupplySynched()
{
	KeywordsUpdated();
}

//
//
//
UINT ConnectionModule::GetNumSupplies()
{
	UINT total=0;
	hash_set<SupplyDataSet>::iterator iter=hs_supply_data.begin();
	while(iter!=hs_supply_data.end())
	{
		total+=iter->GetNumSupplies();
		iter++;
	}
	return total;
}

//
//
//
hash_set<SupplyDataSet>::iterator ConnectionModule::GetSupplyPointer(UINT project_id)
{
	SupplyDataSet set(project_id);
	hash_set<SupplyDataSet>::iterator iter = hs_supply_data.find(set);
	if(iter!=hs_supply_data.end())
		return iter;
	else
		return NULL;	 
}
