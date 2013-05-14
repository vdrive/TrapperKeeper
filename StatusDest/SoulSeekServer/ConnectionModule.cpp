// ConnectionModule.cpp

#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"

#include <afxmt.h>	// for CCriticalSection

#include "ConnectionSockets.h"

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Create sockets for this module
	ConnectionSockets sockets(hwnd);

	// Init the message data structure and send it
	CCriticalSection critical_section;
	queue<SOCKET> qNewParentConnections;
	queue<char *> qNewParentIPs;

	ConnectionModuleThreadData thread_data;

	thread_data.p_critical_section = &critical_section;
	thread_data.m_pqNewParentConnections = &qNewParentConnections;
	thread_data.m_pqNewParentIPs = &qNewParentIPs;
	memcpy(thread_data.m_reserved_events,sockets.m_events,sizeof(WSAEVENT)*4);

	::PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&thread_data,(LPARAM)0);

	// Wait for events
	DWORD num_events=WSA_MAXIMUM_WAIT_EVENTS;
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

	TRACE("Module thread created");
	
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
			WSAResetEvent(sockets.m_events[event]);	// reset event
			break;
		}

		// Event 1 - Process all the items in the process queue and cancel all the transfers in the cancel queue
		if(event==1)
		{
			queue<SOCKET> qLocalSockets;
			queue<char *> qLocalIPs;

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				while( !thread_data.m_pqNewParentConnections->empty() )
				{
					qLocalSockets.push( thread_data.m_pqNewParentConnections->front() );
					thread_data.m_pqNewParentConnections->pop();

					qLocalIPs.push( thread_data.m_pqNewParentIPs->front() );
					thread_data.m_pqNewParentIPs->pop();
				}
				
				singleLock.Unlock();
			}

			while( !qLocalSockets.empty() )
			{
				sockets.AddParentConnection( qLocalSockets.front(), qLocalIPs.front() );
				qLocalSockets.pop();
				qLocalIPs.pop();
			}
		}

		// Event 2 - ?
		if(event==2)
		{
			WSAResetEvent(sockets.m_events[event]);	// reset event
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

	return 0;	// exit the thread
}

//
//
//
ConnectionModule::ConnectionModule()
{
	p_manager=NULL;
	m_nSocketsAvail = 60;

	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in ConnectionModule()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	// Start the worker thread...passing it the handle to the message window
	HWND hwnd=m_wnd.GetSafeHwnd();
	AfxBeginThread(ConnectionModuleThreadProc,(LPVOID)hwnd,THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
ConnectionModule::~ConnectionModule()
{
	// Kill the thread if we have received initialized the thread data
	if(m_thread_data.p_critical_section!=NULL)
	{
		WSASetEvent(m_thread_data.m_reserved_events[0]);
	}
	else
	{
		// Oh well
	}

	// Destroy Window
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
void ConnectionModule::AddParentConnection(SOCKET socket, char* ip)
{
	if(m_nSocketsAvail > 0 && m_nSocketsAvail <= 60) 
	{
		
	}
	else return;

	// First check to see if we have received the thread data. If not, then we have to either cache this data or ignore it.
	if(m_thread_data.p_critical_section==NULL)
	{
		m_qWaitingSockets.push( socket );
		m_qWaitingIPs.push( ip );
		TRACE("Pushing Socket onto queue since critical_section isnt created yet");
		return;
	}

	// We have received the thread data message and can communicate with the thread

	DecrementSocketsAvail();  //decrament counter

	// Enter the critical section and give the request data to the thread in a thread-safe manner
	CSingleLock singleLock(m_thread_data.p_critical_section);
	singleLock.Lock();
	if (singleLock.IsLocked())
	{
		// Set the event to tell the thread that there is data waiting for it
		WSASetEvent(m_thread_data.m_reserved_events[1]);

		m_thread_data.m_pqNewParentIPs->push( ip );
		m_thread_data.m_pqNewParentConnections->push( socket );	
		TRACE("Pushing Socket data");

		singleLock.Unlock();
	}
}

//
// Messages
//
void ConnectionModule::LogMsg(char *pMsg)
{
	p_manager->LogMsg( pMsg );
}

//
// Connection Recieved
//
void ConnectionModule::ReceivedConnection(unsigned int token)
{
	p_manager->ReceivedConnection(token);
}

//
// Connection Recieved
//
void ConnectionModule::ProcessSearchRequest(SearchRequest* sr)
{
	p_manager->ProcessSearchRequest(sr);
}

//
//
//
void ConnectionModule::IncrementSocketsAvail()
{
	//p_manager->UpdateParentSockets(-1,0,0);
	m_nSocketsAvail++;
}
//
//
//
void ConnectionModule::DecrementSocketsAvail()
{
	//p_manager->UpdateParentSockets(1,0,0);
	m_nSocketsAvail--;
}

//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	ConnectionModuleThreadData *thread_data=(ConnectionModuleThreadData *)wparam;
	m_thread_data=*thread_data;

	// Check to see if there is any cached data. If there is, enter the critical secion and give the request data to the thread in a thread-safe manner.

	if( !m_qWaitingSockets.empty() )
	{
		CSingleLock singleLock(m_thread_data.p_critical_section);
		singleLock.Lock();
		if (singleLock.IsLocked())
		{
			// Set the event to tell the thread that there is data waiting for it
			WSASetEvent(m_thread_data.m_reserved_events[1]);

			while( !m_qWaitingSockets.empty() )
			{
				DecrementSocketsAvail();  //decrament counter
				m_thread_data.m_pqNewParentConnections->push( m_qWaitingSockets.front() );
				m_thread_data.m_pqNewParentIPs->push(m_qWaitingIPs.front());
				m_qWaitingSockets.pop();
				m_qWaitingIPs.pop();
				TRACE("Removing sockets from queue");
			}
			singleLock.Unlock();
		}
	}
}

