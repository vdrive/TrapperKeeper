#include "StdAfx.h"
#include "clientconnection.h"
#include <mmsystem.h>
#include "DonkeyFile.h"
#include "spoofingsystem.h"
#include "..\tkcom\PeerSocket.h"
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

ClientConnection::ClientConnection(CAsyncSocket* socket,unsigned short listening_port, byte *my_hash,string my_user_name)
//: m_tcp_connection(con)
{
	m_ip=((PeerSocket*)socket)->GetIP();
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::ClientConnection() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	m_listening_port=listening_port;
	mp_tcp_socket=(PeerSocket*)socket;
	mp_tcp_socket->SetSocketEventListener(this);
	
	m_port=mp_tcp_socket->GetPort();

	//TRACE("New Client Connection Created %s.\n",mp_tcp_socket->GetIP());
	mb_is_declared_source=false;

	m_state=CLIENT_START;
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
	//log_msg.Format("ClientConnection::ClientConnection() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);

	if(rand()&1)
		mb_compressed_poisoner=true;
	else
		mb_compressed_poisoner=false;

	mb_no_swarms_allowed=false;

	mb_emule_compatible=false;

	mb_eserver=false;

	m_throttle_speed=200+(rand()%100);
}

ClientConnection::~ClientConnection(void)
{
	TRACE("MetaMachineSpoofer ClientConnection::~ClientConnection().\n");
	//Close the connection that this object represents
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::~ClientConnection() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);


	//CleanSendQueue();

	if(mp_tcp_socket!=NULL){
		delete mp_tcp_socket;
		mp_tcp_socket=NULL;
	}

	//log_msg.Format("ClientConnection::~ClientConnection() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

void ClientConnection::CheckBuffer(void)
{
	if(m_buffer.GetLength()<5 || mb_abort)
		return;

	//0x%02X

	const byte* p_data=m_buffer.GetBufferPtr();
	if(p_data[0]!=0xe3 && p_data[0]!=0xc5){
		m_buffer.Clear();  //discard this data since we don't know what to do with it
		TRACE("Received invalid packet from %s with first byte = 0x%x.\n",m_ip.c_str(),p_data[0]);
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

	CString log_msg;
	log_msg.Format("%s ClientConnection::CheckBuffer() BEGIN message type protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
	SpoofingSystem::LogToFile(log_msg);

	//len does not include the first 5 bytes of the packet

	//ok here we have a valid packet of data
	if(len<1){  //no packets should be this short lets abort
		mb_abort=true;
		m_buffer.Clear();

		log_msg.Format("%s ClientConnection::CheckBuffer() END [0 - ABNORMAL] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}

//	byte* buffer=(byte*)m_buffer.GetBufferPtr();
	bool b_emule_packet=false;
	if(p_data[0]==0xc5){
		b_emule_packet=true;
		//TRACE("Client Connection %s: Removing Emule Specific Packet\n",mp_tcp_socket->GetIP());
		TRACE("Received emule specific packet from %s with command byte byte = 0x%x.\n",m_ip.c_str(),p_data[5]);
		//m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		//return;
	}
	else{
		TRACE("Received ed2k packet from %s with command byte byte = 0x%x.\n",m_ip.c_str(),p_data[5]);
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
					TRACE("MetaMachineSpoofer::ClientConnection::CheckBuffer() Setting client %s as emule compatible.\n",m_ip.c_str());
					break;
				}
			}
		}
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_HELLO;
		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [1] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x4f){ //file status request
		//TRACE("Client Connection %s: Got File Status Request\n",mp_tcp_socket->GetIP());
		memcpy(m_desired_hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_FILE_STATUS;
		m_idle.Refresh();

		log_msg.Format("%s ClientConnection::CheckBuffer() END [2] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x58){ //file status request
		//TRACE("Client Connection %s: Got File Request\n",mp_tcp_socket->GetIP());
		memcpy(m_desired_hash,p_data+6,16);
		Vector v_tmp;
//		MetaSpooferReference ref;
//		ref.System()->GetFile(v_tmp,m_desired_hash);
//		if(v_tmp.Size()>0){
			//TRACE("Client Connection %s: Got File Request, we have the file\n",mp_tcp_socket->GetIP());
//		}
//		else{
			//TRACE("Client Connection %s: Got File Request, we do not have the file\n",mp_tcp_socket->GetIP());
//		}

		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_FILE_REQUEST;
		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [3] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
	
	if(!b_emule_packet && len>=1+16 && p_data[5]==0x51 ){ //file status request
		//TRACE("Client Connection %s: Got File Request\n",mp_tcp_socket->GetIP());
		memcpy(m_desired_hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_HASH_PARTS;
		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [4] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
	
	if(!b_emule_packet && len>=1 && p_data[5]==0x54){ //slot request
		//TRACE("Client Connection %s: Got Slot Request\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		GrantSlot();
		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [5] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
	if(!b_emule_packet && len>=1 && p_data[5]==0x56){ //slot request
		//TRACE("Client Connection %s: Slot Released\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [6] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}

	if(!b_emule_packet && len>=1+16+24 && p_data[5]==0x47 ){ //requested parts
		memcpy(m_desired_hash,p_data+6,16);
		m_start1=*((UINT*)(p_data+6+16));
		m_start2=*((UINT*)(p_data+6+16+4));
		m_start3=*((UINT*)(p_data+6+16+8));
		m_end1=*((UINT*)(p_data+6+16+12));
		m_end2=*((UINT*)(p_data+6+16+16));
		m_end3=*((UINT*)(p_data+6+16+20));
		TRACE("Client Connection %s: Got Send Parts %d-%d,%d-%d,%d-%d\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_SEND_PARTS;
		m_idle.Refresh();
		log_msg.Format("%s ClientConnection::CheckBuffer() END [7] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
		SpoofingSystem::LogToFile(log_msg);
		return;
	}

	if(b_emule_packet){
		Buffer2000 emule_packet(m_buffer.GetBufferPtr(),len+5);//m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer
		ProcessEmulePacket(emule_packet);
	}

	//TRACE("Client Connection %s: Got Unknown Command 0x%x\n",mp_tcp_socket->GetIP(),p_data[5]);
	m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

	log_msg.Format("%s ClientConnection::CheckBuffer() END [8] protocol=0x%02X, type=0x%02X",m_ip.c_str(),protocol,cmd);
	SpoofingSystem::LogToFile(log_msg);
}

void ClientConnection::Update(void)
{


	if(mb_feed)
		Feed();
	
	CheckBuffer();
	if(m_state==CLIENT_HELLO){
		MetaSpooferReference ref;
		CString log_msg;

		log_msg.Format("ClientConnection::Update() %s RespondHello BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);

		RespondHello();

		log_msg.Format("ClientConnection::Update() %s RespondHello END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}
	else if(m_state==CLIENT_FILE_STATUS){
		MetaSpooferReference ref;
		CString log_msg;

		log_msg.Format("ClientConnection::Update() %s RespondFileStatus BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);

		RespondFileStatus();

		log_msg.Format("ClientConnection::Update() %s RespondFileStatus END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}
	else if(m_state==CLIENT_FILE_REQUEST){
		MetaSpooferReference ref;
		CString log_msg;

		log_msg.Format("ClientConnection::Update() %s RespondFileRequest BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);

		RespondFileRequest();

		log_msg.Format("ClientConnection::Update() %s RespondFileRequest END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}
	else if(m_state==CLIENT_HASH_PARTS){
		MetaSpooferReference ref;
		CString log_msg;

		log_msg.Format("ClientConnection::Update() %s RespondHashParts BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);

		RespondHashParts();

		log_msg.Format("ClientConnection::Update() %s RespondHashParts END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}
	else if(m_state==CLIENT_SEND_PARTS){
		MetaSpooferReference ref;
		CString log_msg;

		log_msg.Format("ClientConnection::Update() %s RespondSendParts BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);

		RespondSendParts();

		log_msg.Format("ClientConnection::Update() %s RespondSendParts END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}

//	ProcessSendQueue();
	CheckSendBuffer();
	//log_msg.Format("ClientConnection::Update() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

void ClientConnection::RespondHello(void)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::RespondHello() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondHello() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
//	TRACE("Client Connection %s: Responding Hello Packet\n",m_ip.c_str());
	UINT my_id=16777217+(int)rand();

	srand(timeGetTime());

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
	packet.WriteBytes(m_my_hash,16);

	if(mb_eserver){  //does this client look like an edonkey server?
		packet.WriteDWord(0);  //we had better have a low id to this server
	}
	else{
		packet.WriteDWord(my_id);  //don't want anyone else thinking we have a lowid
	}
	packet.WriteWord(m_listening_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know
	packet.WriteDWord(5); //number of metatags metatags

	//metatag - name
	packet.WriteByte(0x02);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x01);	
	packet.WriteWord(my_name_length);		//my name length
	packet.WriteString(my_name);		//my name

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

	MetaSpooferReference ref;
	packet.WriteDWord(ref.System()->GetEmuleVersion());		//my version
	//packet.WriteDWord(43520);		//my version

	/*
	Buffer2000 packet;	
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO RESPONSE
	packet.WriteByte(0x4c);  //Set the command byte equal to a HELLO_RESPONSE 0x4c
	packet.WriteBytes(m_my_hash,16);
	packet.WriteDWord(my_id);
	packet.WriteWord(m_listening_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know
	packet.WriteDWord(3); //3 metatags

	//encode name
	packet.WriteDWord(name_nemonic);
	packet.WriteWord(my_name_length);
	packet.WriteString(my_name);

	//encode version
	packet.WriteDWord(version_nemonic);
	packet.WriteDWord(30);  //lets not say we are 0x3c because then they try to do some queer emule specific protocol

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(port_nemonic);
	packet.WriteDWord(m_listening_port);  	

	//encode the server address
	//(193.111.198.137:4242)...

	*/

	if((rand()%100)<5 && !mb_eserver){  //5% chance we will let them know about our server
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
	

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	m_state=CLIENT_HELLO_WAIT;

	//Lets send our hello response to this pirate
	SendData(packet.GetBufferPtr(),packet.GetLength());
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	//log_msg.Format("ClientConnection::RespondHello() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	//SpooferTCPReference reference;
	//reference.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::RespondHashParts(void)
{
	MetaSpooferReference ref;
//	CString log_msg;
//	log_msg.Format("ClientConnection::RespondHashParts() %s STARTED.",m_ip.c_str());
//	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
//		log_msg.Format("ClientConnection::RespondHashParts() %s FINISHED.",m_ip.c_str());
//		ref.System()->LogToFile(log_msg);
		return;
	}
	DonkeyFile *the_file=NULL;

	//MetaSpooferReference sysref;
	Vector v_tmp;
	ref.System()->GetDecoyFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0)
		ref.System()->GetFalseDecoyFile(v_tmp,m_desired_hash);
	if(v_tmp.Size()==0)
		ref.System()->GetSwarmFile(v_tmp,m_desired_hash);	

	if(v_tmp.Size()==0){
		RespondDontHaveFile();
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondHashParts() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
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
	m_state=CLIENT_HASH_PARTS_WAIT;

	//Lets send our hash parts for this file to this pirate
	SendData(packet.GetBufferPtr(),packet.GetLength());
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	//log_msg.Format("ClientConnection::RespondHashParts() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

/*
bool ClientConnection::IsConnection(TKTCPConnection &con)
{
	if(m_tcp_connection==con){
		return true;
	}
	else{
		return false;
	}
}*/

void ClientConnection::Shutdown(void)
{
	this->StopThread();
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("ClientConnection::Shutdown() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//Close the connection that this object represents
	mb_abort=true;
	if(mp_tcp_socket){
		mp_tcp_socket->Close();
		delete mp_tcp_socket;
	}
	mp_tcp_socket=NULL;
//	if(m_tcp_connection.GetHandle()){  //
//		SpooferTCPReference reference;
//		reference.TCP()->CloseConnection(m_tcp_connection);
//		m_tcp_connection.SetHandle(NULL);
//	}
	log_msg.Format("ClientConnection::Shutdown() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

bool ClientConnection::IsOld(void)
{

	if(mb_abort || m_created.HasTimedOut(2*60*60)){  //a connection will have a lifetime of at most 2 hours, or 15 minutes of idle time.
		return true;	
	}
	else{
		return false;
	}
}

void ClientConnection::ClearDesiredHash(void)
{
	for(int i=0;i<16;i++){
		m_desired_hash[i]=0;
	}
}

void ClientConnection::RespondFileStatus(void)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::RespondFileStatus() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		return;
	}
	DonkeyFile *the_file=NULL;

	MetaSpooferReference sysref;
	Vector v_tmp;
	sysref.System()->GetDecoyFile(v_tmp,m_desired_hash);
	bool b_false_decoy=false;
	if(v_tmp.Size()==0)
		b_false_decoy=sysref.System()->GetFalseDecoyFile(v_tmp,m_desired_hash);
	if(v_tmp.Size()==0)
		sysref.System()->GetSwarmFile(v_tmp,m_desired_hash);	

	if(v_tmp.Size()==0){
		RespondDontHaveFile();
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondFileStatus() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}


	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x50);
	packet.WriteBytes(the_file->m_hash,16);
	the_file->WriteFileStatus(packet,b_false_decoy);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	m_state=CLIENT_FILE_STATUS_WAIT;

	//Lets send our file status to this pirate

	SendData(packet.GetBufferPtr(),packet.GetLength());
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	//log_msg.Format("ClientConnection::RespondFileStatus() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::RespondFileRequest(void)
{
	MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::RespondFileRequest() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondFileRequest() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	DonkeyFile *the_file=NULL;

	//MetaSpooferReference sysref;
	if(stricmp(m_ip.c_str(),"38.119.66.24")==0)
		TRACE("Debug connection from myself active.\n");
	Vector v_tmp;
	ref.System()->GetDecoyFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0)
		ref.System()->GetFalseDecoyFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()!=0){
		//was a decoy that we need to report
		the_file=(DonkeyFile*)v_tmp.Get(0);
		ref.System()->LogDecoyRequest(the_file->m_file_name.c_str(),m_ip.c_str());
	}
	if(v_tmp.Size()==0){
		ref.System()->GetSwarmFile(v_tmp,m_desired_hash);	
		if(v_tmp.Size()!=0 && mb_no_swarms_allowed){
			TRACE("MetaMachineSpoofer ClientConnection %s requesting swarm, DENIED and CLOSING CONNECTION.\n",m_ip.c_str());
			RespondDontHaveFile();
			mb_abort=true;			
			return;
		}
	}

	if(v_tmp.Size()==0){
		RespondDontHaveFile();
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondHashParts() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}

	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x59);
	packet.WriteBytes(the_file->m_hash,16);  //write in hash
	packet.WriteWord((WORD)the_file->m_file_name.size());  //write in name length
	packet.WriteBytes((byte*)the_file->m_file_name.c_str(),(UINT)the_file->m_file_name.size());  //write in name

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets send file request response to this pirate

	SendData(packet.GetBufferPtr(),packet.GetLength());
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());


	//log_msg.Format("ClientConnection::RespondFileRequest() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}




void ClientConnection::RespondSendParts(void)
{
	MetaSpooferReference ref;

	if(ref.System()->IsSpoofingOnly()){
		mb_abort=true;
		return;
	}

	//CString log_msg;
	//log_msg.Format("ClientConnection::RespondSendParts() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	//CleanSendQueue();

	CString log_msg;
	log_msg.Format("%s ClientConnection::RespondSendParts() BEGIN",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);

	DonkeyFile *the_file=NULL;

	//MetaSpooferReference sysref;
	//unfortunately we need a memory management check here, to keep us from having too many sends queued up.  (we can easily use a gig of ram without it because we don't limit connections).
//	int queued_size=sysref.System()->GetQueuedDataSize();

//	if(queued_size>(60*(1<<20))){  //allow at most 60 megs to be queued up at any one time
//		return;  //lets let this method continue to be called until there is room in our queue to send this data out.
//	}

//	SpooferTCPReference tcp_ref;
	m_state=CLIENT_SEND_PARTS_WAIT;

	Vector v_tmp;
	bool b_is_real_decoy=ref.System()->GetDecoyFile(v_tmp,m_desired_hash);
	bool b_is_false_decoy=false;
	if(!b_is_real_decoy)
		b_is_false_decoy=ref.System()->GetFalseDecoyFile(v_tmp,m_desired_hash);
	bool b_is_swarm=false;
	if(!b_is_real_decoy && !b_is_false_decoy)
		b_is_swarm=ref.System()->GetSwarmFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0){
		TRACE("MetaMachineSpoofer ClientConnection %s requesting we send parts for a hash we don't have.\n",m_ip.c_str());
		mb_abort=true;
		log_msg.Format("%s ClientConnection::RespondSendParts() END[0] NO SUCH FILE",m_ip.c_str());
		SpoofingSystem::LogToFile(log_msg);
		return;
	}

	//we have a file now
	the_file=(DonkeyFile*)v_tmp.Get(0);


	log_msg.Format("%s ClientConnection::RespondSendParts() Requesting file %s, length=%u",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
	SpoofingSystem::LogToFile(log_msg);

	if(b_is_swarm && mb_no_swarms_allowed){
		TRACE("MetaMachineSpoofer ClientConnection %s requesting swarm, LATE CATCH, DENIED and CLOSING CONNECTION.\n",m_ip.c_str());
		mb_abort=true;

		log_msg.Format("%s ClientConnection::RespondSendParts() END[1] ABNORMAL",m_ip.c_str());
		SpoofingSystem::LogToFile(log_msg);
		return;
	}
//	bool is_real_file=ref.System()->GetFile(v_tmp,m_desired_hash);
//	bool is_false_decoy=false;
//	if(v_tmp.Size()==0){
//		is_false_decoy=is_real_file=ref.System()->GetSyntheticFile(v_tmp,m_desired_hash);
//	}

//	if(v_tmp.Size()==0){  
//		mb_abort=true;  //we don't have this file in any capacity
//		return;
//	}

	HANDLE file_handle = NULL;

	if(b_is_real_decoy){
		log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u is real decoy",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
		SpoofingSystem::LogToFile(log_msg);

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
			//TRACE("Client Connection %s: File Handle was %d.\n",file_handle);
			mb_abort=true;
			CloseHandle(file_handle);
			//log_msg.Format("ClientConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
			//ref.System()->LogToFile(log_msg);
			return;
		}

		DWORD fsize=0;
		DWORD hsize=0;
		fsize=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
		
		if(fsize<1){
			//TRACE("Client Connection %s: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",mp_tcp_socket->GetIP(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			//log_msg.Format("ClientConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
			//ref.System()->LogToFile(log_msg);
			log_msg.Format("%s ClientConnection::RespondSendParts() END File %s, length=%u Couldn't open real file",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
			SpoofingSystem::LogToFile(log_msg);
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
//	if(b_feed){
//		if((rand()%100)>45 ){
//			b_feed=false;  //disable feeding sometimes, so that we are able to corrupt a ton of other parts to the file
//			b_compress=true;  //enable compress so that we swarm really fast
//		}
//	}
	//b_feed=true;
	//b_compress=true;

	int chunk_size=10200;
	if(b_compress){
		chunk_size=BLOCKSIZE;  //a much bigger buffer if we are going to compress
	}

	log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Entering Send Loop",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
	SpoofingSystem::LogToFile(log_msg);

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

			log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, sending data %u-%u",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
			SpoofingSystem::LogToFile(log_msg);

			length=send_end-send_start;
			if(length>BLOCKSIZE)  //to prevent reading past the end of a buffer lets limit the client to asking for at most BLOCKSIZE data, which is all it should ever be asking for anyways.
				length=BLOCKSIZE;

			if(send_end>the_file->m_size){
				TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request\n",mp_tcp_socket->GetIP(),starts[0],ends[0],starts[1],ends[1],starts[2],ends[2]);
				mb_abort=true;
				if(file_handle!=NULL){
					CloseHandle(file_handle);
				}
				//log_msg.Format("ClientConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
				//ref.System()->LogToFile(log_msg);

				log_msg.Format("%s ClientConnection::RespondSendParts() END File %s, length=%u Send loop %d, requested data %u-%u is outside the file size",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				SpoofingSystem::LogToFile(log_msg);
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
						log_msg.Format("%s ClientConnection::RespondSendParts() END File %s, length=%u Send loop %d, requested data %u-%u FAILED TO READ REAL FILE",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
						SpoofingSystem::LogToFile(log_msg);
						return;
					}
				}
				else if(b_is_false_decoy){
					log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u BEGIN predicting data",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
					SpoofingSystem::LogToFile(log_msg);
					the_file->PredictData(g_buffer,start+sent,min(togo,chunk_size));
					log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u END predicting data",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
					SpoofingSystem::LogToFile(log_msg);
				}

				togo-=num_read;

				mb_needs_send=true;

				log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u BEGIN creating packet",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				SpoofingSystem::LogToFile(log_msg);

				Buffer2000 packet;
				packet.WriteByte(0xe3);
				packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
				packet.WriteByte(0x46);

				if(b_is_false_decoy || b_is_real_decoy){
					packet.WriteBytes(the_file->m_hash,16);  //write in hash

				}
				else{  //its a swarm
					packet.WriteBytes(m_desired_hash,16);  //write in hash
				}

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
					//is a swarm
					if(!mb_is_declared_source){  //have we already reported this client as a source for this hash?
						if(m_client_port>0){  //report this client as a source for this hash
							CString str_desired_hash=EncodeBase16(m_desired_hash,16);
							ref.System()->AddSourceForHash(mp_tcp_socket->GetIP(),m_client_port,str_desired_hash);
						}
						mb_is_declared_source=true;
					}
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
						log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u BEGIN compressing data, numread=%d",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end,num_read);
						SpoofingSystem::LogToFile(log_msg);
						CompressData(g_buffer,num_read,compressed_data);
						log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u END compressing data, numread=%d",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end,num_read);
						SpoofingSystem::LogToFile(log_msg);
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
					log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u BEGIN writing compressed data",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
					SpoofingSystem::LogToFile(log_msg);
					packet.WriteDWord(compressed_data.GetLength());  //write in the packed length
					packet.WriteBytes(compressed_data.GetBufferPtr(),compressed_data.GetLength());  //write in the compressed data
					log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u END writing compressed data",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
					SpoofingSystem::LogToFile(log_msg);
				}

				packet.WriteDWord(1,packet.GetLength()-5);  //fix the length
				//packet.WriteDWord(1,compressed_data.GetLength()+25);  //fix the length

				if(b_compress){
					packet.WriteByte(0xc5,0);  //temporarily switch to the emule protocol
					packet.WriteByte(0x40,5);  //specify an emule compressed part
				}
				
				log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u END creating packet",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				SpoofingSystem::LogToFile(log_msg);


				//CompressPacket(packet);  //compress it to make our data transfer extremely efficient

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

					//TRACE("MetaMachineSpoofer::ClientConnection() Sending File Data to %s.\n",mp_tcp_socket->GetIP());
					SendData(packet.GetBufferPtr(),packet.GetLength());
					//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

//					SpooferTCPReference tcp_ref;
//					tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
					sent+=num_read;
				}
				log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u Send loop %d, requested data %u-%u packet queued for send",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size,send_index,send_start,send_end);
				SpoofingSystem::LogToFile(log_msg);
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

	//log_msg.Format("ClientConnection::RespondSendParts() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	//Lets send our hello response to this pirate

	log_msg.Format("%s ClientConnection::RespondSendParts() File %s, length=%u END Normal",m_ip.c_str(),the_file->m_file_name.c_str(),the_file->m_size);
	SpoofingSystem::LogToFile(log_msg);
}

void ClientConnection::ReleaseSlot(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("ClientConnection::ReleaseSlot() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		log_msg.Format("ClientConnection::ReleaseSlot() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
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
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	log_msg.Format("ClientConnection::ReleaseSlot() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
//		SpooferTCPReference tcp_ref;
//		tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

/*
void ClientConnection::AddToSendQueue(byte * buffer, UINT buffer_length,int start_offset, int end_offset)
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
void ClientConnection::CleanSendQueue(void)
{
	for(int i=0;i<(int)v_send_queue_file_names.size();i++){
		DeleteFile(v_send_queue_file_names[i].c_str());
	}

	v_send_queue_file_names.clear();
	v_send_queue_start_offset.clear();
	v_send_queue_end_offset.clear();
}*/

/*
void ClientConnection::ProcessSendQueue(void)
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
void ClientConnection::CompressPacket(Buffer2000& packet)
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

void ClientConnection::CompressData(byte* data, UINT data_len, Buffer2000 &compressed_data)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::CompressData() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	UINT original_data_size=data_len;

	UINT compressed_length=original_data_size+8192;
	byte *compressed_bytes=new byte[compressed_length];  //an extra 8k that is more than adequate for compression headers on data that is not very compressable via lossless compression or because of small size
	
	compress2 ((Bytef *)compressed_bytes, (uLongf *)&compressed_length, (const Bytef *)data, (uLong) original_data_size, 9);
	

	compressed_data.Clear();

	//append the zlib compressed data
	compressed_data.WriteBytes(compressed_bytes,compressed_length);

	delete[] compressed_bytes;
	//log_msg.Format("ClientConnection::CompressData() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

//slowly resend the same data over and over to this client.  To pause a swarmed file indefinitely
void ClientConnection::Feed(void)
{
//	MetaSpooferReference ref;
//	CString log_msg;
//	log_msg.Format("ClientConnection::Feed() %s STARTED.",m_ip.c_str());
//	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::Feed() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	if(!m_last_feed.HasTimedOut(3) || mb_abort){  //once a sec send another byte
		//log_msg.Format("ClientConnection::Feed() %s FINISHED.",m_ip.c_str());
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
//	log_msg.Format("ClientConnection::Feed() %s FINISHED.",m_ip.c_str());
//	ref.System()->LogToFile(log_msg);
}

void ClientConnection::OnClose(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("ClientConnection::OnClose() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//TRACE("ClientConnection::OnClose() %s.\n",m_ip.c_str());
	mb_abort=true;
	if(mp_tcp_socket)
		delete mp_tcp_socket;
	mp_tcp_socket=NULL;
	log_msg.Format("ClientConnection::OnClose() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void ClientConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("ClientConnection::OnClose() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//TRACE("ClientConnection::OnReceive() %s.\n",m_ip.c_str());
	m_buffer.WriteBytes(data,length);

	log_msg.Format("ClientConnection::OnClose() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//m_idle.Refresh();
}

void ClientConnection::OnSend(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("ClientConnection::OnSend() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);

	mb_needs_send=true;
	mb_can_send=true;
	m_idle.Refresh();

	log_msg.Format("ClientConnection::OnSend() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void ClientConnection::OnConnect(CAsyncSocket* src)
{
}

void ClientConnection::GrantSlot(void)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::GrantSlot() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::GrantSlot() %s FINISHED.",m_ip.c_str());
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
	//log_msg.Format("ClientConnection::GrantSlot() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

unsigned short ClientConnection::GetListeningPort(void)
{
	return m_listening_port;
}

void ClientConnection::CheckSendBuffer(void)
{
	if( mv_send_buffers.Size()>0 && mp_tcp_socket){
		Buffer2000 *queued_data=(Buffer2000*)mv_send_buffers.Get(0);
		//MetaSpooferReference ref;
		//CString log_msg;
		//log_msg.Format("ClientConnection::CheckSendBuffer() %s STARTED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		mb_can_send=false;
		int throttle=m_throttle_speed;
		if(queued_data->GetLength()>20000)
			throttle=2000; //speed it up if there is a lot to send, otherwise emules seem to time out
		int send_stat=mp_tcp_socket->Send(queued_data->GetBufferPtr(),min((int)queued_data->GetLength(),throttle));
		//TRACE("ClientConnection::CheckSendBuffer() %s sent %d bytes.\n",m_ip.c_str(),send_stat);
		if(SOCKET_ERROR==send_stat){
		//	TRACE("ClientConnection::CheckSendBuffer() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
			//mb_abort=true;
		}
		else if(send_stat>0 && send_stat<20000000){
			queued_data->RemoveRange(0,send_stat);
		}
		if(queued_data->GetLength()==0)  //have we finished sending all this data?
			mv_send_buffers.Remove(0);
		//log_msg.Format("ClientConnection::CheckSendBuffer() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
	}
}

void ClientConnection::RespondDontHaveFile(void)
{
	//MetaSpooferReference ref;
	//CString log_msg;
	//log_msg.Format("ClientConnection::RespondDontHaveFile() %s STARTED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		//log_msg.Format("ClientConnection::RespondDontHaveFile() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
		return;
	}
	//create a packet to respond to the client
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x48); //tell this client we don't have this file anymore, this is to keep them from repeatedly asking forever and ever
	packet.WriteBytes(m_desired_hash,16);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength());
	//log_msg.Format("ClientConnection::RespondDontHaveFile() %s FINISHED.",m_ip.c_str());
	//ref.System()->LogToFile(log_msg);
}

UINT ClientConnection::Run(){
	/*
	int uPeer[4] ;
	sscanf ( m_ip.c_str(), "%d.%d.%d.%d", &uPeer[0], &uPeer[1], &uPeer[2], &uPeer[3] ) ;
	byte cPeer[4];
	cPeer[0]=uPeer[0];
	cPeer[1]=uPeer[1];
	cPeer[2]=uPeer[2];
	cPeer[3]=uPeer[3];
	*/

	//HOSTENT *ent=gethostbyaddr((char*)cPeer,4,PF_INET);
	/*
	if(ent==NULL){
		Sleep(100);//try once more just to make damn sure
		ent=gethostbyaddr((char*)cPeer,4,PF_INET);
	}
	if(ent){
		CString rdns_result=ent->h_name;
		const char* reason=IsNoSwarmAddress(rdns_result);
		TRACE("MetaMachineSpoofer::ClientConnection reverse dnsed the peer %s to the name %s.\n",m_ip.c_str(),rdns_result);
		if(reason!=NULL){
			mb_no_swarms_allowed=true;
			TRACE("MetaMachineSpoofer::ClientConnection DENYING SWARMS TO %s with rdns of %s because of %s.\n",m_ip.c_str(),rdns_result,reason);
		}
	}
	else{
		TRACE("MetaMachineSpoofer::ClientConnection failed to reverse dns the peer %s.\n",m_ip.c_str());
	}
	*/

	/*
	sockaddr_in sa;
	
	memcpy(sa.sin_addr,cPeer,4);
	host.sin_family=AF_INET;
	host.sin_port=htons(m_port);

	char host_name[2048]; //char array to store the host name into
	host_name[0]=(char)0;

	if(getnameinfo(&sa,sizeof(sa),host_name,2048,NULL,0,0)==0){
		TRACE("MetaMachineSpoofer::ClientConnection reverse dnsed the peer %s to the name %s.\n",m_ip.c_str(),host_name);
	}
	else{
		TRACE("MetaMachineSpoofer::ClientConnection failed to reverse dns the peer %s.\n",m_ip.c_str());
	}
	*/

	return 0;
}

const char* ClientConnection::IsNoSwarmAddress(const char* address)
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

void ClientConnection::ProcessEmulePacket(Buffer2000& packet)
{
	const byte* p_data=packet.GetBufferPtr();
	int buffer_offset=5;
	byte cmd=*(p_data+buffer_offset);
	buffer_offset++;

	
	if(cmd==OP_MULTIPACKET){
		MetaSpooferReference ref;

		TRACE("Client %s handling emule specific OP_MULTIPACKET.\n",m_ip.c_str());

		//debug printout
		//for(int i=0;i<(int)packet.GetLength();i++){
		//	TRACE("incoming packet[%d]=0x%x.\n",i,*((byte*)(packet.GetBufferPtr()+i)));
		//}

		//build a response or this emule ain't gonna do shit
		Buffer2000 response_packet;
		response_packet.WriteByte(0xc5);  //emule response
		response_packet.WriteDWord(0);  //specific length
		response_packet.WriteByte(OP_MULTIPACKETANSWER);
		memcpy(m_desired_hash,p_data+buffer_offset,16);

		//the hash of the file they want...

		//find this file in our system
		Vector v_tmp;
		ref.System()->GetDecoyFile(v_tmp,m_desired_hash);

		bool b_false_decoy=false;

		if(v_tmp.Size()==0)
			ref.System()->GetFalseDecoyFile(v_tmp,m_desired_hash);

		bool b_log_request=false;
		if(v_tmp.Size()!=0){
			//was a decoy that we need to report
			b_log_request=true;
		}
		if(v_tmp.Size()==0){
			ref.System()->GetSwarmFile(v_tmp,m_desired_hash);	
			if(v_tmp.Size()!=0 && mb_no_swarms_allowed){
				TRACE("MetaMachineSpoofer ClientConnection %s requesting swarm, DENIED and CLOSING CONNECTION.\n",m_ip.c_str());
				RespondDontHaveFile();
				mb_abort=true;			
				return;
			}
		}
		else{
			b_false_decoy=true;
		}

		if(v_tmp.Size()==0){
			RespondDontHaveFile();
			mb_abort=true;
			//log_msg.Format("ClientConnection::RespondHashParts() %s FINISHED.",m_ip.c_str());
			//ref.System()->LogToFile(log_msg);
			return;
		}

		//here is the file they are looking for
		DonkeyFile* the_file=(DonkeyFile*)v_tmp.Get(0);



		response_packet.WriteBytes(m_desired_hash,16);

		buffer_offset+=16;  //skip past the hash

		bool b_do_respond=false;
		
		bool b_asked_for_status=false;

		while(buffer_offset<(int)packet.GetLength()){
			byte op_code=*(p_data+buffer_offset);
			buffer_offset++;
			if(op_code==OP_REQUESTFILENAME){
				TRACE("Client %s responding to OP_MULTIPACKET->OP_REQUESTFILENAME.\n",m_ip.c_str());
				b_do_respond=true;
				if(b_log_request){
					ref.System()->LogDecoyRequest(the_file->m_file_name.c_str(),m_ip.c_str());
				}
				response_packet.WriteByte(OP_REQFILENAMEANSWER);
				//response_packet.WriteWord((WORD)/*the_file->m_file_name.size()*/file_name.size());  //write in name length TY DEBUG
				//response_packet.WriteBytes((byte*)file_name.c_str(),(UINT)file_name.size()/*the_file->m_file_name.c_str(),(UINT)the_file->m_file_name.size()*/);  //write in name
				response_packet.WriteWord((WORD)the_file->m_file_name.size());  //write in name length
				response_packet.WriteBytes((byte*)the_file->m_file_name.c_str(),(UINT)the_file->m_file_name.size());  //write in name
			}
			else if(op_code==OP_SETREQFILEID){  //they want the hashset
				TRACE("Client %s responding to OP_MULTIPACKET->OP_SETREQFILEID.\n",m_ip.c_str());
				b_do_respond=true;
				b_asked_for_status=true;
				response_packet.WriteByte(OP_FILESTATUS);
				the_file->WriteFileStatus(response_packet,b_false_decoy);
			}
			else if(op_code==OP_REQUESTSOURCES){  //here is where we blast them with whatever sources we want
				TRACE("Client %s responding to OP_MULTIPACKET->OP_REQUESTSOURCES.\n",m_ip.c_str());
			}
		}

		if(b_do_respond){

			//if(!b_asked_for_status){
				//lie  TY DEBUG
				//response_packet.WriteByte(OP_FILESTATUS);
				//response_packet.WriteWord(1);
				//response_packet.WriteByte(0x0f);
				//the_file->WriteFileStatus(response_packet,b_false_decoy);
			//}

			UINT packet_length=response_packet.GetLength();
			response_packet.WriteDWord(1,packet_length-5);  //fix the length
			//respond to this pirate
			SendData(response_packet.GetBufferPtr(),response_packet.GetLength());
			//m_send_buffer.WriteBytes(response_packet.GetBufferPtr(),response_packet.GetLength());

			//for(int i=0;i<(int)response_packet.GetLength();i++){
			//	TRACE("response packet[%d]=0x%x.\n",i,*((byte*)(response_packet.GetBufferPtr()+i)));
			//}
		}
	}
}

void ClientConnection::SendData(const byte* p_data, UINT data_length)
{
	mv_send_buffers.Add(new Buffer2000(p_data,data_length));
}
