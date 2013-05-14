// ConnectionModule.cpp

#include "stdafx.h"
#include "ClientModule.h"
#include "ConnectionManager.h"
#include <time.h>

#include <afxmt.h>	// for CCriticalSection

#include "ClientSockets.h"

//
//
//
UINT ClientModuleThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Create sockets for this module
	ClientSockets sockets(hwnd);

	// Init the message data structure and send it
	CCriticalSection critical_section;
	ClientModuleThreadData thread_data;

	//queue<PacketWriter *> qPacketsToSend;

	//thread_data.p_critical_section = &critical_section;
	//thread_data.p_qPacketsToSend = &qPacketsToSend;
	memcpy(thread_data.m_reserved_events,sockets.m_events,sizeof(WSAEVENT)*4);

	::PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&thread_data,(LPARAM)&critical_section);

	::PostMessage(hwnd,WM_CMOD_START_TIMER,NULL,NULL);

	// Wait for events
	DWORD num_events=WSA_MAXIMUM_WAIT_EVENTS;
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

	srand ( (unsigned int)time(NULL) );

	TRACE("Client Module thread created");
	
	while(1)
	{
		event=WSAWaitForMultipleEvents(num_events,&sockets.m_events[0],wait_all,timeout,alertable);
		
		if(event==WSA_WAIT_FAILED)
		{
			::MessageBox(NULL,"WSAWaitForMultipleEvents() Failed","Error",MB_OK);
			break;
		}

		// Check to see if it the triggered event is any of the 4 reserved events

		// Event 0 - Check to see if this is the kill thread event (reserved event 0)
		if(event==0)
		{
			{
				CSingleLock singleLock(&critical_section, true);
				WSAResetEvent(sockets.m_events[event]);	// reset event
			}
			//::PostMessage(hwnd,WM_CMOD_STOP_TIMER,NULL,NULL);
			sockets.ClearAll();
			break;
		}

		// Event 1 - Process all the items in the process queue and cancel all the transfers in the cancel queue
		if(event==1)
		{
			queue<PacketWriter *>* qLocalPackets = NULL;

			{
				CSingleLock singleLock(&critical_section, true);
				qLocalPackets = thread_data.p_qPacketsToSend;
				thread_data.p_qPacketsToSend = NULL;
			}

			while( !qLocalPackets->empty() )
			{
				//delete qLocalPackets->front();
				//::PostMessage(hwnd, WM_CMOD_CLOSE_CONNECTION, NULL, NULL );
				sockets.SendPacket(qLocalPackets->front());
				qLocalPackets->pop();
			}
			delete qLocalPackets;
			qLocalPackets = NULL;
			{
				CSingleLock singleLock(&critical_section, true);
				WSAResetEvent(sockets.m_events[event]);	// reset event
			}
		}

		// Event 2 - OnTimer
		if(event==2)
		{
			sockets.CheckForTimeout();
			{
				CSingleLock singleLock(&critical_section, true);
				WSAResetEvent(sockets.m_events[event]);	// reset event
			}
		}

		// Event 3 - For this example, connect to the listening port
		if(event==3)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event
		}

		// Else it is a socket event
		if(event>3)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event

			unsigned int index=event-4;
			
			WSANETWORKEVENTS events=sockets.m_sockets[index].ReturnNetworkEvents();
			
			if(events.lNetworkEvents & FD_CONNECT)
			{
				sockets.m_sockets[index].OnConnect(events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(events.lNetworkEvents & FD_READ)
			{
				// Keep receiving until OnReceive returns false
				while(sockets.m_sockets[index].OnReceive(events.iErrorCode[FD_READ_BIT]));
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
	::PostMessage(hwnd,WM_CMOD_KILL,NULL,NULL);
	return 0;	// exit the thread
}

//
//
//
ClientModule::ClientModule(int id)
: p_critical_section(NULL), p_thread_data(NULL), p_manager(NULL), p_thread(NULL)
{
	sprintf(m_id,"%i",id);
	m_shuttingdown = false;
	m_nSocketsAvail = 60;

	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ClientModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	// Start the worker thread...passing it the handle to the message window
	HWND hwnd=m_wnd.GetSafeHwnd();
	p_thread = 
		AfxBeginThread(ClientModuleThreadProc,(LPVOID)hwnd,THREAD_PRIORITY_NORMAL);
}

//
//
//
ClientModule::~ClientModule()
{
	m_wnd.StopTimer(NULL,NULL);
	while(!m_qWaitingPacketsToSend.empty())
	{
		if( m_qWaitingPacketsToSend.front() != NULL )
			delete m_qWaitingPacketsToSend.front();
		m_qWaitingPacketsToSend.pop();
	}
	// Kill the thread if we have received initialized the thread data
	if(p_critical_section!=NULL && m_shuttingdown == false)
	{
		
		CSingleLock singleLock(p_critical_section, true);
		WSASetEvent(p_thread_data->m_reserved_events[0]);
		p_thread_data = NULL;
		p_critical_section = NULL;
	}
	m_shuttingdown = true;
	//if(p_thread != NULL)
	//{
	//	WaitForSingleObject( p_thread->m_hThread, 2000 );
		p_thread = NULL;
	//}


	// Destroy Window
	m_wnd.DestroyWindow();
}

void ClientModule::Shutdown()
{
	m_shuttingdown = true;
	m_wnd.StopTimer(NULL,NULL);
	
	while(!m_qWaitingPacketsToSend.empty())
	{
		if( m_qWaitingPacketsToSend.front() != NULL )
			delete m_qWaitingPacketsToSend.front();
		m_qWaitingPacketsToSend.pop();
	}

	// Kill the thread if we have received initialized the thread data
	if(p_critical_section!=NULL)
	{
		CSingleLock singleLock(p_critical_section, true);
		WSASetEvent(p_thread_data->m_reserved_events[0]);

		p_critical_section = NULL;
		p_thread_data = NULL;
	}
}

void ClientModule::DeleteThread()
{
	p_manager->DeleteClient(this);
}

//
//
//
void ClientModule::InitParent(ConnectionManager *manager)
{
	p_manager=manager;
}

void ClientModule::SetEvent(int event)
{
	if(p_critical_section == NULL) return;
	if(event < 4)
	{
		{
			CSingleLock singleLock(p_critical_section, true);
			WSASetEvent(p_thread_data->m_reserved_events[event]);
		}
	}
}
//
//
//
bool ClientModule::SendPacket(PacketWriter *packet)
{
	if(m_shuttingdown) 
	{
		TRACE("Module is shutting down, returning false\n");
		return false;
	}
	if(!(m_nSocketsAvail > 0 && m_nSocketsAvail <= 60) )
	{
		TRACE("Error adding client socket in ClientModule.cpp\nSocket Count:  %i\n",m_nSocketsAvail);
		return false;
	}

	DecrementSocketsAvail();  //decrament counter

	// First check to see if we have received the thread data. If not, then we have to either cache this data or ignore it.
	if(p_critical_section==NULL)
	{
		m_qWaitingPacketsToSend.push( packet );
		TRACE("Pushing packet onto queue since critical_section isnt created yet");
		return true;
	}

	// We have received the thread data message and can communicate with the thread


	// Enter the critical section and give the request data to the thread in a thread-safe manner
	{
		CSingleLock singleLock(p_critical_section, true);
		// Set the event to tell the thread that there is data waiting for it
		
		if( p_thread_data->p_qPacketsToSend == NULL )
			p_thread_data->p_qPacketsToSend = new queue<PacketWriter *>;

		p_thread_data->p_qPacketsToSend->push( packet );
		WSASetEvent(p_thread_data->m_reserved_events[1]);
	}
	return true;
}
//
//
//
void ClientModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	//ClientModuleThreadData *thread_data=(ClientModuleThreadData *)wparam;
	//m_thread_data=*thread_data;
	p_critical_section = (CCriticalSection *)lparam;
	p_thread_data = (ClientModuleThreadData *)wparam;
	// Check to see if there is any cached data. If there is, enter the critical secion and give the request data to the thread in a thread-safe manner.

	if( !m_qWaitingPacketsToSend.empty() )
	{
		{
			CSingleLock singleLock(p_critical_section, true);
			// Set the event to tell the thread that there is data waiting for it
			if( p_thread_data->p_qPacketsToSend == NULL )
				p_thread_data->p_qPacketsToSend = new queue<PacketWriter *>;
			
			while( !m_qWaitingPacketsToSend.empty() )
			{
				p_thread_data->p_qPacketsToSend->push( m_qWaitingPacketsToSend.front() );
				m_qWaitingPacketsToSend.pop();
				TRACE("Removing packet from queue\n");
			}
			WSASetEvent(p_thread_data->m_reserved_events[1]);
		}
	}
}


//
// Messages
//
void ClientModule::LogMsg(char *pMsg)
{
	p_manager->LogMsg( pMsg );
}

//
//
//
void ClientModule::IncrementSocketsAvail()
{
	m_nSocketsAvail++;
}
//
//
//
void ClientModule::DecrementSocketsAvail()
{
	m_nSocketsAvail--;
}

//
//
//
void ClientModule::CloseConnection()
{
	p_manager->CloseClientConnection();
}

void ClientModule::OpenAnotherConnection()
{
	p_manager->OpenAnotherClientConnection();
}
void ClientModule::WriteToFile(char* s)
{
	string t;
	t = m_id;
	t.append(" - ");
	t.append(s);
	t.append("\r\n");
	if(p_manager != NULL)
		p_manager->WriteToFile(t);
	delete s;
}


