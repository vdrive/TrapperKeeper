#include "StdAfx.h"
#include "interdictionconnection.h"
#include "SpoofingSystem.h"
#include "..\tkcom\PeerSocket.h"
//#include "MetaSpooferTCP.h"

#define	PARTSIZE				9728000 //hash part sizes
#define BLOCKSIZE				184320  //request block sizes

InterdictionConnection::InterdictionConnection(const char *my_name,byte *my_hash, unsigned short my_port,const char *target_hash,const char* target_ip, unsigned short target_port,const char* project)
//: m_tcp_connection(target_ip,target_port)
{
	
	mp_tcp_socket=NULL;

	m_ip=target_ip;
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::InterdictionConnection() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	m_port=target_port;

	m_my_id=16777217+(int)rand();
	m_my_version=20+(rand()%5);
	mb_awaiting_callback=false;
	mb_abort=false;
	mb_ready_to_download=false;
	//m_tcp_handle=3500000000;  //some impossible number
	//m_kill_after_idle_time=60*60;
	m_project=project;

	m_my_user_name=my_name;
	memcpy(m_my_hash,my_hash,16);
	//we don't actually wan't to be mldonkey just now...
	for(int i=0;i<16;i++){
		if(m_my_hash[i]=='M')
			m_my_hash[i]=15;
		if(m_my_hash[i]=='L')
			m_my_hash[i]=29;
	}

	m_my_port=my_port;
	m_str_target_hash=target_hash;
	//m_ip=target_ip;
	//m_port=target_port;
	m_amount_data_requested=0;
	m_amount_data_sent=0;
	m_total_downloaded=0;

	mb_awaiting_callback=false;

	for(int j=0;j<16;j++){
		char ch1=target_hash[j*2];
		char ch2=target_hash[j*2+1];
		byte val1=ConvertCharToInt(ch1);
		byte val2=ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		m_target_hash[j]=hash_val;
	}					

	log_msg.Format("InterdictionConnection::InterdictionConnection() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

InterdictionConnection::~InterdictionConnection(void)
{
	if(mp_tcp_socket)
		delete mp_tcp_socket;

	//TRACE("MetaMachineSpoofer InterdictionConnection::~InterdictionConnection().\n");
}

void InterdictionConnection::Update()
{

}	

void InterdictionConnection::CheckBuffer(void)
{
	if(m_buffer.GetLength()<5 || mb_abort || mp_tcp_socket==NULL)
		return;

	const byte* p_data=m_buffer.GetBufferPtr();
	if(p_data[0]!=0xe3 && p_data[0]!=0xc5){
		TRACE("Interdiction Connection %s: Received unknown pack with header 0x%x\n",m_ip.c_str(),p_data[0]);
		m_buffer.Clear();  //discard this data as its not something we understand at this point
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

	//len does not include the first 5 bytes of the packet

	//ok here we have a valid packet of data
	if(len<1){  //no packets should be this short lets abort
		TRACE("Interdiction Connection %s: Received packet with too short a length of %d, aborting connection\n",mp_tcp_socket->GetIP(),len);
		mb_abort=true;
		m_buffer.Clear();
		return;
	}

//	byte* buffer=(byte*)m_buffer.GetBufferPtr();
	if(p_data[0]==0xc5){
		TRACE("Interdiction Connection %s: Removing Emule Specific Packet\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		return;
	}

	if(mb_awaiting_callback && len>=4 && p_data[5]==0x01 && p_data[6]==0x10){  //client sends hello
		//TRACE("Interdiction Connection %s hash %s: Got Hello Packet for a callback connection\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
		mb_awaiting_callback=false;
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondHello();
		m_kill_after_idle_time=60*5;
		return;
	}


	//TRACE("Interdiction Connection %s: Received command byte 0x%x with len=%d\n\n",mp_tcp_socket->GetIP(),p_data[5],len);

	if(p_data[5]==0x59){  //they have the file
		//TRACE("Interdiction Connection %s: The client has the file we are looking for\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		AskForFileStatus();
		m_kill_after_idle_time=60*3;
		return;
	}
	if(p_data[5]==0x48){  //they don't have the file
		//TRACE("Interdiction Connection %s: Client doesn't have the file we are looking for\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		MetaSpooferReference ref;
		ref.System()->RemoveSourceForHash(m_str_target_hash.c_str(),m_ip.c_str(),m_port);
		mb_abort=true;
		return;
	}
	if(p_data[5]==0x58){  //they don't have the file
		//TRACE("Interdiction Connection %s: Client doesn't have the file we are looking for\n",mp_tcp_socket->GetIP());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RespondDontHaveFile();
		return;
	}
	if(p_data[5]==0x4c){  //hello answer
		//TRACE("Interdiction Connection %s: Got Hello Answer\n",mp_tcp_socket->GetIP());
		byte *the_data=(byte*)p_data+5;
		//extract the clients name so we can steal it
		const UINT name_nemonic=0x01000102;
		for(int j=0;j<(int)(len-4);j++){
			if(*((UINT*)(++the_data))==name_nemonic){
				the_data+=4;
				WORD name_len=*((WORD*)the_data);
				the_data+=2;
				CString the_name;
				for(int k=0;k<name_len;k++){
					the_name+=*(the_data+k);
				}
				MetaSpooferReference ref;
				if(the_name.GetLength()>0){
					ref.System()->AddUserName(the_name);
				}
				break;
			}
		}
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		CheckIfHasFile();
		m_kill_after_idle_time=60*3;
		return;
	}
	if(p_data[5]==0x46){  //received file data
		//TRACE("Interdiction Connection %s: Received File Data for hash %s\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
		byte *the_data_ptr=(byte*)(p_data+5);
		the_data_ptr+=1;
		the_data_ptr+=16;  //skip the hash;
		UINT data_start=*(UINT*)the_data_ptr;
		the_data_ptr+=4;
		UINT data_end=*(UINT*)the_data_ptr;

		int data_len=data_end-data_start;
		m_amount_data_sent+=data_len;

		m_total_downloaded+=data_len;
		
		if(m_amount_data_sent>=m_amount_data_requested){
			if(m_total_downloaded>=PARTSIZE){
				ReleaseSlot();
				m_kill_after_idle_time=60;
				mb_abort=true;
			}
			else{
				RequestParts();
			}
		}

		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		
		return;
	}
	if(p_data[5]==0x50 && len>18){  //file status we asked for
		//TRACE("Interdiction Connection %s: Received File Status for hash %s\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
		byte *the_data_ptr=(byte*)p_data+6;
		the_data_ptr+=16;  //skip the hash;
		WORD num_parts=*(WORD*)the_data_ptr;
		the_data_ptr+=2;
		mv_has_parts.clear();  //in case they send status twice or something wierd
		if(num_parts>0){
			for(int i=0;i<=num_parts/8 && i<(int)(len-19);i++){
				byte the_byte=*(the_data_ptr+i);
				for(int j=0;j<8;j++){  //see which parts of the file this person has
					int res=1;
					for(int x=0;x<j;x++){  //calculate 2 to the j
						res*=2;
					}
					if((the_byte & res)>0){
						mv_has_parts.push_back(i*8+j);
					}
				}
			}
		}
		
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		
		AskForSlot();
		//RequestParts();
		mb_awaiting_callback=true;  //set this to true since we will likely recieve an OnClose event in about 30 seconds, but this doesn't mean anything bad, they just put us in their queue
		m_kill_after_idle_time=60*60*5;  //wait in queue up to 5 hours before we give up
		return;
	}
	if(p_data[5]==0x55){
		//TRACE("Interdiction Connection %s:  Download slot granted for hash %s\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		RequestParts();
		m_kill_after_idle_time=60*10;  //allow 10 minutes to receive data, this is probably too long
		return;
	}
	if(p_data[5]==0x57){
		//TRACE("Interdiction Connection %s:  Download slot denied for hash %s, closing connection\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
		mb_abort=true;
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		return;
	}
	/*


	
	TRACE("Client Connection %s: Received command byte 0x%x with len=%d\n\n",m_ip.c_str(),p_data[5],len);
	if(p_data[5]==0x01 && p_data[6]==0x10){
		TRACE("Client Connection %s: Got Hello Packet\n",m_ip.c_str());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_HELLO;
		return;
	}
	if(p_data[5]==0x4f && len>=1+16){ //file status request
		TRACE("Client Connection %s: Got File Status Request\n",m_ip.c_str());
		memcpy(m_desired_hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_FILE_STATUS;
		return;
	}
	if(p_data[5]==0x58 && len>=1+16){ //file status request
		TRACE("Client Connection %s: Got File Request\n",m_ip.c_str());
		memcpy(m_desired_hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_FILE_REQUEST;
		return;
	}
	
	if(p_data[5]==0x51 && len>=1+16){ //file status request
		TRACE("Client Connection %s: Got File Request\n",m_ip.c_str());
		memcpy(m_desired_hash,p_data+6,16);
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		m_state=CLIENT_HASH_PARTS;
		return;
	}
	
	if(p_data[5]==0x54 && len>=1){ //slot request
		TRACE("Client Connection %s: Got Slot Request\n",m_ip.c_str());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	

		//create a packet to respond to the client
		Buffer2000 packet;
		packet.WriteByte(0xe3);
		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
		packet.WriteByte(0x55); //grant the slot
		UINT packet_length=packet.GetLength();
		packet.WriteDWord(1,packet_length-5);  //fix the length
		SpooferTCPReference tcp_ref;
		tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
		return;
	}

	if(p_data[5]==0x47 && len>=1+16+24){ //slot request
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
		return;
	}

	*/

	TRACE("Interdiction Connection %s: Got Unknown Command 0x%x, discarding it\n",mp_tcp_socket->GetIP(),p_data[5]);
	m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
}

bool InterdictionConnection::IsDead(void)
{
	if(mb_abort || m_idle_timer.HasTimedOut(m_kill_after_idle_time)){
		return true;
	}
	else
		return false;
}

void InterdictionConnection::AskForFileStatus(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::AskForFileStatus() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		log_msg.Format("InterdictionConnection::AskForFileStatus() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	//TRACE("Interdiction Connection %s hash %s: AskForFileStatus()\n",m_ip.c_str(),m_str_target_hash.c_str());
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x4f);
	packet.WriteBytes(m_target_hash,16);  //write in hash

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets send our file status request to this pirate to see if they still have the file or not
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::AskForFileStatus() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::AskForFileStatus() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void InterdictionConnection::RequestParts(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::RequestParts() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		log_msg.Format("InterdictionConnection::RequestParts() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	m_amount_data_requested=0;
	//TRACE("Interdiction Connection %s hash %s: RequestParts()\n",m_ip.c_str(),m_str_target_hash.c_str());
	//int amount_to_request=BLOCKSIZE;  //request block sizes
	int start=(rand()%2)*BLOCKSIZE;
	if(mv_has_parts.size()>1){
		int rand_index=rand()%((int)mv_has_parts.size()-1);  //avoid asking for something within the last part (the reason for the -1)
		int part_to_get=mv_has_parts[rand_index];
		start=part_to_get*PARTSIZE;
		start+=(rand()%2)*BLOCKSIZE;
	}

	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x47);
	packet.WriteBytes(m_target_hash,16);  //write in hash

	//write in 3 starts
	packet.WriteDWord(start);
	packet.WriteDWord(start+10200);
	packet.WriteDWord(start+2*10200);

	//write in 3 ends
	packet.WriteDWord(start+10200);
	packet.WriteDWord(start+2*10200);
	packet.WriteDWord(start+3*10200);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets send our file request to this pirate to see if they still have the file or not
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::RequestParts() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::RequestParts() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void InterdictionConnection::CheckIfHasFile(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::CheckIfHasFile() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;

		log_msg.Format("InterdictionConnection::CheckIfHasFile() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	//TRACE("Interdiction Connection %s hash %s: CheckIfHasFile()\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x58);
	packet.WriteBytes(m_target_hash,16);  //write in hash

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets send our file request to this pirate to see if they still have the file or not
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::CheckIfHasFile() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::CheckIfHasFile() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void InterdictionConnection::AskForSlot(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::AskForSlot() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;

		log_msg.Format("InterdictionConnection::AskForSlot() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	//TRACE("Interdiction Connection %s hash %s: AskForSlot()\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x54);
	packet.WriteBytes(m_target_hash,16);  //write in hash

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets ask this pirate if they have a slot for us
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::AskForSlot() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::AskForSlot() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());

}

void InterdictionConnection::Init(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::Init() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	m_idle_timer.Refresh();
	//TRACE("Interdiction Connection %s hash %s: Init()\n",m_ip.c_str(),m_str_target_hash.c_str());
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->OpenConnection(m_tcp_connection);

	//TRACE("Client Connection %s: Sending Hello Packet to interdict hash %s\n",m_ip.c_str(),m_str_target_hash.c_str());
	//UINT my_port=4661;
	
	//srand(timeGetTime());

	MetaSpooferReference sysref;
	const char* my_name=m_my_user_name.c_str();	

	UINT my_name_length=(UINT)strlen(my_name);

	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;
	
	//Buffer2000 packet;
	
	m_init_buffer.WriteByte(0xe3);

	m_init_buffer.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO
	m_init_buffer.WriteByte(0x01);  //Set the command byte
	m_init_buffer.WriteByte(0x10);  //Set the command byte part 2

	m_init_buffer.WriteBytes(m_my_hash,16);

	m_init_buffer.WriteDWord(m_my_id);
	m_init_buffer.WriteWord(m_my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	m_init_buffer.WriteDWord(3); //3 metatags

	//encode name
	m_init_buffer.WriteDWord(name_nemonic);
	m_init_buffer.WriteWord(my_name_length);
	m_init_buffer.WriteString(my_name);

	//encode version
	m_init_buffer.WriteDWord(version_nemonic);
	m_init_buffer.WriteDWord(m_my_version);  //lets not say we are 0x3c cus then they try to do some queer emule specific shit

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	m_init_buffer.WriteDWord(port_nemonic);
	m_init_buffer.WriteDWord(m_my_port);  	

	//encode the server address
	//(193.111.198.137:4242)...
	m_init_buffer.WriteByte(66);
	m_init_buffer.WriteByte(54);
	m_init_buffer.WriteByte(78);
	m_init_buffer.WriteByte(14);
	m_init_buffer.WriteWord(4321);

	UINT packet_length=m_init_buffer.GetLength();
	m_init_buffer.WriteDWord(1,packet_length-5);  //fix the length

	//send the packet
	if(mp_tcp_socket)
		delete mp_tcp_socket;
	mp_tcp_socket=new PeerSocket();
	mp_tcp_socket->Init(m_ip.c_str(),m_port,this);

	log_msg.Format("InterdictionConnection::Init() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
	//m_state=CLIENT_HELLO_WAIT;

}

bool InterdictionConnection::IsConnection(const char* ip)
{
	return stricmp(ip,m_ip.c_str())==0;
}

void InterdictionConnection::ReleaseSlot(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::ReleaseSlot() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		log_msg.Format("InterdictionConnection::ReleaseSlot() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	//TRACE("Interdiction Connection %s hash %s: ReleaseSlot()\n",mp_tcp_socket->GetIP(),m_str_target_hash.c_str());
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x56);
	//packet.WriteBytes(m_target_hash,16);  //write in hash

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_state=CLIENT_FILE_REQUEST_WAIT;

	//Lets release our slot
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::ReleaseSlot() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::ReleaseSlot() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference tcp_ref;
	//tcp_ref.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

void InterdictionConnection::Shutdown(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::Shutdown() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(mp_tcp_socket!=NULL){
		mp_tcp_socket->Close();
		delete mp_tcp_socket;
		mp_tcp_socket=NULL;
	}

//	if(m_tcp_connection.GetHandle()!=NULL){  //
//		SpooferTCPReference reference;
//		reference.TCP()->CloseConnection(m_tcp_connection);

	if(m_total_downloaded>0){
		MetaSpooferReference ref;
		ref.System()->LogInterdiction(m_project.c_str(),m_str_target_hash.c_str(),m_ip.c_str(),m_port,m_total_downloaded);
		m_total_downloaded=0;
	}
//		m_tcp_connection.SetHandle(NULL);
//	}

	log_msg.Format("InterdictionConnection::Shutdown() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void InterdictionConnection::RespondHello(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::RespondHello() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mp_tcp_socket){
//		ASSERT(0);
		mb_abort=true;
		log_msg.Format("InterdictionConnection::RespondHello() %s FINISHED.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		return;
	}
	//TRACE("Interdiction Connection %s hash %s: RespondHello()\n",m_ip.c_str(),m_str_target_hash.c_str());
	
	//UINT my_port=4661;
	
	//srand(timeGetTime());

	MetaSpooferReference sysref;
	const char* my_name=m_my_user_name.c_str();	

	UINT my_name_length=(UINT)strlen(my_name);

	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;
	
	Buffer2000 packet;
	
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO RESPONSE
	packet.WriteByte(0x4c);  //Set the command byte equal to a HELLO_RESPONSE 0x4c

	packet.WriteBytes(m_my_hash,16);

	packet.WriteDWord(m_my_id);
	packet.WriteWord(m_my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	packet.WriteDWord(3); //3 metatags

	//encode name
	packet.WriteDWord(name_nemonic);
	packet.WriteWord(my_name_length);
	packet.WriteString(my_name);

	//encode version
	packet.WriteDWord(version_nemonic);
	packet.WriteDWord(m_my_version);  //lets not say we are 0x3c cus then they try to do some queer emule specific shit

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(port_nemonic);
	packet.WriteDWord(m_my_port);  	

	//encode the server address
	//(193.111.198.137:4242)...
	packet.WriteByte(66);
	packet.WriteByte(54);
	packet.WriteByte(78);
	packet.WriteByte(14);
	packet.WriteWord(4321);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//m_state=CLIENT_HELLO_WAIT;

	//Lets send our hello response to this pirate
	if(SOCKET_ERROR==mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
		TRACE("InterdictionConnection::ReleaseSlot() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::RespondHello() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//SpooferTCPReference reference;
	//reference.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

bool InterdictionConnection::IsAwaitingCallBack(void)
{
	return mb_awaiting_callback;
}

void InterdictionConnection::ReestablishedConnection(CAsyncSocket *new_con)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::ReestablishedConnection() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	m_idle_timer.Refresh();
	m_kill_after_idle_time=60*5;
	//TRACE("Interdiction Connection %s hash %s: ReestablishedConnection()\n",m_ip.c_str(),m_str_target_hash.c_str());
	if(mp_tcp_socket) delete mp_tcp_socket;

	mp_tcp_socket=(PeerSocket*)new_con;
	mp_tcp_socket->SetSocketEventListener(this);

	log_msg.Format("InterdictionConnection::ReestablishedConnection() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//m_tcp_connection.SetHandle(handle);
}

const char* InterdictionConnection::GetIP(void)
{
	return m_ip.c_str();
}

void InterdictionConnection::OnClose(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::OnClose() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(!mb_awaiting_callback)
		mb_abort=true;

	delete mp_tcp_socket;
	mp_tcp_socket=NULL;
	MetaSpooferReference sys;
	if(mb_awaiting_callback && !mb_abort){
		sys.System()->SwitchInterdictionConnectionToInactive(this);
	}

	log_msg.Format("InterdictionConnection::OnClose() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//mb_abort=true;
}

void InterdictionConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::OnReceive() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//TRACE("Interdiction Connection %s hash %s: OnReceive()\n",m_ip.c_str(),m_str_target_hash.c_str());
	if(!mb_abort){
		m_idle_timer.Refresh();
		m_buffer.WriteBytes(data,length);
		CheckBuffer();
	}

	log_msg.Format("InterdictionConnection::OnReceive() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void InterdictionConnection::OnSend(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::OnSend() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//if(stricmp(mp_tcp_socket->GetIP() , "66.54.78.16")==0){
		//TRACE("InterdictionConnection::OnSend() to 66.54.78.12\n");
	//}
//	mb_needs_send=true;
	m_idle_timer.Refresh();

	log_msg.Format("InterdictionConnection::OnSend() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void InterdictionConnection::OnConnect(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("InterdictionConnection::OnConnect() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	if(m_init_buffer.GetLength()>0){
		if(SOCKET_ERROR==mp_tcp_socket->Send(m_init_buffer.GetBufferPtr(),m_init_buffer.GetLength(),0))
			mb_abort=true;
	}

	log_msg.Format("InterdictionConnection::OnConnect() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void InterdictionConnection::OnConnectFailed(CAsyncSocket* src)
{
	mb_abort=true;
}
void InterdictionConnection::RespondDontHaveFile(void)
{
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
	packet.WriteBytes(m_target_hash,16);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	//m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());

	mp_tcp_socket->Send(packet.GetBufferPtr(),packet.GetLength());
}
