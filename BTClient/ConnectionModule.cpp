// ConnectionModule.cpp
#include "stdafx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"
#include <algorithm>
using namespace std;


#include <afxmt.h>	// for CCriticalSection

// For thread
#include "ConnectionSockets.h"
#include "ConnectionModuleStatusData.h"
//#include "VendorCount.h"

//#include "FastTrackGiftDll.h"	// temp kludge for logging

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
//	UINT i;
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	ConnectionModuleThreadData thread_data;
	ConnectionModuleStatusData status_data;
//	vector<SupernodeHost> connect_hosts;
	vector<SOCKET> accepted_sockets;

	// Create the sockets for this module
	ConnectionSockets sockets;
	sockets.m_dlg_hwnd = hwnd;
	memcpy(&thread_data.m_reserved_events[0],&sockets.m_events[0],sizeof(WSAEVENT)*5);
	thread_data.p_status_data=&status_data;
	thread_data.p_accepted_sockets=&accepted_sockets;
//	thread_data.p_connect_hosts=&connect_hosts;
//	SendMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);
	PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);
//	sockets.p_file_sharing_manager = thread_data.p_file_sharing_manager;

	int num_reserved_events=sockets.ReturnNumberOfReservedEvents();

	WSANETWORKEVENTS events;	// what the fired event data was

	DWORD num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=FALSE;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=FALSE;
	DWORD event;	// which event fired

	TorrentFile torrent;
	int listening_port;
	
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
			// Temp variables
//			vector<SupernodeHost> tmp_connect_hosts;
			
			// Copy the critical data to the temp variables
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event
				torrent = thread_data.m_torrent;
				listening_port = thread_data.m_port;
			
/*
				if(accepted_sockets.size()>0)
				{
					for(int i=0;i<(int)accepted_sockets.size();i++)
					{
						sockets.AddNewClient(accepted_sockets[i]);
					}
					accepted_sockets.clear();
				}
*/


			// connect to the trackers
				sockets.InitTorrent(torrent);
				sockets.RegisterWithTracker(&torrent, listening_port);
			// Start spitting out sockets here, it'll be fun.
				PeerList * peers = torrent.GetPeers();
				int peercount = peers->GetCount();
			// temp KLUDGE for debug testing if we have more than 50 peers just use 50
			
				if (peercount > 55)
				{
					peercount = 55;
				}
			
				for(int i = 0;i < peercount ;i++)
				{
				//DEBUG connecting to everyone for the test
	//			if (peers->IsSeed())
					if (peers != NULL)
					{
						char * connectip = new char[strlen(peers->GetIP().c_str())];
						while (peers != NULL)
						{

							strcpy(connectip,peers->GetIP().c_str());
							if ((strstr(connectip,"38.118.151")==NULL)&&(strstr(connectip,"204.9.116")==NULL)&&(strstr(connectip,"38.118.154")==NULL))
							{
								break;
							}
							else
							{
								peers = peers->GetNext();
							}
						}
						if (peers != NULL)
						{
							bool is_seed = peers->IsSeed();
							sockets.MakeNewConnection(connectip,atoi(peers->GetPort().c_str()), is_seed);
							peers = peers->GetNext();	
						}
					//connectsocket->Connect(connectip,atoi(peers[i].GetPort().c_str()));
					//mv_sockets.push_back(connectsocket);
					//delete [] connectip;  // This is deleted elsewhere
					//break;  // just testing one for now
					}
					char msg[1024+1];
					sprintf(msg,"peercount = %d i = %d, getting next peer",peercount,i);
					CFile logfile;
					/*
					if (logfile.Open("BTConnectionModuleLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
					{
						logfile.SeekToEnd();
						logfile.Write(msg, (unsigned int)strlen(msg));
						logfile.Write("\r\n",2);
						logfile.Close();
					}
					*/

					sprintf(msg,"Got next peer");
					/*
					if (logfile.Open("BTConnectionModuleLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
					{
						logfile.SeekToEnd();
						logfile.Write(msg, (unsigned int)strlen(msg));
						logfile.Write("\r\n",2);
						logfile.Close();
					}
					*/
				}

				singleLock.Unlock();
			}
			
			/*
			// Call the functions, passing the temp data
			if(tmp_connect_hosts.size()>0)
			{
				//if(!thread_data.m_disconnect_to_hosts)
				//	sockets.ConnectToHosts(tmp_connect_hosts);
			}
			if(thread_data.m_reconnect_to_hosts)
			{
				//thread_data.m_reconnect_to_hosts = false;
				//sockets.ReConnectAll();
			}
			if(thread_data.m_disconnect_to_hosts)
			{
				//sockets.ReConnectAll();
			}
*/
		}

		// The Timer has fired send keep alive messages
		if(event==2)
		{
		
			//LogMess("Timer Fired Message received");
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				//LogMess("Timer Fired Message Lock");
				WSAResetEvent(sockets.m_events[event]);	// reset event
				
				int num_connected = sockets.CheckNumberOfConnections();

				if (num_connected > 0)
				{
					sockets.KeepConnectionsAlive();
				}
				else
				{
					PostMessage(hwnd,WM_KILL_MODULE	,(WPARAM)&critical_section,(LPARAM)&thread_data);
				}
				singleLock.Unlock();
				//LogMess("Timer Fired Message UnLocked");
			}

		}
		/*
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
*/
		// Event 3
		
		if(event==3)
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				accepted_sockets = *thread_data.p_accepted_sockets;
				if(accepted_sockets.size()>0)
				{
					for(int i=0;i<(int)accepted_sockets.size();i++)
					{
						sockets.AddNewClient(accepted_sockets[i]);
					}
					accepted_sockets.clear();
				}
				
				singleLock.Unlock();
			}


		}
/*
//			vector<VendorCount> *tmp_vendor_counts=sockets.ReturnVendorCounts();

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				WSAResetEvent(sockets.m_events[event]);	// reset event

				// Do nothing else

				singleLock.Unlock();
			}

//			PostMessage(hwnd,WM_VENDOR_COUNTS_READY,(WPARAM)tmp_vendor_counts,(LPARAM)0);
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
				while (sockets.m_sockets[index].OnReceive(events.iErrorCode[FD_READ_BIT]));
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
	char * id = CreatePeerID();
	strcpy(m_peer_id,id);

	m_modnum = rand()%20000;
	m_is_listener = false;
	delete[]id;


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
	//SetTimer(m_hwnd,6,5*1000,0);
	// 2 min keep alive timer
	SetTimer(m_hwnd,7,5*60*1000,0);
}


ConnectionModule::ConnectionModule(int modnum)
{
	char * id = CreatePeerID();
	strcpy(m_peer_id,id);

	delete[]id;

	m_modnum = modnum;
	m_is_listener = false;

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
	//SetTimer(m_hwnd,6,5*1000,0);
	// 2 min keep alive timer
	SetTimer(m_hwnd,7,5*60*1000,0);
}

//
//
//
ConnectionModule::~ConnectionModule()
{
	KillTimer(m_hwnd,7);
	// Kill the thread
	if(p_critical_section!=NULL)
	{
		WSASetEvent(p_thread_data->m_reserved_events[0]);
		p_critical_section = NULL;
	}
	if(m_thread!=NULL)
	{
		if(WaitForSingleObject(m_thread->m_hThread,5000)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
		m_thread = NULL;
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
	m_config = manager->GetConfig();
}

//
//
//
//void ConnectionModule::ConnectToHosts(vector<SupernodeHost> &hosts)
//{
//	
//	if(p_critical_section==NULL)
//	{
//		return;
//	}
//
//	int i;
//
//	// Tell the thread to connect to these hosts
//	CSingleLock singleLock(p_critical_section);
//	singleLock.Lock();
//	if(singleLock.IsLocked())
//	{
//		*p_thread_data->p_connect_hosts=hosts;
//		WSASetEvent(p_thread_data->m_reserved_events[1]);	// vector data
//		singleLock.Unlock();
//	}
//
//	// Keep track of the connect to hosts for when we are checking to see if we can add an ip to the cache
//	v_connect_to_ips.clear();
//	for(i=0;i<(int)hosts.size();i++)
//	{
//		v_connect_to_ips.push_back(hosts[i].m_ip);
//	}
//	
//	sort(v_connect_to_ips.begin(),v_connect_to_ips.end());
//
//	// Add these hosts to the connected hosts vector, just so that another mod won't connect to them since i am connecting to them
////	for(i=0;i<hosts.size();i++)
////	{
////		v_connected_host_ips.push_back(hosts[i].IP());
////	}
//
//}

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
	//if(binary_search(v_connected_host_ips.begin(),v_connected_host_ips.end(),ip) || binary_search(v_connect_to_ips.begin(),v_connect_to_ips.end(),ip))
	//{
	//	return true;
	//}
	//else
	//{
	//	return false;
	//}

	return false;
}


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
//	p_thread_data->p_file_sharing_manager = p_manager->p_file_sharing_manager;
/*
	if(v_initial_accepted_connections.size()>0)
	{
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			*p_thread_data->p_accepted_sockets=v_initial_accepted_connections;
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// set event
			singleLock.Unlock();
		}

		v_initial_accepted_connections.clear();
	}
	*/

		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			p_thread_data->m_torrent = m_torrent;
			p_thread_data->m_port = m_listening_port;
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// set event
			singleLock.Unlock();
		}
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
		case 7:	// 2 min timer
		{
			LogMess("Sending Timer Event Message");
			if(p_critical_section!=NULL)
			{
				WSASetEvent(p_thread_data->m_reserved_events[2]);
			}
			LogMess("Sent Timer Event Message");
				//SetTimer(m_hwnd,7,1*60*1000,0);
			SetTimer(m_hwnd,7,m_config.m_timer_time,0);
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
//		p_thread_data->p_status_data->m_mod=m_mod;
//		p_thread_data->p_status_data->p_mod=this;


		//// sort the list of connected ips
		//sort(p_thread_data->p_status_data->v_connected_host_ips.begin(),p_thread_data->p_status_data->v_connected_host_ips.end());

		//// Store the latest vector of sorted connected host ips
		//v_connected_host_ips=p_thread_data->p_status_data->v_connected_host_ips;

		//// Report the status to the parent
		//p_manager->ReportStatus(*(p_thread_data->p_status_data));

		//// Copy the status data and reset
		////status=*p_thread_data->p_status_data;
		//p_thread_data->p_status_data->Clear();

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
	SetTimer(m_hwnd,6,2*1000,0);
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
//	p_manager->VendorCountsReady((vector<VendorCount> *)wparam);
}

//
//
//
void ConnectionModule::LogMsg(WPARAM wparam,LPARAM lparam)
{
	p_manager->LogMsg((char*)wparam);
	char * ptr = (char*)wparam;
	delete [] ptr;
}

void ConnectionModule::DataReported(WPARAM wparam,LPARAM lparam)
{
	p_manager->ClientDataReported((ClientData *)wparam);
	ClientData * ptr = (ClientData *)wparam;
	delete ptr;
}

void ConnectionModule::ModuleDataReported(WPARAM wparam,LPARAM lparam)
{
	ModuleData * mod = (ModuleData *)wparam;
	mod->modnum = m_modnum;
	p_manager->ModuleDataReported(mod);
	//ModuleData * ptr = (ModuleData *)wparam;
	//delete ptr;
}


//
//
//
void ConnectionModule::ReConnectAll()
{
	if(p_critical_section==NULL)
	{
		return;
	}

//    v_connected_host_ips.clear();
//	v_connect_to_ips.clear();
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
	//if(v_connected_host_ips.size() >= 45)
	//	return true;
	//else
		return false;
}

void ConnectionModule::AddNewClient(SOCKET hSocket)
{
	if(p_critical_section==NULL)
	{
		v_initial_accepted_connections.push_back(hSocket);
		return;
	}

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		p_thread_data->p_accepted_sockets->push_back(hSocket);
		WSASetEvent(p_thread_data->m_reserved_events[3]);	// set event
		singleLock.Unlock();
	}
}

bool ConnectionModule::HasIdleSocket(void)
{
	if(m_status_data.m_idle_socket_count>0)
	{
		return true;
	}
	return false;
}



void ConnectionModule::test(TorrentFile * tf)
{
}

char * ConnectionModule::CreatePeerID() {
	char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	char * id = new char [21];
	strcpy(id, "-AZ2202-");
	srand( (unsigned)time( NULL ) );
	for(int i = 8; i < 20; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[20] = 0;
	return id;
}

unsigned char * ConnectionModule::GetHandshakeMessage()
{

	LogMess("Getting Handshake Message");
	unsigned char * message = new unsigned char[68];
	unsigned short * info_hash = m_torrent.GetInfoHash();
	string id = m_peer_id;
	string protocol = "BitTorrent protocol";
	int len = (int)protocol.length();
	int cur_pos = 0;
	message[0] = len;
	cur_pos++;
	memcpy(&message[1], protocol.c_str(), len); // protocol
	cur_pos += len;
	for(int i = 0; i < 8 ; i++ ) { // reserve bits
		message[i+cur_pos] = 0;
	}
	cur_pos += 8;
	for(int i = 0 ; i < 20; i++) {
		message[i+cur_pos] = (TCHAR)info_hash[i];
	}
	cur_pos += 20;
	memcpy(&message[cur_pos], id.c_str(), id.length());
	cur_pos += (int)id.length();

	LogMess("Returning Handshake Message");
	return message;

}

void ConnectionModule::InitTorrent(TorrentFile * tf,int listening_port)
{

	m_torrent = *tf;
	m_listening_port = listening_port;

}

int ConnectionModule::GetModnum()
{
	return m_modnum;
}

int ConnectionModule::GetListeningPort()
{
	if( m_listening_port !=NULL)
	{
		return m_listening_port;
	}
	else return(6881);
}

void ConnectionModule::KillYourself()
{
	// Don't kill listening mods even if they are empty, they will fill up later.
	if (m_is_listener == true)
	{
		
	}
	else
	{
		char message[256+1];
		sprintf(message,"Killing module number %d",m_modnum);
		char * msg = new char[strlen(message)];
		strcpy(msg,message);
		p_manager->LogMsg(msg);
		p_manager->KillModule(m_modnum);
	}
}

void ConnectionModule::SetAsListener()
{
	m_is_listener = true;
}

void ConnectionModule::LogMess(char * msg)
{
		
	CFile logfile;

	if (logfile.Open("BTConnectionModuleLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
	{
		if(logfile.GetLength()> 100000000)
		{
			logfile.Close();
			remove("BTConnectionModuleLog.txt");
			logfile.Open("BTConnectionModuleLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite);
		}
		logfile.SeekToEnd();
		logfile.Write(msg, (unsigned int)strlen(msg));
		logfile.Write("\r\n",2);
		logfile.Close();
	}
	

}

TorrentFile ConnectionModule::GetTorrentFile()
{
	return m_torrent;
}

void ConnectionModule::ConnectToNewClients(TorrentFile *torrent)
{

		if(p_critical_section != NULL)
		{
			p_thread_data->m_torrent = *torrent;
			p_thread_data->m_port = m_listening_port;
			WSASetEvent(p_thread_data->m_reserved_events[1]);	// set event
		}
}
