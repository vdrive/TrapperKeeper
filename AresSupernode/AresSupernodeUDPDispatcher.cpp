#include "StdAfx.h"
#include "aressupernodeudpdispatcher.h"

AresSupernodeUDPDispatcher::AresSupernodeUDPDispatcher(void)
{
	BOOL stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP Supernode Socket %d\n",error);
	}

}

AresSupernodeUDPDispatcher::~AresSupernodeUDPDispatcher(void)
{
}

void AresSupernodeUDPDispatcher::AddPacket(AresSNUDPHost *host, Buffer2000* packet)
{
	mv_packets_to_send.Add(new UDPDispatchJob(packet,host->GetIP(),host->GetPort()));
}

UINT AresSupernodeUDPDispatcher::Run(void)
{
	while(!this->b_killThread){
		Sleep(15);
		UINT count=0;
		while(mv_packets_to_send.Size()>0){
			count++;
			UDPDispatchJob *job=(UDPDispatchJob*)mv_packets_to_send.Get(0);
			Buffer2000* packet=job->mp_packet;

			int sent=m_send_socket.SendTo(packet->GetMutableBufferPtr(),packet->GetLength(),job->m_port,job->m_host_ip.c_str());
			if(sent==SOCKET_ERROR){
				int error=GetLastError();
				TRACE("ED2KAntiLeakServer::RespondPing() Error sending init to %s, error was %d.\n",job->m_host_ip.c_str(),error);
			}

			mv_packets_to_send.Remove(0);
		//	if(mv_packets_to_send.Size()<500 && (count%5==0))  //if too many get in our queue, go as fast as possible, otherwise take a small break in between sends
		//		Sleep(3);
		}
	}
	return 0;
}
