#include "StdAfx.h"
#include "communicationsystem.h"
#include "..\..\RackRecoverySystem\RackRecoverInterface.h"
#include "ReceivedMessage.h"

CommunicationSystem::CommunicationSystem(void)
{
	m_rcv_socket.Create(RACKMONITORPORT,SOCK_DGRAM,0,NULL);
	m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
}

CommunicationSystem::~CommunicationSystem(void)
{
}

UINT CommunicationSystem::Run()
{
	while(!this->b_killThread){
		byte buf[512];
		CString addr;
		UINT port;
		int nread=m_rcv_socket.ReceiveFrom(buf,512,addr,port);
		if(nread<8){
			Sleep(50);
			continue;
		}

		if(mv_received_messages.Size()>20000){
			continue;
		}
		else{
			mv_received_messages.Add(new ReceivedMessage(addr,buf,nread));
		}

		Sleep(1);
	}
	return 0;
}

void CommunicationSystem::Shutdown(void)
{
	m_rcv_socket.Close();
	m_send_socket.Close();
	this->StopThread();
}

void CommunicationSystem::Send(const char* dest, UINT header)
{
	byte buf[4];
	*(UINT*)buf=header;
	int sent=m_send_socket.SendTo(buf,4,RACKRECOVERYPORT,dest);
	if(sent==SOCKET_ERROR){
		int error=GetLastError();
		TRACE("CommunicationSystem::Send() Error sending header %u to address %s, error was %d.\n",header,dest,error);
	}
}

ReceivedMessage* CommunicationSystem::GetNextReceivedMessage(void)
{
	if(mv_received_messages.Size()>0)
		return (ReceivedMessage*)mv_received_messages.Get(0);
	else
		return NULL;
}

void CommunicationSystem::PopMessages(void)
{
	if(mv_received_messages.Size()>0)
		mv_received_messages.Remove(0);
}

int CommunicationSystem::HasReceived(void)
{
	return mv_received_messages.Size();
}

void CommunicationSystem::SendPing(const char* dest_ip)
{
	//send 3 pings to insure that our message gets there
	Send(dest_ip,RACKPING);
}

void CommunicationSystem::SendDllRequest(const char* dest_ip)
{
	Send(dest_ip,RACKDLLS);
}

void CommunicationSystem::SendProcessRequest(const char* dest_ip)
{
	Send(dest_ip,RACKPROCESSES);
}
