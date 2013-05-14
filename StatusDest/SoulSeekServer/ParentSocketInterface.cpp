#include "StdAfx.h"
#include "parentsocketinterface.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionSockets.h"
#include "SoulSeekServerDll.h"
#include "PacketIO.h"
#include "SearchRequest.h"

ParentSocketInterface::ParentSocketInterface(void)
{
	p_parent=NULL;
	m_bGotHeader = false;
	ip = NULL;
}

ParentSocketInterface::~ParentSocketInterface(void)
{
	if(ip != NULL)
	{
        delete[] ip;
		ip = NULL;
	}
}

//
//
//
void ParentSocketInterface::InitParent(ConnectionSockets *parent)
{
	p_parent=parent;
}

int ParentSocketInterface::Attach(SOCKET socket, char* theIP )
{
	connected = false;
	ip = theIP;
	return __super::Attach(socket);
}

//
//
//
void ParentSocketInterface::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}

	// Get IP of client the socket is talking to
	/*sockaddr_in sinRemote;
	int nAddrSize = sizeof( sinRemote );
	ZeroMemory( &sinRemote, nAddrSize );
	getpeername( this->m_hSocket, (sockaddr *)&sinRemote, &nAddrSize );
	sinRemote.sin_addr.S_un.S_addr;

	char aIP[16];
	strcpy( aIP, inet_itoa( sinRemote.sin_addr ) );*/

	// You are connected and can call SendSocketData() now or wait for an OnReceive()
}

//
//
//
bool ParentSocketInterface::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return false;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return false;
	}

	// ---

	// Your ReceiveSocketData() or ReceiveSomeSocketData() code goes here

	// For this example, we will attempt to receive the string "HELLO" without the NULL
	//p_parent->m_dlg.WriteToLog("ReceiveSocketData");
	ReceiveSocketData(4);

	// ---

	// Check for more data
	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			return true;	// continute to call OnReceive()
		}
	}

	return false;
}

//
//
//
void ParentSocketInterface::OnClose(int error_code)
{
	// Do any additional cleanup here
	Close();
	__super::OnClose(error_code);	// calls Close()
}

//
//
//
int ParentSocketInterface::Close()
{
	//INCREMENT COUNTER
	::PostMessage( p_parent->m_hwnd, WM_CMOD_CLOSE_CONNECTION, NULL, NULL );
	return 0;
}

//
//
//
void ParentSocketInterface::SocketDataReceived(char *data,unsigned int len)
{
	if( !m_bGotHeader )
	{
		if(*(unsigned int *)(data) != 0)
		{
			m_bGotHeader = true;
			ReceiveSocketData( *(unsigned int *)(data) );
		}
	}
	else
	{
		m_bGotHeader = false;
		// process message
		PacketReader pr(data);
		//int location;
		BYTE cmd;
		cmd = pr.GetNextByte();
		//char *ip;

        //PacketWriter *packet;
		char *pMsg;
		switch(cmd)
		{
			case 0:  //Firewall
				{
					if(connected == true)
					{
						//Once we connect this is used for ping
						//pMsg = new char[512];
						//bool thebool = pr.GetNextByte();
						//sprintf( pMsg, "Ping %s",ip);
						//::PostMessage( p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pMsg, NULL );
						//Close();
						return;
					}
					connected = true;
					unsigned int token = pr.GetNextInt();
					::PostMessage(p_parent->m_hwnd, WM_CMOD_RECEIVED_CONNECTION, (WPARAM)token, NULL );
					pMsg = new char[512];
					sprintf( pMsg, "Recieved CONNECTION request from parent %s using token %u",ip,token );
					::PostMessage( p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pMsg, NULL );
				}
				break;

			case 3:  //Search
				{
					//pMsg = new char[128];
					//sprintf( pMsg, "Recieved SEARCH request from parent" );
					//::PostMessage( p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)pMsg, NULL );

					pr.GetNextInt();
					char* username = pr.GetNextString(); //Username
					int token = pr.GetNextInt(); //Token
					char* searchText = pr.GetNextString(); //Search Text

					SearchRequest* sr;
					sr = new SearchRequest(username,searchText,token);
					::PostMessage( p_parent->m_hwnd, WM_CMOD_SEARCH_REQUEST, (WPARAM)sr, NULL );
				}
				break;
		}
	}
}

//
//
//
void ParentSocketInterface::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	// Some data has been received

	// If you have not received enough data (HTTP header example)
	/*if(strstr(data,"\r\n\r\n")==NULL)
	{
		ReceiveSomeMoreSocketData(data,data_len,new_len,max_len);
		return;
	}*/
	
	// You have received enough data
}
