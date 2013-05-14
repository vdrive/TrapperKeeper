#include "StdAfx.h"
#include "kadudpbootstrapper.h"
#include "KademliaProtectorSystem.h"
#include "..\tkcom\buffer2000.h"

KadUDPBootstrapper::KadUDPBootstrapper(void)
{
	mp_socket=NULL;
}

KadUDPBootstrapper::~KadUDPBootstrapper(void)
{
}

UINT KadUDPBootstrapper::Run(void)
{
	TRACE("KadUDPBootstrapper::Run(void) THREAD STARTED\n");
	KademliaProtectorSystemReference ref;
	UINT my_ip;
	int ip1,ip2,ip3,ip4;
	const char* sip=ref.System()->GetMyIP();
	sscanf(sip,"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
	my_ip=(ip4<<24)|(ip3<<16)|(ip2<<8)|(ip1);

	byte *my_hash=ref.System()->GetMyHash();

	Timer m_startup;

	while(!this->b_killThread){
		if(m_startup.HasTimedOut(10*60))
			Sleep(30);
		else if(m_startup.HasTimedOut(25*60))
			Sleep(700);  //TYDEBUG - slow down to this rate in the long run
		else
			Sleep(80);  //briefly run faster right after startup to learn about network quicker

		Vector v_tmp;
		ref.System()->GetContactManager()->GetNextBootStrapContact(v_tmp);
		if(v_tmp.Size()==0)
			continue;
		KadContact* kc=(KadContact*)v_tmp.Get(0);
		CTime now=CTime::GetCurrentTime();
		if(kc->m_last_bootstrap_request>=(now-CTimeSpan(0,0,5,0)))
			continue;  //we have made this request to recently, lets skip it for now

		kc->m_last_bootstrap_request=now;
		
		//TRACE("KadUDPBootstrapper::Run(void) bootstrapping %s\n",kc->m_sip.c_str());
		BootStrap(kc,my_ip,my_hash);
	}

	TRACE("KadUDPBootstrapper::Run(void) THREAD TERMINATED\n");
	return 0;
}

void KadUDPBootstrapper::SetSocket(CAsyncSocket* p_socket)
{
	mp_socket=p_socket;
}

void KadUDPBootstrapper::BootStrap(KadContact* p_contact,UINT my_ip,byte *my_hash)
{
	//TRACE("KadUDPBootstrapper::BootStrap() Sending bootstrap to %s:%d\n",p_contact->m_sip.c_str(),p_contact->m_port);
	Buffer2000 boot_strap_packet;
	boot_strap_packet.WriteByte(0xE4);
	boot_strap_packet.WriteByte(0x00);
	boot_strap_packet.WriteBytes(my_hash,16);
	boot_strap_packet.WriteDWord(my_ip);
	boot_strap_packet.WriteWord(4672);
	boot_strap_packet.WriteWord(4662);
	boot_strap_packet.WriteByte(10);
	mp_socket->SendTo(boot_strap_packet.GetBufferPtr(),boot_strap_packet.GetLength(),p_contact->m_port,p_contact->m_sip.c_str());  //ask them to tell us about some clients they happen to know about
}
