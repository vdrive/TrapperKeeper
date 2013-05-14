#include "StdAfx.h"
#include "KadFTConnection.h"
#include <mmsystem.h>
#include "..\MetaMachineSpoofer\DonkeyFile.h"
#include "..\tkcom\PeerSocket.h"
#include "KademliaProtectorSystem.h"
//#include "Ws2tcpip.h"

#define	PARTSIZE				9728000 //hash part sizes
#define BLOCKSIZE				184320  //request block sizes

//emule specific op codes
#define	OP_EMULEINFO			0x01	//
#define	OP_EMULEINFOANSWER		0x02	//
#define OP_COMPRESSEDPART		0x40	//
#define OP_QUEUERANKING			0x60	// <RANG 2>
#define OP_FILEDESC				0x61	// <len 2><NAME len>
#define OP_REQUESTSOURCES		0x81	// <HASH 16>
#define OP_ANSWERSOURCES		0x82	//
#define OP_PUBLICKEY			0x85	// <len 1><pubkey len>
#define OP_SIGNATURE			0x86	// v1: <len 1><signature len>  v2:<len 1><signature len><sigIPused 1>
#define OP_SECIDENTSTATE		0x87	// <state 1><rndchallenge 4>
#define OP_REQUESTPREVIEW		0x90	// <HASH 16>
#define OP_PREVIEWANSWER		0x91	// <HASH 16><frames 1>{frames * <len 4><frame len>}
#define OP_MULTIPACKET			0x92
#define OP_MULTIPACKETANSWER	0x93

// client <-> client
#define	OP_HELLO				0x01	// 0x10<HASH 16><ID 4><PORT 2><1 Tag_set>
#define OP_SENDINGPART			0x46	// <HASH 16><von 4><bis 4><Daten len:(von-bis)>
#define	OP_REQUESTPARTS			0x47	// <HASH 16><von[3] 4*3><bis[3] 4*3>
#define OP_FILEREQANSNOFIL		0x48	// <HASH 16>
#define OP_END_OF_DOWNLOAD     	0x49    // <HASH 16>
#define OP_ASKSHAREDFILES		0x4A	// (null)
#define OP_ASKSHAREDFILESANSWER 0x4B	// <count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
#define	OP_HELLOANSWER			0x4C	// <HASH 16><ID 4><PORT 2><1 Tag_set><SERVER_IP 4><SERVER_PORT 2>
#define OP_CHANGE_CLIENT_ID 	0x4D	// <ID_old 4><ID_new 4>
#define	OP_MESSAGE				0x4E	// <len 2><Message len>
#define OP_SETREQFILEID			0x4F	// <HASH 16>
#define	OP_FILESTATUS			0x50	// <HASH 16><count 2><status(bit array) len:((count+7)/8)>
#define OP_HASHSETREQUEST		0x51	// <HASH 16>
#define OP_HASHSETANSWER		0x52	// <count 2><HASH[count] 16*count>
#define	OP_STARTUPLOADREQ		0x54	// <HASH 16>
#define	OP_ACCEPTUPLOADREQ		0x55	// (null)
#define	OP_CANCELTRANSFER		0x56	// (null)	
#define OP_OUTOFPARTREQS		0x57	// (null)
#define OP_REQUESTFILENAME		0x58	// <HASH 16>	(more correctly file_name_request)
#define OP_REQFILENAMEANSWER	0x59	// <HASH 16><len 4><NAME len>
#define OP_CHANGE_SLOT			0x5B	// <HASH 16>
#define OP_QUEUERANK			0x5C	// <wert  4> (slot index of the request)
#define OP_ASKSHAREDDIRS        0x5D    // (null)
#define OP_ASKSHAREDFILESDIR    0x5E    // <len 2><Directory len>
#define OP_ASKSHAREDDIRSANS     0x5F    // <count 4>(<len 2><Directory len>)[count]
#define OP_ASKSHAREDFILESDIRANS 0x60    // <len 2><Directory len><count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
#define OP_ASKSHAREDDENIEDANS   0x61    // (null)

byte g_buffer[BLOCKSIZE];
byte g_garbage_buffer[BLOCKSIZE];
Buffer2000 g_compressed_garbage;
bool gb_garbage_initialized=false;

KadFTConnection::KadFTConnection(CAsyncSocket* socket,unsigned short listening_port, byte *my_hash,string my_user_name)
{
	m_emule_version=44160;

	m_ip=((PeerSocket*)socket)->GetIP();

	m_my_ip=((PeerSocket*)socket)->GetBindIP();
	m_listening_port=listening_port;
	mp_tcp_socket=(PeerSocket*)socket;
	mp_tcp_socket->SetSocketEventListener(this);
	
	m_port=mp_tcp_socket->GetPort();

	mb_overnet=false;

	mb_abort=false;
	memcpy(m_my_hash,my_hash,16);
	m_my_user_name=my_user_name;
	m_send_queue_index=0;
	m_send_delay=0;
	m_client_port=0;

	mb_needs_send=false;
	m_feed_pos=0;

	if(!gb_garbage_initialized){
		gb_garbage_initialized=true;
		for(int i=0;i<BLOCKSIZE;i++)
			g_garbage_buffer[i]=rand()&1;
		CompressData(g_garbage_buffer,BLOCKSIZE,g_compressed_garbage);  //precompress the garbage buffer to save time later.
	}

	mb_can_send=true;
	mb_feed=false;

	if(rand()&1)
		mb_compressed_poisoner=true;
	else
		mb_compressed_poisoner=false;

	mb_no_swarms_allowed=false;

	mb_emule_compatible=false;

	mb_eserver=false;

	mb_overnet_special_status=false;
}

KadFTConnection::~KadFTConnection(void)
{
	TRACE("MetaMachineSpoofer KadFTConnection::~KadFTConnection().\n");
	//Close the connection that this object represents
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("KadFTConnection::~KadFTConnection() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);


	//CleanSendQueue();

	if(mp_tcp_socket!=NULL){
		delete mp_tcp_socket;
		mp_tcp_socket=NULL;
	}

	//log_msg.Format("KadFTConnection::~KadFTConnection() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

void KadFTConnection::CheckBuffer(void)
{
	if(m_buffer.GetLength()<5 || mb_abort)
		return;

	const byte* p_data=m_buffer.GetBufferPtr();
	if(p_data[0]!=0xe3 && p_data[0]!=0xc5){
		m_buffer.Clear();  //discard this data since we don't know what to do with it
		TRACE("KadFTConnection::CheckBuffer(void) Received invalid packet from %s with first byte = 0x%x.\n",m_ip.c_str(),p_data[0]);
		mb_abort=true;
		return;
	}

	DWORD len=m_buffer.GetDWord(1);  //get the length of the packet

	if(len>200000){  //if someone is trying to crash us with a large length drop the connection immediately
		mb_abort=true;
		m_buffer.Clear();
		return;
	}

	if(len>m_buffer.GetLength()-5){  //have we received enough of the data yet?
		return;
	}

	byte protocol=*p_data;
	byte cmd=*(p_data+5);

	TRACE("KadFTConnection::CheckBuffer() %s BEGIN message type protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);

	CString log_msg;
	log_msg.Format("KadFTConnection::CheckBuffer() BEGIN %s message type protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);

	KademliaProtectorSystemReference ref;
	ref.System()->LogToFile(log_msg);

	//len does not include the first 5 bytes of the packet

	//ok here we have a valid packet of data
	if(len<1){  //no packets should be this short lets abort
		mb_abort=true;
		m_buffer.Clear();

		TRACE("KadFTConnection::CheckBuffer() %s END [0 - ABNORMAL] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 0");
		return;
	}

	bool b_emule_packet=false;
	if(p_data[0]==0xc5){
		b_emule_packet=true;
		TRACE("KadFTConnection::CheckBuffer(void) Received emule specific packet from %s with command byte byte = 0x%x.\n",m_ip.c_str(),p_data[5]);
	}
	else{
		TRACE("KadFTConnection::CheckBuffer(void) Received ed2k packet from %s with command byte byte = 0x%x.\n",m_ip.c_str(),p_data[5]);
	}
	
	//TRACE("Client Connection %s: Received command byte 0x%x with len=%d\n\n",mp_tcp_socket->GetIP(),p_data[5],len);
	if(!b_emule_packet && len>=7 && p_data[5]==0x01 && p_data[6]==0x10){
		//TRACE("Client Connection %s: Got Hello Packet\n",mp_tcp_socket->GetIP());
		byte *the_data=(byte*)p_data+5;

		//extract the clients port so we can report them as a source later on
		const UINT port_nemonic=0x0f000103;
		const UINT version_nemonic=0x11000103;
		for(int j=0;j<(int)(len-8);j++){
			if(*((UINT*)(++the_data))==port_nemonic){
				the_data+=4;
				DWORD port=*((DWORD*)the_data);
				m_client_port=(unsigned short)port;
				break;
			}
		}

		the_data=(byte*)p_data+5;
		//check to see if this server is emule compatible
		for(int j=0;j<(int)(len-8);j++){
			if(*((UINT*)(++the_data))==version_nemonic){
				the_data+=4;
				DWORD version=*((DWORD*)the_data);
				if(version==60){
					mb_emule_compatible=true;
					TRACE("MetaMachineSpoofer::KadFTConnection::CheckBuffer() Setting client %s as emule compatible.\n",m_ip.c_str());
					break;
				}
				else if(version==1000){
					mb_overnet=true;
				}
			}
		}
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondHello();
		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [1] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 1");
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x4f){ //file status request
		//TRACE("Client Connection %s: Got File Status Request\n",mp_tcp_socket->GetIP());
		byte hash[16];
		memcpy(hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		//m_state=CLIENT_FILE_STATUS;
		RespondFileStatus(hash);
		m_idle.Refresh();

		TRACE("KadFTConnection::CheckBuffer() %s END [2] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 2");
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x58){ //file status request
		byte hash[16];
		memcpy(hash,p_data+6,16);
		Vector v_tmp;

		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondFileRequest(hash);

		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [3] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 3");
		return;
	}
	
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x51 ){ //file hash parts request
		//TRACE("Client Connection %s: Got File Request\n",mp_tcp_socket->GetIP());
		byte hash[16];
		memcpy(hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondHashParts(hash);

		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [4] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 4");
		return;
	}
	
	if(!b_emule_packet && len>=1 && p_data[5]==0x54){ //slot request
		//TRACE("Client Connection %s: Got Slot Request\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		GrantSlot();
		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [5] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 5");
		return;
	}
	if(!b_emule_packet && len>=1 && p_data[5]==0x56){ //slot request
		//TRACE("Client Connection %s: Slot Released\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [6] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 6");
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x6D){ //overnet message
		if(mb_overnet)
			mb_overnet_special_status=true;
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 7");
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x65){ //overnet message
		//TRACE("Client Connection %s: Slot Released\n",mp_tcp_socket->GetIP());

		byte tmp[16];
		memcpy(tmp,p_data+6,16);
		RespondOvernet0x66(tmp);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s Respond overnet 0x65\n",m_ip.c_str());
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 8");
		return;
	}

	if(!b_emule_packet && len>=1+16+24 && p_data[5]==0x47 ){ //requested parts
		byte hash[16];
		memcpy(hash,p_data+6,16);
		m_start1=*((UINT*)(p_data+6+16));
		m_start2=*((UINT*)(p_data+6+16+4));
		m_start3=*((UINT*)(p_data+6+16+8));
		m_end1=*((UINT*)(p_data+6+16+12));
		m_end2=*((UINT*)(p_data+6+16+16));
		m_end3=*((UINT*)(p_data+6+16+20));
		TRACE("Client Connection %s: Got Send Parts %d-%d,%d-%d,%d-%d\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondSendParts(hash);
		//m_state=CLIENT_SEND_PARTS;
		m_idle.Refresh();
		TRACE("KadFTConnection::CheckBuffer() %s END [7] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
		ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 9");
		return;
	}

	ref.System()->LogToFile("KadFTConnection::CheckBuffer() Position A");
	if(b_emule_packet){
		Buffer2000 emule_packet(m_buffer.GetBufferPtr(),len+5);//m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer
		ProcessEmulePacket(emule_packet);
	}

	//TRACE("Client Connection %s: Got Unknown Command 0x%x\n",mp_tcp_socket->GetIP(),p_data[5]);
	m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	


	TRACE("KadFTConnection::CheckBuffer() %s END [8] protocol=0x%02X, type=0x%02X\n",m_ip.c_str(),protocol,cmd);
	ref.System()->LogToFile("KadFTConnection::CheckBuffer() END 10");
}

void KadFTConnection::Update(void)
{
	CSingleLock lock(&m_lock,TRUE);
	if(mb_feed)
		Feed();
	
	CheckBuffer();

	CheckSendBuffer();
}

void KadFTConnection::RespondHello(void)
{
	if(!mp_tcp_socket){
		mb_abort=true;
		return;
	}

	UINT my_id=16777217+(int)rand();
	if(m_my_ip.size()>0){
		int ip1=0,ip2=0,ip3=0,ip4=0;
		sscanf(m_my_ip.c_str(),"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
		my_id=(ip4<<24) | (ip3<<16) | (ip2<<8) | (ip1);
	}

	srand(GetTickCount());

	//MetaSpooferReference sysref;
	const char* my_name=m_my_user_name.c_str();	

	UINT my_name_length=(UINT)strlen(my_name);

	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;

	//DISPATCH OUR EMULE SPECIFIC PACKET IF APPLICABLE
	if(mb_emule_compatible){
		Buffer2000 emule_packet;
		emule_packet.WriteByte(0xc5);	//emule protocol
		emule_packet.WriteDWord(0);		//length
		emule_packet.WriteByte(0x01);	//hello
		emule_packet.WriteWord(323);	//version - changes with each version of emule
		emule_packet.WriteDWord(7);		//number of metatags

		//metatag - compression
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x20);	//compression
		emule_packet.WriteDWord(1);		//yes we support compression

		//metatag - UDP Version
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x22);	//udp version
		emule_packet.WriteDWord(4);		//version 4

		//metatag - UDP ClientPort
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x21);	//udp client port
		emule_packet.WriteDWord(4672);	//we will be listening on 4672

		//metatag - Source Exchange
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x23);	//Source Exchange
		emule_packet.WriteDWord(3);		//version of source exchange

		//metatag - Comments
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x24);	//Comments
		emule_packet.WriteDWord(1);		//Version of comments?

		//metatag - Extended Request
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x25);	//Extended Request
		emule_packet.WriteDWord(2);		//Version?

		//metatag - Features
		emule_packet.WriteByte(0x03);	//type
		emule_packet.WriteWord(0x01);	//size
		emule_packet.WriteByte(0x27);	//Features
		emule_packet.WriteDWord(3);		//version 3 of features

		UINT emule_packet_length=emule_packet.GetLength();
		emule_packet.WriteDWord(1,emule_packet_length-5);  //fix the length

		SendData(emule_packet.GetBufferPtr(),emule_packet.GetLength());
		//m_send_buffer.WriteBytes(emule_packet.GetBufferPtr(),emule_packet.GetLength());
	}

	Buffer2000 packet;	
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x4c);  //Set the command byte equal to a HELLO_RESPONSE 0x4c

	if(mb_overnet){
		byte tmp[16];

		for(int i=0;i<16;i++)
			tmp[i]=i*7+10;

		for(int i=0;i<(int)m_my_ip.size() && i<16;i++){
			tmp[i]=(int)m_my_ip[i];
		}

		packet.WriteBytes(tmp,16);
	}
	else{
		packet.WriteBytes(m_my_hash,16);
	}

	if(mb_eserver){  //does this client look like an edonkey server?
		packet.WriteDWord(0);  //we had better have a low id to this server
	}
	else{
		
		packet.WriteDWord(my_id);  //don't want anyone else thinking we have a lowid
	}
	packet.WriteWord(m_listening_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	if(mb_overnet){
		packet.WriteDWord(3); //overnet only has 3 metatags
	}
	else{
		packet.WriteDWord(5); //number of metatags metatags
	}

	//metatag - name
	packet.WriteByte(0x02);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x01);	
	packet.WriteWord(my_name_length);		//my name length
	packet.WriteString(my_name);		//my name


	if(mb_overnet){
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x01);	//size
		packet.WriteByte(0x11);	//version
		packet.WriteDWord(1000);//look we are overnet too

		//add stupid meaningless overnet packet
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x02);	//size
		packet.WriteByte('p');	//version
		packet.WriteByte('r');	//version
		packet.WriteDWord(1);//
	}
	else{

		//metatag - version
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x01);	//size
		packet.WriteByte(0x11);	//version
		packet.WriteDWord(60);		//my name length

		//metatag - 0xf9 (i think its udp port)
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x01);	//size
		packet.WriteByte(0xf9);	//udp port
		packet.WriteDWord(4672);		//my name length

		//metatag - ET_MISC_OPTIONS
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x01);	//size
		packet.WriteByte(0xfa);	//misc options
		packet.WriteDWord(68366878);		//my name length

		//metatag - emule version
		packet.WriteByte(0x03);	//type
		packet.WriteWord(0x01);	//size
		packet.WriteByte(0xfb);	//emule version

		packet.WriteDWord(m_emule_version);		//my version
	}


	if((rand()%100)<5 && !mb_eserver && !mb_overnet){  //5% chance we will let them know about our server
		packet.WriteByte(38);
		packet.WriteByte(118);
		packet.WriteByte(163);
		packet.WriteByte(138);
		packet.WriteWord(4661);
	}
	else{
		packet.WriteByte(0);
		packet.WriteByte(0);
		packet.WriteByte(0);
		packet.WriteByte(0);
		packet.WriteWord(0);
	}
	

	if(mb_overnet){
		packet.WriteByte(0x8a);
		packet.WriteByte(0x9a);
		packet.WriteByte(0xba);
		packet.WriteByte(0x1a);
	}

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	SendData(packet.GetBufferPtr(),packet.GetLength());
}

void KadFTConnection::RespondHashParts(byte *hash)
{
	if(!mp_tcp_socket){
		mb_abort=true;
		return;
	}
	DonkeyFile *the_file=NULL;

	KademliaProtectorSystemReference ref;

	Vector v_tmp;
	ref.System()->GetSwarmFile(v_tmp,hash);	

	if(v_tmp.Size()==0)
		ref.System()->GetDecoyFile(v_tmp,hash);

	if(v_tmp.Size()==0){
		RespondDontHaveFile(hash);
		return;
	}

	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x52);
	packet.WriteBytes(the_file->m_hash,16);  //write in hash
	the_file->WriteHashSet(packet);  //write in hash set

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//Lets send our hash parts for this file to this pirate
	SendData(packet.GetBufferPtr(),packet.GetLength());
}

/*
bool KadFTConnection::IsConnection(TKTCPConnection &con)
{
	if(m_tcp_connection==con){
		return true;
	}
	else{
		return false;
	}
}*/

void KadFTConnection::Shutdown(void)
{
	CSingleLock lock(&m_lock,TRUE);
	TRACE("KadFTConnection::Shutdown() %s STARTED.\n",m_ip.c_str());

	//Close the connection that this object represents
	mb_abort=true;
	if(mp_tcp_socket){
		//do this so the socket can be closed from a different thread without throwing an exception in CSocket
		if(mp_tcp_socket->m_hSocket!=INVALID_SOCKET){
			SOCKET s=mp_tcp_socket->Detach();
			closesocket(s);
		}

		delete mp_tcp_socket;
	}
	mp_tcp_socket=NULL;

	TRACE("KadFTConnection::Shutdown() %s FINISHED.\n",m_ip.c_str());
}

bool KadFTConnection::IsOld(void)
{
	
	if(mb_abort || m_created.HasTimedOut(3*60*60) || m_idle.HasTimedOut(15*60)){  //a connection will have a lifetime of at most 2 hours, or 15 minutes of idle time.
		return true;	
	}
	else{
		return false;
	}
}


void KadFTConnection::RespondFileStatus(byte *hash)
{
	if(!mp_tcp_socket){
		mb_abort=true;
		return;
	}
	DonkeyFile *the_file=NULL;

	KademliaProtectorSystemReference sysref;
	Vector v_tmp;

	bool b_false_decoy=false;
	sysref.System()->GetSwarmFile(v_tmp,hash);	

	if(v_tmp.Size()==0)
		sysref.System()->GetDecoyFile(v_tmp,hash);

	if(v_tmp.Size()==0){
		RespondDontHaveFile(hash);
		return;
	}

	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x50);
	packet.WriteBytes(the_file->m_hash,16);

	if(!mb_overnet_special_status)
		the_file->WriteFileStatus(packet,b_false_decoy,false);
	else
		the_file->WriteFileStatus(packet,b_false_decoy,true);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	SendData(packet.GetBufferPtr(),packet.GetLength());
}

void KadFTConnection::RespondFileRequest(byte *hash)
{
	if(!mp_tcp_socket){
		mb_abort=true;

		return;
	}
	DonkeyFile *the_file=NULL;

	KademliaProtectorSystemReference ref;

	Vector v_tmp;

	ref.System()->GetSwarmFile(v_tmp,hash);	

	if(v_tmp.Size()==0)
		ref.System()->GetDecoyFile(v_tmp,hash);

	if(v_tmp.Size()==0){
		RespondDontHaveFile(hash);
		mb_abort=true;

		return;
	}

	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x59);
	packet.WriteBytes(the_file->m_hash,16);  //write in hash

	CString file_name=the_file->m_file_name.c_str();

	packet.WriteWord((WORD)file_name.GetLength());  //write in name length
	packet.WriteBytes((byte*)(LPCSTR)file_name,(UINT)file_name.GetLength());  //write in name

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	SendData(packet.GetBufferPtr(),packet.GetLength());
}




void KadFTConnection::RespondSendParts(byte *hash)
{
	KademliaProtectorSystemReference ref;


	if(!mp_tcp_socket){
		mb_abort=true;
		return;
	}

	TRACE("KadFTConnection::RespondSendParts(byte *hash) %s BEGIN\n",m_ip.c_str());

	DonkeyFile *the_file=NULL;

	Vector v_tmp;
	bool b_is_real_decoy=false;
	bool b_is_false_decoy=false;
	bool b_is_swarm=false;
	b_is_swarm=ref.System()->GetSwarmFile(v_tmp,hash);

	if(v_tmp.Size()==0)
		b_is_false_decoy=ref.System()->GetDecoyFile(v_tmp,hash);

	if(v_tmp.Size()==0){
		TRACE("MetaMachineSpoofer KadFTConnection %s requesting we send parts for a hash we don't have.\n",m_ip.c_str());
		mb_abort=true;
		return;
	}

	//we have a file now
	the_file=(DonkeyFile*)v_tmp.Get(0);

	TRACE("KadFTConnection::RespondSendParts() %s Requesting file %s, length=%u\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);

	HANDLE file_handle = NULL;

	if(b_is_real_decoy){
		TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u is real decoy\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);

		m_send_delay=1;  //send it as fast as possible

		//open the file
		CString tmp_name;
		tmp_name.Format("%s\\%s",the_file->m_file_path.c_str(),the_file->m_file_name.c_str());
		
		file_handle=CreateFile(tmp_name, // open file at local_path 
					GENERIC_READ,              // open for reading 
					FILE_SHARE_READ,           // share for reading 
					NULL,                      // no security 
					OPEN_EXISTING,             // existing file only 
					FILE_ATTRIBUTE_NORMAL,     // normal file 
					NULL);                     // no attr. template 

		if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
			TRACE("Client Connection %s: Sending Parts [A] %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",mp_tcp_socket->GetIP(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}

		DWORD fsize=0;
		DWORD hsize=0;
		fsize=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
		
		if(fsize<1){
			mb_abort=true;
			CloseHandle(file_handle);
			TRACE("KadFTConnection::RespondSendParts() %s END File %s, length=%u Couldn't open real file\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
			return;
		}

		m_end1=min(fsize,m_end1);
		m_end2=min(fsize,m_end2);
		m_end3=min(fsize,m_end3);
	}
	else{  //not a real file, must be a swarm
		m_send_delay=0;
	}

	int ends[3];
	int starts[3];
	ends[0]=m_end1;
	ends[1]=m_end2;
	ends[2]=m_end3;
	starts[0]=m_start1;
	starts[1]=m_start2;
	starts[2]=m_start3;

//	if(b_is_false_decoy){  //we always want to leave out part of the file if its bigger than PARTSIZE
//		the_file=(DonkeyFile*)v_tmp.Get(0);

		//lets go ahead and serve up the entire file.
		/*  code to not serve up entire file
		if(the_file->m_size>PARTSIZE){  //only apply to large files, so that the user may never finish downloading them.  If the file is small we may as well distribute the whole file even though they will see that it is fake
			UINT part_to_leave_out=100%the_file->mv_hash_set.Size();
			//if they are asking for a block within this section, we will just release the slot and let them figure it out.
			if(m_end1>(part_to_leave_out*PARTSIZE) && m_end1<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0a]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
			if(m_end2>(part_to_leave_out*PARTSIZE) && m_end2<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0b]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
			if(m_end3>(part_to_leave_out*PARTSIZE) && m_end3<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0c]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
		}
		*/
//	}

//	if(is_false_decoy){  //we are now keeping track of requests, but this isn't terribly accurate since we are logging a request for a 384k chuck of the file as a request :)
		//ref.System()->GetRequestLogger()->LogFalseDecoyRequest(the_file->m_project.c_str(),mp_tcp_socket->GetIP(),the_file->m_file_name.c_str());
//	}
	//byte buffer[10200];
	
	bool b_feed=false;
	bool b_compress=false;

	if(b_is_false_decoy || b_is_real_decoy){  //if its a false decoy, we know it is extremely compressible and thus we shall compress
		b_compress=true;
	}
	else if(b_is_swarm){  //its a swarm
		b_feed=true;  //we want to go into feed mode where we keep sending the same part over and over
	}

	if(b_is_false_decoy && the_file->m_size>PARTSIZE)
		b_compress=false;

	int feed_size=2;

	if(mb_overnet){
		b_compress=false;
	}

	int chunk_size=10200;
	if(b_compress){
		chunk_size=BLOCKSIZE;  //a much bigger buffer if we are going to compress
	}

	TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Entering Send Loop\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);

	//ban this client after a lot of activity, we need to cut down on load
	ref.System()->GetClientManager()->UseCredit(m_ip.c_str());

	for(int send_index=0;send_index<3;send_index++){
		UINT send_start;
		UINT send_end;
		int length=0;
		if(ends[send_index]!=0){
			send_start=starts[send_index];
			send_end=ends[send_index];

			if(send_end<send_start){  //if end comes before start, then swap them.
				UINT tmp=send_end;
				send_end=send_start;
				send_start=tmp;
			}

			TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, sending data %u-%u\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
			

			length=send_end-send_start;
			if(length>BLOCKSIZE)  //to prevent reading past the end of a buffer lets limit the client to asking for at most BLOCKSIZE data, which is all it should ever be asking for anyways.
				length=BLOCKSIZE;

			if(send_end>the_file->m_size){
				TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request\n",mp_tcp_socket->GetIP(),starts[0],ends[0],starts[1],ends[1],starts[2],ends[2]);
				mb_abort=true;
				if(file_handle!=NULL){
					CloseHandle(file_handle);
				}
				//log_msg.Format("KadFTConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
				//ref.System()->LogToFile(log_msg);

				TRACE("KadFTConnection::RespondSendParts() %s END File %s, length=%u Send loop %d, requested data %u-%u is outside the file size\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);

				return;
			}

			int start=send_start;
			
			int sent=0;
			int togo=length;
			if(b_is_real_decoy){
				SetFilePointer(file_handle,start,0,FILE_BEGIN);
			}

			//sysref.System()->AddToQueuedData(length);  //let the system know we have queued up more data

			//send out a bunch of packets for one of those things.
			while(togo>0){
				DWORD num_read=min(togo,chunk_size);
				if(b_is_real_decoy){  //if it is real actually read the file info
					BOOL stat=ReadFile(file_handle,g_buffer,min(togo,chunk_size),&num_read,NULL);
					if(!stat){
						TRACE("Client Connection %s : Sending Parts %d-%d,%d-%d,%d-%d error reading file at offset %d,%d\n",mp_tcp_socket->GetIP(),starts[0],ends[0],starts[1],ends[1],starts[2],ends[2],start,togo);
						mb_abort=true;
						CloseHandle(file_handle);
						TRACE("KadFTConnection::RespondSendParts() %s END File %s, length=%u Send loop %d, requested data %u-%u FAILED TO READ REAL FILE\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
						return;
					}
				}
				else if(b_is_false_decoy){
					TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u BEGIN predicting data\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);

					the_file->PredictData(g_buffer,start+sent,min(togo,chunk_size));

					TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u END predicting data\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				}

				togo-=num_read;

				mb_needs_send=true;

				TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u BEGIN creating packet\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);

				Buffer2000 packet;
				packet.WriteByte(0xe3);
				packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
				packet.WriteByte(0x46);

				packet.WriteBytes(hash,16);  //write in hash

				//TRACE("Client Connection %s: Sent offset %d-%d\n",m_ip.c_str(),send_start+sent,send_start+sent+num_read);

				if(b_compress){
					packet.WriteDWord(send_start+sent);  //write in the start offset
				}
				else{
					if(b_feed){
						packet.WriteDWord(send_start);  //write in the start offset
						packet.WriteDWord(send_start+feed_size);  //write in the end offset of only 1k of data regardless
					}
					else{
						packet.WriteDWord(send_start+sent);  //write in the start offset
						packet.WriteDWord(send_start+sent+num_read);  //write in the end offset - if not compressed
					}
				}
				
				Buffer2000 compressed_data;

				if(b_is_swarm){  //is it a file that is physically on the hard disk
					if(b_compress){  //COMPRESS THE DATA
						if(num_read==BLOCKSIZE)
							compressed_data.WriteBytes(g_compressed_garbage.GetBufferPtr(),g_compressed_garbage.GetLength());
						else
							CompressData(g_garbage_buffer,num_read,compressed_data);
					}
					else{
						//packet.WriteBytes(g_garbage_buffer,num_read);
						if(b_feed){
							packet.WriteBytes(g_garbage_buffer,feed_size);
						}
						else{
							packet.WriteBytes(g_garbage_buffer,num_read);
						}
					}
				}
				else{  //is a false decoy or real decoy
					if(b_compress){  //COMPRESS THE DATA
						TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u BEGIN compressing data, numread=%d\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end,num_read);
						CompressData(g_buffer,num_read,compressed_data);
						TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u END compressing data, numread=%d\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end,num_read);
					}
					else{
						//packet.WriteBytes(buffer,num_read);
						if(b_feed){
							packet.WriteBytes(g_buffer,feed_size);
						}
						else{
							packet.WriteBytes(g_buffer,num_read);
						}
					}
				}

				if(b_compress){  //COMPRESSED DATA
					TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u BEGIN writing compressed data\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
					packet.WriteDWord(compressed_data.GetLength());  //write in the packed length
					packet.WriteBytes(compressed_data.GetBufferPtr(),compressed_data.GetLength());  //write in the compressed data
					TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u END writing compressed data\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				}

				packet.WriteDWord(1,packet.GetLength()-5);  //fix the length
				//packet.WriteDWord(1,compressed_data.GetLength()+25);  //fix the length

				if(b_compress){
					packet.WriteByte(0xc5,0);  //temporarily switch to the emule protocol
					packet.WriteByte(0x40,5);  //specify an emule compressed part
				}
				
				TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u END creating packet\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);

				if(b_feed){
					//FEED
					//save the packet we want to send over and over and over
					if(send_index==0){
						m_feed_packet1.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
					}
					else if(send_index==1){
						m_feed_packet2.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
					}
					else if(send_index==2){
						m_feed_packet3.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
					}

					mb_needs_send=true;
					m_send_delay=0;  //our delay between packet sends
					break;
					//END FEED
				}
				else{  //we aren't feeding, so lets just send the real data that they asked for
					SendData(packet.GetBufferPtr(),packet.GetLength());
					sent+=num_read;
				}
				TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u Send loop %d, requested data %u-%u packet queued for send\n",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
			}
		}
	}

	if(file_handle!=NULL){
		CloseHandle(file_handle);
	}

	if(b_feed){
		mb_feed=true;
		Feed();  //send indefinitely
	}

	TRACE("KadFTConnection::RespondSendParts() %s File %s, length=%u END Normal",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
}

void KadFTConnection::ReleaseSlot(void)
{
	TRACE("KadFTConnection::ReleaseSlot() %s STARTED.\n",m_ip.c_str());

	if(!mp_tcp_socket){
		mb_abort=true;
		return;
	}

	//create a packet to respond to the client
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x56); //grant the slot
	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	SendData(packet.GetBufferPtr(),packet.GetLength());

	TRACE("KadFTConnection::ReleaseSlot() %s FINISHED.\n",m_ip.c_str());
}

/*
void KadFTConnection::AddToSendQueue(byte * buffer, UINT buffer_length,int start_offset, int end_offset)
{
	CString file_name;
	file_name.Format("c:\\metamachine\\send_queue\\send_%s_%u_%u.dat",mp_tcp_socket->GetIP(),mp_tcp_socket->m_hSocket,m_send_queue_index);

	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_WRITE,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				CREATE_ALWAYS,             // always create a new file
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		TRACE("Client Connection %s: Could not save queue with filename %s.\n",mp_tcp_socket->GetIP(),file_name);
		mb_abort=true;
		CloseHandle(file_handle);
		return;
	}

	m_send_queue_index++;

	DWORD tmp;
	WriteFile(file_handle,buffer,buffer_length,&tmp,NULL);
	CloseHandle(file_handle);

	if(tmp!=buffer_length){
		TRACE("Client Connection %s: Couldn't write the proper amount of data to the file %s: %d bytes, %d actually written.\n",mp_tcp_socket->GetIP(),file_name,buffer_length,tmp);
		mb_abort=true;
		return;
	}

	v_send_queue_file_names.push_back(string((LPCSTR)file_name));
	v_send_queue_start_offset.push_back(start_offset);
	v_send_queue_end_offset.push_back(end_offset);
}
*/

/*
void KadFTConnection::CleanSendQueue(void)
{
	for(int i=0;i<(int)v_send_queue_file_names.size();i++){
		DeleteFile(v_send_queue_file_names[i].c_str());
	}

	v_send_queue_file_names.clear();
	v_send_queue_start_offset.clear();
	v_send_queue_end_offset.clear();
}*/

/*
void KadFTConnection::ProcessSendQueue(void)
{
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		return;
	}
	if(v_send_queue_file_names.size()==0 || !mb_needs_send || !m_last_send.HasTimedOut(m_send_delay) || mb_abort)
		return;

	mb_needs_send=false;
	m_last_send.Refresh();

	byte buffer[10200];

	int loop_count=1;
	if(m_send_delay<2)
		loop_count=2;
	for(int i=0;i<loop_count && !mb_abort;i++){
		CString file_name=v_send_queue_file_names[0].c_str();
		
		HANDLE file_handle=CreateFile(file_name, // open file at local_path 
					GENERIC_READ,              // open for reading 
					FILE_SHARE_READ,           // share for reading 
					NULL,                      // no security 
					OPEN_EXISTING,             // existing file only 
					FILE_ATTRIBUTE_NORMAL,     // normal file 
					NULL);                     // no attr. template 


		if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
			TRACE("Client Connection %s: Could not open queue with filename %s.\n",mp_tcp_socket->GetIP(),file_name);
			mb_abort=true;
			CloseHandle(file_handle);
			DeleteFile(file_name);
			return;
		}

		DWORD high_size=0;
		DWORD file_size=GetFileSize(file_handle,&high_size);

		if(file_size>10200 || file_size==0){
			TRACE("Client Connection %s: file size %d was bigger than 10200 or was 0 for queue file %s.\n",mp_tcp_socket->GetIP(),file_size,file_name);
			mb_abort=true;
			CloseHandle(file_handle);
			DeleteFile(file_name);
			return;
		}

		DWORD tmp;
		ReadFile(file_handle,buffer,file_size,&tmp,NULL);
		CloseHandle(file_handle);
		DeleteFile(file_name); //free up harddrive space

		if(tmp!=file_size){
			TRACE("Client Connection %s: Couldn't read the proper amount of data from the file %s: %d bytes, %d actually read.\n",mp_tcp_socket->GetIP(),file_name,file_size,tmp);
			mb_abort=true;
			return;
		}

		Buffer2000 packet;
		packet.WriteByte(0xe3);
		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
		packet.WriteByte(0x46);

		packet.WriteBytes(m_desired_hash,16);  //write in the hash of the file

		//TRACE("Client Connection %s: Sent offset %d-%d\n",m_ip.c_str(),send_start+sent,send_start+sent+num_read);
		packet.WriteDWord(v_send_queue_start_offset[0]);//send_start+sent);  //write in the start offset
		packet.WriteDWord(v_send_queue_end_offset[0]);  //write in the end offset

		packet.WriteBytes(buffer,file_size);

		packet.WriteDWord(1,packet.GetLength()-5);  //fix the length

		m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

		//SpooferTCPReference tcp_ref;
		//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());

		//pop these off the head of our send queue vectors
		v_send_queue_file_names.erase(v_send_queue_file_names.begin()+0);
		v_send_queue_start_offset.erase(v_send_queue_start_offset.begin()+0);
		v_send_queue_end_offset.erase(v_send_queue_end_offset.begin()+0);
	}
	
}*/

/*
void KadFTConnection::CompressPacket(Buffer2000& packet)
{
	if(packet.GetLength()<6)
		return;
	UINT original_data_size=packet.GetLength();
	byte *compressed_data=new byte[original_data_size+4096];  //an extra 4k that is more than adequate for compression headers on data that is not very compressable via lossless compression or because of small size
	UINT compressed_length=original_data_size+4096;
	compress ((Bytef *)compressed_data, (uLongf *)&compressed_length, (const Bytef *)packet.GetBufferPtr()+6, (uLong) packet.GetLength()-6);
	
	byte tmp_buf[6];
	memcpy(tmp_buf,packet.GetBufferPtr(),6);

	packet.Clear();

	//rewrite the header
	tmp_buf[0]=0xd4;  //set to emule compressed packet
	packet.WriteBytes(tmp_buf,6);

	//append the zlib compressed data
	packet.WriteBytes(compressed_data,compressed_length);

	//re-encode the length
	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);

	delete[] compressed_data;
}
*/

void KadFTConnection::CompressData(byte* data, UINT data_len, Buffer2000 &compressed_data)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("KadFTConnection::CompressData() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	UINT original_data_size=data_len;

	UINT compressed_length=original_data_size+8192;
	byte *compressed_bytes=new byte[compressed_length];  //an extra 8k that is more than adequate for compression headers on data that is not very compressable via lossless compression or because of small size
	
	compress2 ((Bytef *)compressed_bytes, (uLongf *)&compressed_length, (const Bytef *)data, (uLong) original_data_size, 9);
	

	compressed_data.Clear();

	//append the zlib compressed data
	compressed_data.WriteBytes(compressed_bytes,compressed_length);

	delete[] compressed_bytes;
	//log_msg.Format("KadFTConnection::CompressData() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

//slowly resend the same data over and over to this client.  To pause a swarmed file indefinitely
void KadFTConnection::Feed(void)
{
//	MetaSpooferReference ref;
//	CString log_msg;
//	log_msg.Format("KadFTConnection::Feed() %s STARTED.",m_ip.c_str());
//	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("KadFTConnection::Feed() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	if(!m_last_feed.HasTimedOut(2) || mb_abort){  //once a sec send another byte
		//log_msg.Format("KadFTConnection::Feed() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}

	m_last_feed.Refresh();
	m_last_send.Refresh();
	mb_needs_send=false;
	//SpooferTCPReference tcp_ref;

	if(m_feed_pos==0 && m_feed_packet1.GetLength()>0){
		SendData(m_feed_packet1.GetBufferPtr(),m_feed_packet1.GetLength());
		//m_send_buffer.WriteBytes(m_feed_packet1.GetBufferPtr(),m_feed_packet1.GetLength());

	
		//tcp_ref.TCP()->SendData(m_tcp_connection,m_feed_packet1.GetBufferPtr(),m_feed_packet1.GetLength());
	}
	else if(m_feed_pos==1 && m_feed_packet2.GetLength()>0){
		SendData(m_feed_packet2.GetBufferPtr(),m_feed_packet2.GetLength());
		//m_send_buffer.WriteBytes(m_feed_packet2.GetBufferPtr(),m_feed_packet2.GetLength());

		//tcp_ref.TCP()->SendData(m_tcp_connection,m_feed_packet2.GetBufferPtr(),m_feed_packet2.GetLength());
	}
	else if(m_feed_pos==2 && m_feed_packet3.GetLength()>0){
		SendData(m_feed_packet3.GetBufferPtr(),m_feed_packet3.GetLength());
		//m_send_buffer.WriteBytes(m_feed_packet3.GetBufferPtr(),m_feed_packet3.GetLength());

		//tcp_ref.TCP()->SendData(m_tcp_connection,m_feed_packet3.GetBufferPtr(),m_feed_packet3.GetLength());
	}
	m_feed_pos++;
	if(m_feed_pos>=3)
		m_feed_pos=0;

	m_idle.Refresh();
//	log_msg.Format("KadFTConnection::Feed() %s FINISHED.",m_ip.c_str());
//	ref.System()->LogToFile(log_msg);
}

void KadFTConnection::OnClose(CAsyncSocket* src)
{
	//CSingleLock lock(&m_lock,TRUE);
	//TRACE("KadFTConnection::OnClose() %s STARTED.\n",m_ip.c_str());
	
	mb_abort=true;
	//if(mp_tcp_socket)
	//	delete mp_tcp_socket;
	//mp_tcp_socket=NULL;
	//TRACE("KadFTConnection::OnClose() %s FINISHED.\n",m_ip.c_str());
}

void KadFTConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length)
{
	KademliaProtectorSystemReference ref;
	ref.System()->LogToFile("KadFTConnection::OnReceive() BEGIN");
	CSingleLock lock(&m_lock,TRUE);
	TRACE("KadFTConnection::OnReceive() %s.\n",m_ip.c_str());
	m_buffer.WriteBytes(data,length);
	m_idle.Refresh();
	ref.System()->LogToFile("KadFTConnection::OnReceive() END");
}

void KadFTConnection::OnSend(CAsyncSocket* src)
{
	KademliaProtectorSystemReference ref;
	ref.System()->LogToFile("KadFTConnection::OnSend() BEGIN");
	CSingleLock lock(&m_lock,TRUE);
	TRACE("KadFTConnection::OnSend() %s.\n",m_ip.c_str());
	mb_needs_send=true;
	mb_can_send=true;
	m_idle.Refresh();

	CheckSendBuffer();
	ref.System()->LogToFile("KadFTConnection::OnSend() END");
}

void KadFTConnection::OnConnect(CAsyncSocket* src)
{
}

void KadFTConnection::GrantSlot(void)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("KadFTConnection::GrantSlot() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("KadFTConnection::GrantSlot() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	//create a packet to respond to the client
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x55); //grant the slot
	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	SendData(packet.GetBufferPtr(),packet.GetLength());
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());

	m_idle.Refresh();
	//log_msg.Format("KadFTConnection::GrantSlot() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

unsigned short KadFTConnection::GetListeningPort(void)
{
	return m_listening_port;
}

void KadFTConnection::CheckSendBuffer(void)
{
	if( mv_send_buffers.Size()>0 && mp_tcp_socket && !mb_abort){
		Buffer2000 *queued_data=(Buffer2000*)mv_send_buffers.Get(0);

		mb_can_send=false;

		int send_stat=mp_tcp_socket->Send(queued_data->GetBufferPtr(),queued_data->GetLength());  //TYDEBUG

		if(SOCKET_ERROR==send_stat){
			return;
		}
		else if(send_stat>0 && send_stat<20000000){
			if(queued_data->GetLength()==send_stat)
				queued_data->Clear();
			else
				queued_data->RemoveRange(0,send_stat);
		}
		if(queued_data->GetLength()==0)  //have we finished sending all this data?
			mv_send_buffers.Remove(0);
	}
}

void KadFTConnection::RespondDontHaveFile(byte *hash)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("KadFTConnection::RespondDontHaveFile() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("KadFTConnection::RespondDontHaveFile() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	//create a packet to respond to the client
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x48); //tell this client we don't have this file anymore, this is to keep them from repeatedly asking forever and ever
	packet.WriteBytes(hash,16);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength());
	//log_msg.Format("KadFTConnection::RespondDontHaveFile() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

const char* KadFTConnection::IsNoSwarmAddress(const char* address)
{
	CString tmp=address;
	tmp=tmp.MakeLower();

	int dot_index=tmp.Find(".");
	while(dot_index!=-1){
		int tmp_index=tmp.Find(".",dot_index+1);
		if(tmp_index!=-1)
			dot_index=tmp_index;
		else
			break;
	}

	if(dot_index==-1)
		return NULL;

	CString tld=tmp.Right(tmp.GetLength()-dot_index);

	if(tld.Compare(".edu")==0)
		return "School";

	if(tld.Compare(".de")==0)  //let germany through
		return NULL;

	if(tmp.Find(".aol")!=-1)
		return "aol user";


	if(tld.GetLength()==3) //.fr etc
		return "bad country";

	return NULL;
}

void KadFTConnection::ProcessEmulePacket(Buffer2000& packet)
{
	const byte* p_data=packet.GetBufferPtr();
	int buffer_offset=5;
	byte cmd=*(p_data+buffer_offset);
	buffer_offset++;

	
	if(cmd==OP_MULTIPACKET){
		KademliaProtectorSystemReference ref;

		TRACE("KadFTConnection::ProcessEmulePacket(Buffer2000& packet) %s handling emule specific OP_MULTIPACKET.\n",m_ip.c_str());

		//debug printout
		//for(int i=0;i<(int)packet.GetLength();i++){
		//	TRACE("incoming packet[%d]=0x%x.\n",i,*((byte*)(packet.GetBufferPtr()+i)));
		//}

		//build a response or this emule ain't gonna do shit
		Buffer2000 response_packet;
		response_packet.WriteByte(0xc5);  //emule response
		response_packet.WriteDWord(0);  //specific length
		response_packet.WriteByte(OP_MULTIPACKETANSWER);
		byte hash[16];
		memcpy(hash,p_data+buffer_offset,16);

		//the hash of the file they want...

		//find this file in our system
		Vector v_tmp;

		bool b_false_decoy=false;

		bool b_log_request=false;

		ref.System()->GetSwarmFile(v_tmp,hash);	

		if(v_tmp.Size()==0)
			b_false_decoy=ref.System()->GetDecoyFile(v_tmp,hash);

		if(v_tmp.Size()==0){
			RespondDontHaveFile(hash);
			mb_abort=true;
			return;
		}

		//here is the file they are looking for
		DonkeyFile* the_file=(DonkeyFile*)v_tmp.Get(0);

		response_packet.WriteBytes(hash,16);

		buffer_offset+=16;  //skip past the hash

		bool b_do_respond=false;
		
		bool b_asked_for_status=false;

		while(buffer_offset<(int)packet.GetLength()){
			byte op_code=*(p_data+buffer_offset);
			buffer_offset++;
			if(op_code==OP_REQUESTFILENAME){
				TRACE("KadFTConnection::ProcessEmulePacket(Buffer2000& packet) %s responding to OP_MULTIPACKET->OP_REQUESTFILENAME.\n",m_ip.c_str());
				b_do_respond=true;

				response_packet.WriteByte(OP_REQFILENAMEANSWER);
				response_packet.WriteWord((WORD)the_file->m_file_name.size());  //write in name length
				response_packet.WriteBytes((byte*)the_file->m_file_name.c_str(),(UINT)the_file->m_file_name.size());  //write in name
			}
			else if(op_code==OP_SETREQFILEID){  //they want the hashset
				TRACE("KadFTConnection::ProcessEmulePacket(Buffer2000& packet) %s responding to OP_MULTIPACKET->OP_SETREQFILEID.\n",m_ip.c_str());
				b_do_respond=true;
				b_asked_for_status=true;
				response_packet.WriteByte(OP_FILESTATUS);
				the_file->WriteFileStatus(response_packet,b_false_decoy,false);
			}
			else if(op_code==OP_REQUESTSOURCES){  //here is where we blast them with whatever sources we want
				TRACE("KadFTConnection::ProcessEmulePacket(Buffer2000& packet) %s responding to OP_MULTIPACKET->OP_REQUESTSOURCES.\n",m_ip.c_str());
			}
		}

		if(b_do_respond){
			UINT packet_length=response_packet.GetLength();
			response_packet.WriteDWord(1,packet_length-5);  //fix the length
			//respond to this pirate
			SendData(response_packet.GetBufferPtr(),response_packet.GetLength());
		}
	}
}

void KadFTConnection::SendData(const byte* p_data, UINT data_length)
{
	mv_send_buffers.Add(new Buffer2000(p_data,data_length));
}

void KadFTConnection::RespondOvernet0x66(byte *hash)
{
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x66);
	packet.WriteBytes(hash,16);  //write in hash
	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//Lets send file request response to this pirate

	SendData(packet.GetBufferPtr(),packet.GetLength());
}


/*
void KadFTConnection::RespondOvernet0x68(byte* hash)
{
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x68);
	packet.WriteBytes(hash,16);  //write in hash


	byte peer1_1[] = {
	0x01, 0xf2, 
	0xad, 0x7d, 0x97, 0x51, 0xc8, 0x79, 0x38, 0x6c, 
	0xa1, 0x78, 0x70, 0xcf, 0x86, 0x51, 0x7e, 0xf4, 
	0x4a, 0xc9, 0x9b, 0xc2, 0x2b, 0xec, 0x49, 0x3e, 
	0x15, 0x37, 0x53, 0x57, 0xcb, 0x56, 0x2c, 0xcf, 
	0x0a, 0x8b, 0x4b, 0x8f, 0xa2, 0xae, 0x03, 0xd3, 
	0x6b, 0xc0, 0xc6, 0xc1, 0x8d, 0xed, 0x1e, 0x64, 
	0x62, 0xbc, 0xdd, 0x45, 0xab, 0x68, 0xc1, 0x49, 
	0xd6, 0xd7, 0x04, 0x7e, 0x13, 0x70, 0xa3, 0xfe, 
	0xec, 0xdf, 0xaf, 0xc1, 0xa5, 0xbc, 0xed, 0xcd, 
	0x92, 0xe9, 0x44, 0xb1, 0x39, 0x2b, 0x75, 0xf2, 
	0x3b, 0xc3, 0x21, 0x69, 0x09, 0x23, 0xe6, 0xd0, 
	0x13, 0x2b, 0x89, 0xd5, 0xef, 0x39, 0x58, 0x99, 
	0x78, 0xf9, 0xb7, 0x0a, 0x02, 0xc8, 0xc2, 0x54, 
	0xaa, 0x6f, 0xac, 0xdc, 0xee, 0xb4, 0x11, 0x37, 
	0x17, 0x4c, 0xba, 0xd6, 0xbe, 0x2b, 0x4b, 0xa5, 
	0x62, 0x69, 0x14, 0x54, 0x1b, 0x52, 0xf5 };

	packet.WriteBytes(peer1_1,sizeof(peer1_1));

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//Lets send file request response to this pirate

	SendData(packet.GetBufferPtr(),packet.GetLength());
}*/

const char* KadFTConnection::GetIP(void)
{
	return m_ip.c_str();
}
