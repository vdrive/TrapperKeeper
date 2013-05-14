#include "StdAfx.h"
#include "clientconnection.h"
#include <mmsystem.h>
#include "DonkeyFile.h"
#include "spoofingsystem.h"

#define	PARTSIZE				9728000 //hash part sizes
#define BLOCKSIZE				184320  //request block sizes

ClientConnection::ClientConnection(UINT tcp_handle,const char* ip)
{
	TRACE("New Client Connection Created %s.\n",ip);
	m_ip=ip;
	m_tcp_handle=tcp_handle;
	m_state=CLIENT_START;
	mb_abort=false;
}

ClientConnection::~ClientConnection(void)
{
	//Close the connection that this object represents
	if(m_tcp_handle!=NULL){  //
		SpooferTCPReference reference;
		reference.TCP()->CloseConnection(m_tcp_handle);
	}
}

void ClientConnection::NewData(Buffer2000& packet)
{
	TRACE("ClientConnection::NewData() %s.\n",m_ip.c_str());
	m_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::CheckBuffer(void)
{
	if(m_buffer.GetLength()<5)
		return;

	const byte* p_data=m_buffer.GetBufferPtr();
	if(p_data[0]!=0xe3 && p_data[0]!=0xc5){
		m_buffer.Clear();  //discard this nasty shit pile of queer data
		mb_abort=true;
		return;
	}

	DWORD len=m_buffer.GetDWord(1);  //get the length of the packet

	if(len>100000){  //if someone is trying to crash us with a large length drop the connection immediately
		mb_abort=true;
		m_buffer.Clear();
	}

	if(len>m_buffer.GetLength()-5){  //have we received enough of the data yet?
		return;
	}

	//len does not include the first 5 bytes of the packet

	//ok here we have a valid packet of data
	if(len<2){  //no packets should be this short lets abort
		mb_abort=true;
		m_buffer.Clear();
		return;
	}

//	byte* buffer=(byte*)m_buffer.GetBufferPtr();
	if(p_data[0]==0xc5){
		TRACE("Client Connection %s: Removing Emule Specific Packet\n",m_ip.c_str());
		m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
		return;
	}
	
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

	TRACE("Client Connection %s: Got Unknown Command 0x%x\n",m_ip.c_str(),p_data[5]);
	m_buffer.RemoveRange(0,len+5);  //lets pull this packet out of our buffer	
	mb_abort=true;
}

void ClientConnection::Update(void)
{
	CheckBuffer();
	if(m_state==CLIENT_HELLO){
		RespondHello();
	}

	if(m_state==CLIENT_FILE_STATUS){
		RespondFileStatus();
	}

	if(m_state==CLIENT_FILE_REQUEST){
		RespondFileRequest();
	}

	if(m_state==CLIENT_HASH_PARTS){
		RespondHashParts();
	}

	if(m_state==CLIENT_SEND_PARTS){
		RespondSendParts();
	}
}

void ClientConnection::RespondHello(void)
{
	TRACE("Client Connection %s: Responding Hello Packet\n",m_ip.c_str());
	UINT my_id=0;
	UINT my_port=4662;
	const char* my_name="www.shereaza.com";
	UINT my_name_length=(UINT)strlen(my_name);

	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;

	srand(timeGetTime());
	byte my_hash[16];
	for(UINT i=0;i<16;i++){  //lets make an md4 hash!  sweet.
		my_hash[i]=(byte)(rand()%255);
	}
	
	Buffer2000 packet;
	
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO RESPONSE
	packet.WriteByte(0x4c);  //Set the command byte equal to a HELLO_RESPONSE 0x4c

	packet.WriteBytes(my_hash,16);

	packet.WriteDWord(my_id);
	packet.WriteWord(my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	packet.WriteDWord(3); //3 metatags

	//encode name
	packet.WriteDWord(name_nemonic);
	packet.WriteWord(my_name_length);
	packet.WriteString(my_name);

	//encode version
	packet.WriteDWord(version_nemonic);
	packet.WriteDWord(0x01);  //lets not say we are 0x3c cus then they try to do some queer emule specific shit

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(port_nemonic);
	packet.WriteDWord(my_port);  	

	//encode the server address
	//(193.111.198.137:4242)...
	packet.WriteByte(193);
	packet.WriteByte(111);
	packet.WriteByte(198);
	packet.WriteByte(137);
	packet.WriteWord(4242);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	m_state=CLIENT_HELLO_WAIT;

	//Lets send our hello response to this pirate
	SpooferTCPReference reference;
	reference.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::RespondHashParts(void)
{
	DonkeyFile *the_file=NULL;

	MetaSpooferReference sysref;
	Vector v_tmp;
	sysref.System()->GetFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0){
		mb_abort=true;
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

	//Lets send our hello response to this pirate
	SpooferTCPReference tcp_ref;
	tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
}

bool ClientConnection::IsConnection(UINT tcp_handle)
{
	if(m_tcp_handle==tcp_handle){
		return true;
	}
	else{
		return false;
	}
}

void ClientConnection::Shutdown(void)
{
	//Close the connection that this object represents
	if(m_tcp_handle!=NULL){  //
		SpooferTCPReference reference;
		reference.TCP()->CloseConnection(m_tcp_handle);
		m_tcp_handle=NULL;
	}
}

bool ClientConnection::IsOld(void)
{
	if(mb_abort || m_created.HasTimedOut(2*60*60)){
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
	DonkeyFile *the_file=NULL;

	MetaSpooferReference sysref;
	Vector v_tmp;
	sysref.System()->GetFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0){
		mb_abort=true;
		return;
	}

	the_file=(DonkeyFile*)v_tmp.Get(0);
	
	Buffer2000 packet;
	packet.WriteByte(0xe3);
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
	packet.WriteByte(0x50);
	packet.WriteBytes(the_file->m_hash,16);
	the_file->WriteFileStatus(packet);

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length
	m_state=CLIENT_FILE_STATUS_WAIT;

	//Lets send our hello response to this pirate
	SpooferTCPReference tcp_ref;
	tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::RespondFileRequest(void)
{
	DonkeyFile *the_file=NULL;

	MetaSpooferReference sysref;
	Vector v_tmp;
	sysref.System()->GetFile(v_tmp,m_desired_hash);

	if(v_tmp.Size()==0){
		mb_abort=true;
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

	//Lets send our hello response to this pirate
	SpooferTCPReference tcp_ref;
	tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
}

void ClientConnection::RespondSendParts(void)
{

	//unfortunately we need a memory management check here, to keep us from having too many sends queued up.  (we can easily use a gig of ram without it because we don't limit connections).
	SpooferTCPReference tcp_ref;
	if(tcp_ref.TCP()->GetQueuedDataSize()>(250*(1<<20))){  //allow 250 megs to be queued up at any one time
		return;  //lets let this method continue to be called until there is room in our queue to send this data out.
	}

	m_state=CLIENT_SEND_PARTS_WAIT;
	srand(timeGetTime());
	DonkeyFile *the_file=NULL;

	MetaSpooferReference sysref;
	Vector v_tmp;
	bool is_real_file=sysref.System()->GetFile(v_tmp,m_desired_hash);

	bool is_false_decoy=false;
	HANDLE file_handle = NULL;

	if(is_real_file){
		the_file=(DonkeyFile*)v_tmp.Get(0);

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
			TRACE("Client Connection %s: Sending Parts [A] %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			TRACE("Client Connection %s: File Handle was %d.\n",file_handle);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}


		DWORD fsize=0;
		DWORD hsize=0;
		fsize=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
		
		if(fsize<6){
			TRACE("Client Connection %s: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}

		m_end1=min(fsize,m_end1);
		m_end2=min(fsize,m_end2);
		m_end3=min(fsize,m_end3);
	}

	UINT send_start;
	UINT send_end;
	int length=0;

	//check to make sure they are asking for data within a valid range
	if(!is_real_file && !is_false_decoy){
		int valid_check1=m_end1%PARTSIZE;
		int valid_check2=m_end2%PARTSIZE;
		int valid_check3=m_end3%PARTSIZE;

		//after we have sent them 3/4ths of a corrupted part, we will make them get the rest of the part form someone else to save our bandwidth.  This will result in multiple downloads of the same part from some other user, with the result of our client possibly getting banned from them (hopefully)
		if(valid_check1>(3*PARTSIZE/4) || valid_check2>(3*PARTSIZE/4) || valid_check3>(3*PARTSIZE/4)){
			TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			CloseHandle(file_handle);
			ReleaseSlot();
			return;
		}
	}
	else if(is_false_decoy){  //we always want to leave out part of the file if its bigger than PARTSIZE
		the_file=(DonkeyFile*)v_tmp.Get(0);
		if(the_file->m_size>PARTSIZE){  //only apply to large files, so that the user may never finish downloading them.  If the file is small we may as well distribute the whole file even though they will see that it is fake
			UINT part_to_leave_out=100%the_file->mv_hash_set.Size();
			//if they are asking for a block within this section, we will just release the slot and let them figure it out.
			if(m_end1>(part_to_leave_out*PARTSIZE) && m_end1<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
			if(m_end2>(part_to_leave_out*PARTSIZE) && m_end2<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
			if(m_end3>(part_to_leave_out*PARTSIZE) && m_end3<(part_to_leave_out*PARTSIZE+PARTSIZE)){
				TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d triggered release slot by false decoy!\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
				CloseHandle(file_handle);
				ReleaseSlot();
				return;				
			}
		}
	}


	if(m_end1!=0){
		length=m_end1-m_start1;
		send_start=m_start1;
		send_end=m_end1;
		if(length>2000000){
			TRACE("Client Connection %s [0]: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}

		int start=send_start;
		int chunk_size=10200;
		byte buffer[10200];
		
		int sent=0;
		int togo=length;
		if(is_real_file){
			SetFilePointer(file_handle,start,0,FILE_BEGIN);
		}

		//send out a bunch of packets for one of those things.
		while(togo>0){
			DWORD num_read=0;
			if(is_real_file){  //if it is real actually read the file info
				BOOL stat=ReadFile(file_handle,buffer,min(togo,chunk_size),&num_read,NULL);
				togo-=num_read;
				if(!stat){
					TRACE("Client Connection %s : Sending Parts %d-%d,%d-%d,%d-%d error reading file at offset %d,%d\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3,start,togo);
					mb_abort=true;
					CloseHandle(file_handle);
					return;
				}
			}
			else{ //if it is a swarm fill it with garbage data for now
				num_read=min(togo,chunk_size);
				togo-=num_read;
				for(int fake_index=0;fake_index<(int)num_read;fake_index++){
					buffer[fake_index]='X';  //generate a spoofing effect because after clients download they will recalculate their hash and this will create more hashes on the network.  
				}
			}
			Buffer2000 packet;
			packet.WriteByte(0xe3);
			packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
			packet.WriteByte(0x46);
			if(is_real_file){
				packet.WriteBytes(the_file->m_hash,16);  //write in hash
			}
			else{
				packet.WriteBytes(m_desired_hash,16);  //write in hash
			}
			//TRACE("Client Connection %s: Sent offset %d-%d\n",m_ip.c_str(),send_start+sent,send_start+sent+num_read);
			packet.WriteDWord(send_start+sent);  //write in the start offset
			packet.WriteDWord(send_start+sent+num_read);  //write in the end offset
			packet.WriteBytes(buffer,num_read);
			packet.WriteDWord(1,packet.GetLength()-5);  //fix the length
			SpooferTCPReference tcp_ref;
			tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
			sent+=num_read;
		}
	}
	if(m_end2!=0){
		length=m_end2-m_start2;
		send_start=m_start2;
		send_end=m_end2;
		if(length>2000000){
			TRACE("Client Connection %s [1]: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}

		int start=send_start;
		int chunk_size=10200;
		byte buffer[10200];
		
		int sent=0;
		int togo=length;
		if(is_real_file){
			SetFilePointer(file_handle,start,0,FILE_BEGIN);
		}

		//send out a bunch of packets for one of those things.
		while(togo>0){
			DWORD num_read=0;
			if(is_real_file){  //if it is real actually read the file info
				BOOL stat=ReadFile(file_handle,buffer,min(togo,chunk_size),&num_read,NULL);
				togo-=num_read;
				if(!stat){
					TRACE("Client Connection %s : Sending Parts %d-%d,%d-%d,%d-%d error reading file at offset %d,%d\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3,start,togo);
					mb_abort=true;
					CloseHandle(file_handle);
					return;
				}
			}
			else{ //if it is a swarm fill it with garbage data for now
				num_read=min(togo,chunk_size);
				togo-=num_read;
				for(int fake_index=0;fake_index<(int)num_read;fake_index++){
					buffer[fake_index]='X';  //generate a spoofing effect because after clients download they will recalculate their hash and this will create more hashes on the network.  
				}
			}
			Buffer2000 packet;
			packet.WriteByte(0xe3);
			packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
			packet.WriteByte(0x46);
			if(is_real_file){
				packet.WriteBytes(the_file->m_hash,16);  //write in hash
			}
			else{
				packet.WriteBytes(m_desired_hash,16);  //write in hash
			}
			//TRACE("Client Connection %s: Sent offset %d-%d\n",m_ip.c_str(),send_start+sent,send_start+sent+num_read);
			packet.WriteDWord(send_start+sent);  //write in the start offset
			packet.WriteDWord(send_start+sent+num_read);  //write in the end offset
			packet.WriteBytes(buffer,num_read);
			packet.WriteDWord(1,packet.GetLength()-5);  //fix the length
			SpooferTCPReference tcp_ref;
			tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
			sent+=num_read;
		}
	}
	if(m_end3!=0){
		length=m_end3-m_start3;
		send_start=m_start3;
		send_end=m_end3;
		if(length>2000000){
			TRACE("Client Connection %s [2]: Sending Parts %d-%d,%d-%d,%d-%d has invalid data request, the file doesn't exist or isn't that size\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3);
			mb_abort=true;
			CloseHandle(file_handle);
			return;
		}

		int start=send_start;
		int chunk_size=10200;
		byte buffer[10200];
		
		int sent=0;
		int togo=length;
		if(is_real_file){
			SetFilePointer(file_handle,start,0,FILE_BEGIN);
		}

		//send out a bunch of packets for one of those things.
		while(togo>0){
			DWORD num_read=0;
			if(is_real_file){  //if it is real actually read the file info
				BOOL stat=ReadFile(file_handle,buffer,min(togo,chunk_size),&num_read,NULL);
				togo-=num_read;
				if(!stat){
					TRACE("Client Connection %s : Sending Parts %d-%d,%d-%d,%d-%d error reading file at offset %d,%d\n",m_ip.c_str(),m_start1,m_end1,m_start2,m_end2,m_start3,m_end3,start,togo);
					mb_abort=true;
					CloseHandle(file_handle);
					return;
				}
			}
			else{ //if it is a swarm fill it with garbage data for now
				num_read=min(togo,chunk_size);
				togo-=num_read;
				for(int fake_index=0;fake_index<(int)num_read;fake_index++){
					buffer[fake_index]='X';  //generate a spoofing effect because after clients download they will recalculate their hash and this will create more hashes on the network.  
				}
			}
			Buffer2000 packet;
			packet.WriteByte(0xe3);
			packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written
			packet.WriteByte(0x46);
			if(is_real_file){
				packet.WriteBytes(the_file->m_hash,16);  //write in hash
			}
			else{
				packet.WriteBytes(m_desired_hash,16);  //write in hash
			}
			//TRACE("Client Connection %s: Sent offset %d-%d\n",m_ip.c_str(),send_start+sent,send_start+sent+num_read);
			packet.WriteDWord(send_start+sent);  //write in the start offset
			packet.WriteDWord(send_start+sent+num_read);  //write in the end offset
			packet.WriteBytes(buffer,num_read);
			packet.WriteDWord(1,packet.GetLength()-5);  //fix the length
			SpooferTCPReference tcp_ref;
			tcp_ref.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
			sent+=num_read;
		}
	}
	if(file_handle!=NULL){
		CloseHandle(file_handle);
	}
	

	//Lets send our hello response to this pirate

}
