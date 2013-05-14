#include "StdAfx.h"
#include "ClientSocketInterface.h"
#include "ClientModuleMessageWnd.h"
#include "ClientSockets.h"
#include "SoulSeekClientDll.h"
#include <time.h>
#include <vector>
using namespace std;

ClientSocketInterface::ClientSocketInterface(void)
{
	p_parent=NULL;
	p_packet = NULL;
	m_closedCalled = false;
}

ClientSocketInterface::~ClientSocketInterface(void)
{
	if(p_packet != NULL)
	{
		Close();
		delete p_packet;
		p_packet = NULL;
	}
}

//
//
//
void ClientSocketInterface::InitParent(ClientSockets *parent)
{
	p_parent=parent;
}

void ClientSocketInterface::SendPacket(PacketWriter* packet)
{
	if(packet == NULL)
	{
		::PostMessage(p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)strdup("Packet is null!"), NULL);
		Close();
		return;
	}
	Close();

	p_packet = packet;
	m_closedCalled = true;

	m_connectTime = CTime::GetCurrentTime();
	::PostMessage( p_parent->m_hwnd, WM_CMOD_CONNECTED, NULL, NULL );
	Create(0);
	Connect(packet->p_ip,packet->m_port);
}

//
//
//
void ClientSocketInterface::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		::PostMessage(p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)strdup("Error connecting to client!"), NULL);
		Close();
		return;
	}

	//Peer Init
	if(p_packet->p_username != NULL)
	{
		PacketWriter *packet;
		packet = new PacketWriter(1+4+(int)strlen(p_packet->p_username)+4+1+4);  // Create a packet with body size 8
		packet->AppendByte(1,false); // wait port cmd code
		packet->AppendSizeAndString(p_packet->p_username,false);
		packet->AppendSizeAndString("P",false); //port to wait on
		packet->AppendInt(0, false);
		SendSocketData( packet->GetPacket(), packet->GetPacketSize() );  //Send packet specifying packet string and size
		delete packet;
	}
	else
	{
		::PostMessage(p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)strdup("Error sending peer init!"), NULL);
		Close(); 
		return;
	}

	if(p_packet != NULL)
	{
		SendSocketData( (unsigned char*) p_packet->GetPacket(), p_packet->GetPacketSize() );
		//delete p_packet;
		//p_packet = NULL;
		//TRACE("Packet Sent to client\n");
		//result = uncompress( (Bytef *)unzipText, (uLongf *)&unzipLen,
		//	(const Bytef *)(p_packet->GetPacket()+8), (uLong)(p_packet->GetPacketSize()) );

							
	}
	else
	{
		::PostMessage(p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)strdup("Error sending packet...p_packet is null!"), NULL);
	}

	Close();  //Close Connection
}

//
//
//
bool ClientSocketInterface::OnReceive(int error_code)
{
	return false;
	if(error_code!=0)
	{
		::PostMessage(p_parent->m_hwnd, WM_CMOD_LOG_MSG, (WPARAM)strdup("Error: onrecieve!"), NULL);
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
void ClientSocketInterface::OnClose(int error_code)
{
	// Do any additional cleanup here
	Close();
}

//
//
//
int ClientSocketInterface::Close()
{
	if(m_closedCalled) ::PostMessage( p_parent->m_hwnd, WM_CMOD_CLOSE_CONNECTION, NULL, NULL );
	
	m_closedCalled = false;

	if(p_packet != NULL)
	{
		delete p_packet;
		p_packet = NULL;
	}
	return __super::Close();;
}

//
//
//
void ClientSocketInterface::SocketDataReceived(char *data,unsigned int len)
{
	Close();
}

//
//
//
void ClientSocketInterface::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
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
