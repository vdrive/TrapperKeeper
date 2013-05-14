#include "StdAfx.h"
#include "udpdispatcher.h"

UDPDispatcher::UDPDispatcher(void)
{
	BOOL stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP Dispatcher Socket %d\n",error);
	}
}

UDPDispatcher::~UDPDispatcher(void)
{
}

void UDPDispatcher::AddPacket(const char* ip,unsigned short port, Buffer2000* packet)
{
	packet->ref();
	if(mv_packets_to_send.Size()<50000)
		mv_packets_to_send.Add(new UDPDispatchJob(packet,ip,port));
	packet->deref();
}

UINT UDPDispatcher::Run(void)
{
	while(!this->b_killThread){
		Sleep(35);
		UINT count=0;
		while(mv_packets_to_send.Size()>0 && !this->b_killThread){
			if(count++>25 && mv_packets_to_send.Size()<2000)
				break; //take a small break and flush some of this data
			UDPDispatchJob *job=(UDPDispatchJob*)mv_packets_to_send.Get(0);
			Buffer2000* packet=job->mp_packet;

			int sent=m_send_socket.SendTo(packet->GetMutableBufferPtr(),packet->GetLength(),job->m_port,job->m_host_ip.c_str());
			if(sent==SOCKET_ERROR){
				int error=GetLastError();
				TRACE("UDPDispatcher::Run() Error sending packet to %s, error was %d.\n",job->m_host_ip.c_str(),error);
				//buffer is probably, full lets stop trying to send things briefly
				break;
			}

			mv_packets_to_send.Remove(0);
		//	if(mv_packets_to_send.Size()<500 && (count%5==0))  //if too many get in our queue, go as fast as possible, otherwise take a small break in between sends
		//		Sleep(3);
		}
	}
	return 0;
}

void UDPDispatcher::StartSystem(void)
{
	this->StartThreadLowPriority();
}

void UDPDispatcher::StopSystem(void)
{

	this->StopThread();
	m_send_socket.Close();
}
