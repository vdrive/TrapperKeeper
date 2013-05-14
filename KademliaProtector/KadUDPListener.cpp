#include "StdAfx.h"
#include "kadudplistener.h"
#include "..\tkcom\Buffer2000.h"
#include "KademliaProtectorSystem.h"
#include "zlib.h"


KadUDPListener::KadUDPListener(void)
{
	AfxSocketInit();
	BOOL stat=m_rcv_socket.Create(4674,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("FAILED:  Couldn't create KadUDPListener Socket, error = %d\n",error);
	}
	//stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	//if(!stat){
	//	int error=GetLastError();
	//	TRACE("Couldn't create UDP ED2KServerStatusPingManager Socket %d\n",error);
	//}
}

KadUDPListener::~KadUDPListener(void)
{
}

UINT KadUDPListener::Run(void)
{
	TRACE("KadUDPListener::Run(void) THREAD STARTED\n");
	srand(GetTickCount());
	byte buf[1024];
	byte buf2[1024*40];
	while(!this->b_killThread){
		
		CString addr;
		UINT port=0;
		int nread=m_rcv_socket.ReceiveFrom(buf,1023,addr,port);  //TY DEBUG
		if(nread==SOCKET_ERROR){
			int error=GetLastError();
			Sleep(10);
			continue;
		}
		if(nread<2 || nread>1000){
			Sleep(5);
			continue;  //someone sent us some bad information
		}

		byte header=buf[0];
		byte op_code=buf[1];

		//TRACE("Read %d bytes from %s\n",nread,addr);

		if(header!=0xe4 && header!=0xe5){
			continue;
		}

		byte *p_data=buf;


		//decompress the packet if needed
		if(header==0xe5 && op_code==0x08 && nread>4){
			//data needs to be decompressed
			DWORD nNewSize = 1024*40-1;
			DWORD unpackedsize = nNewSize-2;
			
			//uncompress ((Bytef*)uncompressed_data, (uLongf *)&uncompress_buf_length, (const Bytef *)(buffer+16), (uLong)total_length-16);

			DWORD result = uncompress(buf2+2, &unpackedsize, buf+2, nread-2);
			if (result == Z_OK)
			{
				p_data=buf2;
				buf2[0] = 0xe4;
				buf2[1] = buf[1];	
				nread=unpackedsize+2;
			}
			else{
				continue;
			}
		}

		if(op_code==0x08){
			this->ProcessBootstrapResponse(p_data+2,nread-2);
			continue;
		}
		if(op_code==0x48){
			//TRACE("Received publish response from %s\n",addr);
			continue;
		}
		if(op_code==0x10){
			//TRACE("Received udp request 0x%X from %s with length %d\n",op_code,addr,nread);
			continue;
		}
		if(op_code==0x20){
			//TRACE("Received udp request 0x%X from %s with length %d\n",op_code,addr,nread);
			continue;
		}
		if(op_code==0x00){
			continue;
		}
		else{
			TRACE("Received udp request 0x%X from %s with length %d\n",op_code,addr,nread);
			continue;
		}

	}

	TRACE("KadUDPListener::Run(void) THREAD TERMINATED\n");
	return 0;
}

void KadUDPListener::StartKadUDPSystem(void)
{
	this->StartThreadLowPriority();
	m_boot_strapper.SetSocket(&m_rcv_socket);
	m_publisher.SetSocket(&m_rcv_socket);
	m_boot_strapper.StartThreadLowPriority();
	m_publisher.StartPublishing();
}

void KadUDPListener::StopKadUDPSystem(void)
{
	m_boot_strapper.StopThread();
	m_publisher.StopPublishing();
	this->StopThread();
}

void KadUDPListener::ProcessBootstrapResponse(byte* p_data, UINT data_length)
{
	Buffer2000 buf(p_data,data_length);

	if(data_length<27){
		ASSERT(0);
		return;
	}

	WORD num_contacts = buf.GetWord(0);

	// Verify packet is expected size
	if (data_length != (2 + 25*num_contacts)){
		ASSERT(0);
		return;
	}

	KademliaProtectorSystemReference ref;
	
	int data_offset=2;
	Vector v_contacts;
	CTime now=CTime::GetCurrentTime();
	for(int i=0;i<(int)num_contacts;i++){
		CUInt128 id;
		id.setValueBE(buf.GetBufferPtr()+data_offset);
		data_offset+=16;
		DWORD ip = ntohl(buf.GetDWord(data_offset));
		data_offset+=4;

		WORD port = buf.GetWord(data_offset);
		data_offset+=2;
		WORD tport = buf.GetWord(data_offset);
		data_offset+=2;
		byte type = buf.GetByte(data_offset++);
		v_contacts.Add(new KadContact(CUInt128(id.getDataPtr()),ip,port,now));
		
	}

	//TYDEBUG
	//byte test_hash1[16];
	//byte test_hash2[16];

	//memset(test_hash1,1,16);
	//memset(test_hash2,2,16);

	//v_contacts.Add(new KadContact(CUInt128(test_hash1),5555555,4672,now));
	//v_contacts.Add(new KadContact(CUInt128(test_hash2),5555556,4672,now));

	ref.System()->GetContactManager()->AddContacts(v_contacts);	
}

KadUDPPublisher* KadUDPListener::GetPublisher(void)
{
	return &m_publisher;
}
