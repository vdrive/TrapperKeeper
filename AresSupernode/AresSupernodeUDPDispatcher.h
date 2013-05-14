#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Buffer2000.h"
#include "AresSNUDPHost.h"

class AresSupernodeUDPDispatcher : public ThreadedObject
{
	class UDPDispatchJob : public Object
	{
	public:
		Buffer2000* mp_packet;
		string m_host_ip;
		unsigned short m_port;

		UDPDispatchJob(Buffer2000* packet, const char* ip, unsigned short port){
			mp_packet=packet;
			m_host_ip=ip;
			m_port=port;
			mp_packet->ref();
		}
		~UDPDispatchJob(){
			mp_packet->deref();  //if its out of references, it will delete itself
			mp_packet=NULL;
		}
	};
	Vector mv_packets_to_send;
	CAsyncSocket m_send_socket;
public:
	AresSupernodeUDPDispatcher(void);
	~AresSupernodeUDPDispatcher(void);
	void AddPacket(AresSNUDPHost *host, Buffer2000* packet);
	UINT Run(void);
};
