#include "StdAfx.h"
#include "ServerObject.h"
#include "Buffer2000.h"
#include <mmsystem.h>
#include "MetaTCP.h"
#include "MetaMachineDll.h"
#include "SearchResult.h"


ServerObject::ServerObject(const char* ip,unsigned short port)
{
	m_ip=ip;
	m_port=port;
	m_tcp_handle=0;
	m_tcp_firewall_check=0;
	m_state=S_INIT;
	m_retry=0;
	m_hash_probe=0;
	mp_current_search_job=NULL;
	m_search_count=0;
	m_connected=CTime(1971,1,1,1,1,1);

	b_new_normal_data=false;
	b_new_firewall_data=false;
}

ServerObject::~ServerObject(void)
{
}

bool ServerObject::IsServer(const char* ip)
{
	if(stricmp(m_ip.c_str(),ip)==0)
		return true;
	else return false;
}

void ServerObject::Init(void)
{
	m_connected=CTime(1971,1,1,1,1,1);
	TRACE("MetaMachine:  ServerObject::Init() %s %d\n",m_ip.c_str(),m_port);
	srand(timeGetTime());

	for(UINT i=0;i<16;i++){  //lets make a md4 hash!  sweet.
		m_my_hash[i]=(byte)(rand()%255);
	}

	UINT my_id=0;
	UINT my_port=4662;
	const char* my_name="PleaseShare";
	UINT my_name_length=(UINT)strlen(my_name);

	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;

	Buffer2000 packet;
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO MOTHERFUCKER
	packet.WriteByte(0x01);  //Set the command byte equal to a HELLO

	packet.WriteBytes(m_my_hash,16);

	packet.WriteDWord(my_id);
	packet.WriteWord(my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	packet.WriteDWord(3); //3 metatags

	//encode name
	packet.WriteDWord(name_nemonic);
	packet.WriteWord(my_name_length);
	packet.WriteString(my_name);

	//encode version
	packet.WriteDWord(version_nemonic);
	packet.WriteDWord(0x3c);  //yes we are the current version of emule.  trust us.

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(port_nemonic);
	packet.WriteDWord(my_port);  	

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//Lets login to this server
	TCPReference reference;

	//close connections if they already exist.
	if(m_tcp_handle){
		reference.TCP()->CloseConnection(m_tcp_handle);
		m_tcp_handle=NULL;
	}
	if(m_tcp_firewall_check){
		reference.TCP()->CloseConnection(m_tcp_firewall_check);
		m_tcp_firewall_check=NULL;
	}
	reference.TCP()->OpenConnection(m_ip.c_str(),m_port,m_tcp_handle);
	reference.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());

	/*
	const byte *data=packet.GetBufferPtr();
	for(UINT i=0;i<packet.GetLength();i++){
		if(isalnum(data[i])){
			TRACE("ServerObject::Init sent[%d] \t = \t 0x%x \t %c \n",i,data[i],data[i]);
		}
		else{//0x%x
			TRACE("ServerObject::Init sent[%d] \t = \t 0x%x \n",i,data[i]);
		}
	}*/
}

void ServerObject::NewConnection(UINT handle)
{
	if(m_tcp_firewall_check){
		TRACE("MetaMachine:  ServerObject::NewConnection()  We already have a connection???  This isn't right.\n");
		TCPReference tcp;
		tcp.TCP()->CloseConnection(m_tcp_firewall_check);
	}
	m_tcp_firewall_check=handle;
}


//called when data has been received by the socket system
void ServerObject::NewData(UINT handle, Buffer2000 &data)
{
	if(handle==m_tcp_handle){
		CSingleLock sl(&m_tcp_buffer_lock,TRUE);
		//TRACE("Received normal data from %s.\n",m_ip.c_str());
		m_tcp_buffer_pre.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_normal_data=true;
	}
	else if(handle==m_tcp_firewall_check){
		CSingleLock sl(&m_firewall_buffer_lock,TRUE);
		//TRACE("Received firewall data from %s.\n",m_ip.c_str());
		m_firewall_buffer.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_firewall_data=true;
	}
	else{
		TRACE("Receiving data from a wierd ass handle from %s.\n",m_ip.c_str());
		CString log_msg;
		log_msg.Format("Receiving data on an invalid handle connected to %s.\n",m_ip.c_str());
		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
	}
}

//REGULAR DATA
void ServerObject::ProcessMainBuffer(void)
{
	if(m_tcp_buffer_post.GetLength()<5)
		return;

	const byte* p_data=m_tcp_buffer_post.GetBufferPtr();
	if(p_data[0]!=0xe3){
		ASSERT(0);
		m_tcp_buffer_post.Clear();  //discard this disgustingly foul data
		return;
	}

	DWORD len=m_tcp_buffer_post.GetDWord(1);  //get the length of the packet
	if(len>m_tcp_buffer_post.GetLength()-5){  //have we received enough of the data yet?
		return;
	}
}

//FIREWALL DATA
void ServerObject::ProcessFirewallBuffer(void)
{
	if(m_firewall_buffer.GetLength()<5)
		return;

	const byte* p_data=m_firewall_buffer.GetBufferPtr();
	if(p_data[0]!=0xe3){
		ASSERT(0);
		m_firewall_buffer.Clear();  //discard this disgustingly foul data
		return;
	}

	DWORD len=m_firewall_buffer.GetDWord(1);  //get the length of the packet
	if(len>m_firewall_buffer.GetLength()-5){  //have we received enough of the data yet?
		return;
	}

	if(p_data[5]==0x01 && p_data[6]==0x10){  //server is saying hello for a firewall port check
		CString log_msg;
		log_msg.Format("Server %s Responding to firewall query from %s.\n",m_ip.c_str(),m_ip.c_str());
		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);	

		UINT my_id=0;
		UINT my_port=4662;
		const char* my_name="PleaseShare";
		UINT my_name_length=(UINT)strlen(my_name);

		const UINT name_nemonic=0x01000102;
		const UINT version_nemonic=0x11000103;
		const UINT port_nemonic=0x0f000103;

		Buffer2000 packet;
		packet.WriteByte(0xe3);

		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

		//HELLO MOTHERFUCKER
		packet.WriteByte(0x4c);  //Set the command byte equal to a HELLOANSWER

		packet.WriteBytes(m_my_hash,16);

		packet.WriteDWord(my_id);
		packet.WriteWord(my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

		packet.WriteDWord(2); //2 metatags

		//encode name
		packet.WriteDWord(name_nemonic);
		packet.WriteWord(my_name_length);
		packet.WriteString(my_name);

		//encode version
		packet.WriteDWord(version_nemonic);
		packet.WriteDWord(0x3c);  //yes we are the current version of emule.  trust us.

		//now encode our server ip and port
		packet.WriteDWord(0);  //our server ip
		packet.WriteWord(0);  //our server port

		UINT packet_length=packet.GetLength();
		packet.WriteDWord(1,packet_length-5);  //fix the length

		//Lets login to this server
		TCPReference reference;

		reference.TCP()->SendData(m_tcp_firewall_check,packet.GetBufferPtr(),packet.GetLength());
	}

	//we've had our chance to analyze this packet.  lets discard it whether we read it or not.
	m_firewall_buffer.RemoveRange(0,len+5);  //remove packet data we have already analysed.
}

void ServerObject::Reset(bool b_change_state)
{
	m_connected=CTime(1971,1,1,1,1,1);
	if(m_tcp_firewall_check){
		TCPReference tcp;
		tcp.TCP()->CloseConnection(m_tcp_firewall_check);
		m_tcp_firewall_check=NULL;
	}
	if(m_tcp_handle){
		TCPReference tcp;
		tcp.TCP()->CloseConnection(m_tcp_handle);
		m_tcp_handle=NULL;
	}

	CSingleLock sl(&m_tcp_buffer_lock,TRUE);
	CSingleLock sl2(&m_firewall_buffer_lock,TRUE);
	m_tcp_buffer_post.Clear();
	m_tcp_buffer_pre.Clear();
	m_firewall_buffer.Clear();
	m_hash_probe=0;
	m_retry=0;
	m_search_count=0;

	if(b_change_state)  //to avoid an infinite recursive call from the change state method itself
		ChangeState(S_INIT);
}

void ServerObject::Search(SearchJob *new_search)
{
	mv_search_tasks.Add(new_search);
}

void ServerObject::LostConnection(UINT handle)
{
	if(handle==m_tcp_handle){
		CString log_msg;
		//log_msg.Format("LOST CONNECTION TO %s!\n",m_ip.c_str());
		//MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
		//TRACE("MetaMachine:  ServerObject::LostConnection() %s LOST it's MAIN CONNECTION!  This can't be good.\n",m_ip.c_str());
		m_tcp_handle=NULL;
		ChangeState(S_INIT);
		m_retry=0;
	}
	else if(handle==m_tcp_firewall_check){
		//TRACE("MetaMachine:  ServerObject::LostConnection() %s lost it's firewall back check!  This can't stop us now.\n",m_ip.c_str());
		m_tcp_firewall_check=NULL;
	}
}

void ServerObject::Update(void)
{
	for(UINT i=0;i<mv_search_tasks.Size();i++){
		SearchJob* sj=(SearchJob*)mv_search_tasks.Get(i);
		if(sj->IsClosed()){
			if(mp_current_search_job==sj){
				mp_current_search_job=NULL;
				m_hash_probe=0;
			}
			TRACE("MetaMachine: ServerObject::Update() Removing closed search job from ServerObject %s.\n",m_ip.c_str());
			mv_search_tasks.Remove(i);	
			i--;
		}
	}

	if(m_state==S_INIT){
		if(m_state_time.HasTimedOut(40)){  //let it wait a bit before trying to reconnect
			m_retry=0;
			//TRACE("MetaMachine:  ServerObject %s going into INIT_RESPONSE  mode.\n",m_ip.c_str());
			Init();
			ChangeState(S_INIT_RESPONSE);
		}
		return;
	}

	if(m_state==S_INIT_RESPONSE){  //we are waiting for a connection response from this server
		if(m_state_time.HasTimedOut(60)){  //once every 30 seconds we will try to reconnect
			//TRACE("MetaMachine:  ServerObject %s init timed out, going back into INIT  mode.\n",m_ip.c_str());
			ChangeState(S_INIT);
		}
		return;
	}

	if(m_state==S_READY_PAUSE){
		if(m_state_time.HasTimedOut(5)){  //allow a bit of a pause before we begin searching again
			//TRACE("MetaMachine:  ServerObject %s ready delay has completed, going to mode S_READY to do searches.\n",m_ip.c_str());
			ChangeState(S_READY);
		}
	}

	if(m_state==S_READY){  //we are ready to do a search
		if(mv_search_tasks.Size()>0){
			m_retry=0;
			//TRACE("MetaMachine:  ServerObject %s going into search mode.\n",m_ip.c_str());
			mp_current_search_job=(SearchJob*)mv_search_tasks.Get(0);
			//TRACE("ServerObject %s beginning search for project %s.\n",m_ip.c_str(),mp_current_search_job->GetProject());
			CString log_msg;
			log_msg.Format("ServerObject %s beginning search for project %s.\n",m_ip.c_str(),mp_current_search_job->GetProject());
			MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
			ChangeState(S_SEARCH);
			m_hash_probe=0;
		}
		return;
	}

	if(m_state==S_SEARCH){
		//TRACE("MetaMachine:  ServerObject %s searching, going back into SEARCH_RESPONSE  mode.\n",m_ip.c_str());
		ChangeState(S_SEARCH_RESPONSE);
		SubmitSearch();
		return;
	}

	if(m_state==S_SEARCH_RESPONSE){
		if(m_state_time.HasTimedOut(50)){  //give it 60 seconds before we just open a new connection and move onto another search
			
			AdvanceSearch();  //move on to the next search task

			TRACE("MetaMachine:  ServerObject %s search response timed out, going into S_INIT mode.\n",m_ip.c_str());
			ChangeState(S_INIT);  //give up our connection and retry
		}		
		return;
	}

	if(m_state==S_HASH_STROBE){
		TRACE("MetaMachine:  ServerObject %s querying hash %d.\n",m_ip.c_str(),m_hash_probe);
		QueryHash();  //query the current hash
		return;
	}

	if(m_state==S_HASH_STROBE_RESPONSE){
		if(m_state_time.HasTimedOut(20)){ 
			TRACE("MetaMachine:  ServerObject %s timed out on S_HASH_STROBE, going back to state S_HASH_STROBE.\n",m_ip.c_str());
			ChangeState(S_HASH_STROBE);
			m_retry++;
			if((UINT)m_retry>=mv_current_search_results.Size()){  //have we failed many times?
				AdvanceSearch();
				//TRACE("MetaMachine:  ServerObject %s timed out on S_HASH_STROBE, it has failed too many timed and is dropping back to state S_INIT.\n",m_ip.c_str());
				ChangeState(S_INIT);  //just reopen a whole new connection.
			}
		}	
		return;
	}

	if(m_state==S_HASH_STROBE_PAUSE){  //if in this state, we have gotten at least
		if(m_state_time.HasTimedOut(6)){  //take a break before strobing the next hash
			TRACE("MetaMachine:  ServerObject %s S_HASH_STROBE_PAUSE has completed, going into state S_HASH_STROBE to strobe %d.\n",m_ip.c_str(),m_hash_probe);
			ChangeState(S_HASH_STROBE);
		}	
		return;
	}
}

void ServerObject::SubmitSearch(void)
{
	SearchJob *search_job=mp_current_search_job;
	if(!search_job){
		return;
	}

	Buffer2000 packet;
	packet.WriteByte(0xe3);  //yeah we are emule.
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x16);  //Set the command byte equal to a SEARCHRQST
	packet.WriteByte(0x01);  //We are going to write a string
	
	packet.WriteWord((WORD)strlen(search_job->GetSearchString()));  //Write in the length of the string
	packet.WriteString(search_job->GetSearchString());  //Write in the string
	packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

	TCPReference reference;

	reference.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
}

void ServerObject::QueryHash(void)
{
	SearchJob *search_job=mp_current_search_job;
	if(!search_job){  //no search
		ChangeState(S_READY);  //go into a ready state and wait for a new search request
		return;
	}


	SearchResult *sr=(SearchResult*)mv_current_search_results.Get(m_hash_probe);

	if(sr==NULL){  //we are done.
		mv_current_search_results.RemoveByReference(mp_current_search_job);
		mp_current_search_job=NULL;
//		TRACE("MetaMachine:  ServerObject %s
		ChangeState(S_READY_PAUSE);  //ready for a new search.
		return;
	}

	Buffer2000 packet;
	packet.WriteByte(0xe3);  //yeah we are emule.
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x19);  //Set the command byte equal to a GETSOURCES
	packet.WriteBytes(sr->m_hash,16);
	packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

	TCPReference reference;

	ChangeState(S_HASH_STROBE_RESPONSE);
	reference.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
	m_hash_probe++;
}

void ServerObject::ChangeState(SERVERSTATE new_state)
{
	SERVERSTATE old_state=m_state;
	m_state_time.Refresh();
	m_state=new_state;

	if(m_state==S_INIT){
		Reset(false);
		//TRACE("MetaMachine:  ServerObject %s dropping back to S_INIT state from %d state.\n",m_ip.c_str(),old_state);
	}

	if(m_state==S_READY){
		mv_current_search_results.Clear();
	}
}

//checks for a specific message, and removes any messages in the buffer that aren't this specific message
bool ServerObject::HasFullResponse(byte code)
{
	if(m_tcp_buffer_post.GetLength()<6)
		return false;

	const byte* p_data=m_tcp_buffer_post.GetBufferPtr();
	if(p_data[0]!=0xe3){
		ASSERT(0);
		m_tcp_buffer_post.Clear();  //discard this disgustingly foul data
		return false;
	}

	DWORD len=m_tcp_buffer_post.GetDWord(1);  //get the length of the packet
	if(len>m_tcp_buffer_post.GetLength()-5){  //have we received enough of the data yet?
		return false;
	}

	if(p_data[5]!=code){  //is there a message that isn't the correct code?
		m_tcp_buffer_post.RemoveRange(0,len+5); //discard this message and move on.
		return HasFullResponse(code);  //recursive call.
	}

	return true;  //apparently we have all of this requested message
}

void ServerObject::ProcessSearchResults(void)
{
	m_hash_probe=0;
	SearchJob *p_search_job=mp_current_search_job;
	if(!p_search_job){
		ChangeState(S_READY);
		return;
	}	

	const byte *p_data=m_tcp_buffer_post.GetBufferPtr();
	UINT len=m_tcp_buffer_post.GetDWord(1);
	const byte *p_end=p_data+len+5;

	UINT result_count=m_tcp_buffer_post.GetDWord(6);
	p_data+=10;

	const int artist_nemonic=0x41000602;
	const int sent_nemonic=0x23000604;  //i have no idea what this one means
	const int name_nemonic=0x01000102;
	const int type_nemonic=0x03000102;
	const int description_nemonic=0x0b000102;
	const int extension_nemonic=0x04000102;
	const int length_nemonic=0x02000103;
	const int priority_nemonic=0x13000103;
	const int availability_nemonic=0x15000103;  //it gives us an IP count, but since we need the IP we can ignore this because we have to query it later.
	//need more nemonics just in case.
	bool b_ok=true;
	for(UINT i=0;i<result_count && b_ok;i++){
		SearchResult *sr=new SearchResult();
		UINT availability;

		memcpy(sr->m_hash,p_data,16);  //HASH
		p_data+=16;
		ASSERT(p_data<=p_end);  //check just in case.

		p_data+=6; //skip past the bogus clientid and port
		ASSERT(p_data<=p_end);  //check just in case.

		UINT tag_count=*((UINT*)p_data);  //how many metadata tags are attached?
		p_data+=4;
		ASSERT(p_data<=p_end);  //check just in case.
		
		int prev_tag=0;
		int last_after_tag_debug=0;
		for(UINT j=0;j<tag_count;j++){
			UINT tag_type=*((UINT*)p_data);
			p_data+=4;
			ASSERT(p_data<=p_end);  //check just in case.

			if(tag_type==name_nemonic){
				unsigned short slen=*((unsigned short*)p_data);
				p_data+=2;
				ASSERT(p_data<=p_end);  //check just in case.
				char buf[4096];
				memcpy(buf,p_data,slen);
				p_data+=slen;
				ASSERT(p_data<=p_end);  //check just in case.
				buf[slen]=(char)0;
				sr->m_file_name=buf;  //NAME
			}
			else if(tag_type==type_nemonic){
				unsigned short slen=*((unsigned short*)p_data);
				p_data+=2;
				ASSERT(p_data<=p_end);  //check just in case.
				p_data+=slen;
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if(tag_type==extension_nemonic){
				unsigned short slen=*((unsigned short*)p_data);
				p_data+=2;
				ASSERT(p_data<=p_end);  //check just in case.
				p_data+=slen;
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if(tag_type==description_nemonic){
				unsigned short slen=*((unsigned short*)p_data);
				p_data+=2;
				ASSERT(p_data<=p_end);  //check just in case.
				p_data+=slen;
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if(tag_type==length_nemonic){
				sr->m_size=*((UINT*)p_data);
				p_data+=4;
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if(tag_type==availability_nemonic){
				availability=*((UINT*)p_data);
				p_data+=4;  
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if((tag_type & 255)==2){ //we don't know what this tag is, but it appears to be some type of string.
				UINT tag_length=*((unsigned short*)(p_data-3));
				p_data+=(tag_length-1);  //skip over the tag id
				unsigned short slen=*((unsigned short*)p_data);
				p_data+=2; //skip over the length of the string
				p_data+=slen;  //skip over this freak string
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if((tag_type & 255)==3){ //we don't know what this tag is, but it appears to be some type of int.
				UINT tag_length=*((unsigned short*)(p_data-3));
				p_data+=(tag_length-1);  //skip over the tag id
				p_data+=4;  //skip over the int value whatever it is
				ASSERT(p_data<=p_end);  //check just in case.
			}
			else if((tag_type & 255)==4){ //we don't know what this tag is, but it appears to be some type of float.
				UINT tag_length=*((unsigned short*)(p_data-3));
				p_data+=(tag_length-1);  //skip over the tag id

				p_data+=4;  //skip over the float value whatever it is
				ASSERT(p_data<=p_end);  //check just in case.
			}
			/*else if(tag_type==sent_nemonic){
				p_data+=5;
				p_data+=4;
			}*/
			//else if(tag_type==priority_nemonic){
			//	p_data+=4;  //hope its a DWORD
			//}
			else{
				ASSERT(p_data<=p_end);  //check just in case.
				CString log_msg;
				log_msg.Format("WARNING:  A donkey server is feeding us an unknown tag format %d, and the previous tag format was %d.  Ignoring remaining search results since the parser doesn't know how to continue.\n",tag_type,prev_tag);
				TRACE((LPCSTR)log_msg);
				MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
				UINT tag_length=*((unsigned short*)(p_data-3));
				p_data+=(tag_length-1);  //skip over the tag, hopefully it doesn't have extra data
				last_after_tag_debug=*((int*)p_data);
				ASSERT(p_data<=p_end);  //check just in case.
				//p_data+=4;  //hope its a DWORD
//				b_ok=false;
				//ASSERT(0);
			}

			prev_tag=tag_type;
		}

		mv_current_search_results.Add(sr);
		if(!p_search_job->IsClosed()){
			//is it a spoof?
			if((((sr->m_hash[13]>>4)&0x0f)+(sr->m_hash[13]&0x0f))==15 && (((sr->m_hash[14]>>4)&0x0f)+(sr->m_hash[14]&0x0f))==15 && (((sr->m_hash[15]>>4)&0x0f)+(sr->m_hash[15]&0x0f))==15)  
				continue;//yes it was a spoof.
			if(p_search_job->AddResult(sr)){
				p_search_job->AddIP(0,sr,availability);
			}
		}
	}

	//p_search_job->SortResults();

	if((*p_data)==0x01 && m_search_count<4){  //is there more?  we want as many as possible, but we run into extremely diminishing returns on projects like eminem where it keeps returning more forever.
		m_search_count++;
		CString log_msg;
		log_msg.Format("Donkey server %s reports it has MORE results on project %s after giving %d results, continuing search.\n",m_ip.c_str(),p_search_job->GetProject(),result_count);
		//TRACE("Donkey server %s reports it has MORE results on project %s after giving %d results, continuing search.\n",m_ip.c_str(),p_search_job->GetProject(),result_count);
		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
		Buffer2000 packet;
		packet.WriteByte(0xe3);  //yeah we are emule.
		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

		packet.WriteByte(0x21);  //Set the command byte equal to a MORE RESULTS
		packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

		TCPReference reference;

		reference.TCP()->SendData(m_tcp_handle,packet.GetBufferPtr(),packet.GetLength());
		this->m_state_time.Refresh();  //restart the result timer.
	}
	else{  //no more search results
		AdvanceSearch();
		#ifdef LOGIPS
			ChangeState(S_HASH_STROBE);
		#else
			ChangeState(S_READY_PAUSE);
		#endif
	}

	m_tcp_buffer_post.RemoveRange(0,len+5);
	//TRACE("MetaMachine:  ServerObject::ProcessSearchResults() length of tcp buffer after search is %d, length of results was %d.\n",m_tcp_buffer_post.GetLength(),len);
}

//checks received data for hash queries, and adds them to the result sets of all our jobs.
//function is responsible for building ip sets
void ServerObject::ProcessHashQueryResponse(void)
{
	/*
	if(m_tcp_buffer.GetLength()<6)
		return;

	UINT index=0;
	while(index<m_tcp_buffer.GetLength()){
		const byte* p_data=m_tcp_buffer.GetBufferPtr();
		if(p_data[index+0]!=0xe3){
			ASSERT(0);
			m_tcp_buffer.Clear();  //discard this disgustingly foul data
			return;
		}

		DWORD len=m_tcp_buffer.GetDWord(index+1);  //get the length of the packet
		if(len>m_tcp_buffer.GetLength()-5-index){  //have we received enough of the data yet?
			return;
		}

		if(p_data[index+5]!=0x42){  //is there a message that isn't the correct code?
			index+=5;
			index+=len;
		}
		else{
			p_data=m_tcp_buffer.GetBufferPtr()+index;
			p_data+=6;
			byte hash[16];
			memcpy(hash,p_data,16);
			p_data+=16;
			UINT count=(UINT)(*p_data);  //number of ips as a byte
			TRACE("Adding %d sources.\n",count);
			p_data++;
			for(UINT j=0;j<count;j++){
				UINT ip=*((UINT*)p_data);
				p_data+=4;
				p_data+=2;  //skip the port, we don't care about that at this point.

				MetaSystemReference ref;
				ref.System()->NewSupplySource(hash,ip);
			}

			m_tcp_buffer.RemoveRange(index,5+len);  //extract this packet from our buffer.
			ChangeState(S_HASH_STROBE_PAUSE);  //go into a paused state while more data possibly streams in.
		}
	}
	*/
}

void ServerObject::ClearUnwantedPackets(void)
{
	if(m_tcp_buffer_post.GetLength()<6)
		return;

	UINT index=0;
	while(index<m_tcp_buffer_post.GetLength()){
		const byte* p_data=m_tcp_buffer_post.GetBufferPtr();
		if(p_data[index+0]!=0xe3){  //all packets had better start with this
			byte b=p_data[index+0]; //debug
			ASSERT(0);
			m_tcp_buffer_post.Clear();  //discard this disgustingly foul data
			return;
		}

		DWORD len=m_tcp_buffer_post.GetDWord(index+1);  //get the length of the packet
		if(len>m_tcp_buffer_post.GetLength()-5-index){  //have we received enough of the data yet?
			return;
		}

		if(p_data[index+5]==0x38){  //is there a message that isn't the correct code?
			m_tcp_buffer_post.RemoveRange(index,5+len);  //extract this packet from our buffer.
			//TRACE("MetaMachine:  ServerObject %s removing unwanted server message (0x38) from its buffer.  New buffer length is %d.\n",m_ip.c_str(),m_tcp_buffer_post.GetLength());
		}
		else{ //leave any others
			index+=5;
			index+=len;
		}
	}
}

void ServerObject::AdvanceSearch(void)
{
	if(mp_current_search_job!=NULL){
		CString log_msg;
		log_msg.Format("Donkey server %s finished searching on project %s.\n",m_ip.c_str(),mp_current_search_job->GetProject());
		//TRACE(log_msg);

		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
	}
	mp_current_search_job=NULL;
	m_hash_probe=0;
	m_search_count=0;
	if(mv_search_tasks.Size()>0){
		mv_search_tasks.Remove(0);  //remove this search job since it isn't generating any info
	}
}

CTime ServerObject::ConnectionTime(void)
{
	return m_connected;
}

UINT ServerObject::Run(){
	while(!this->b_killThread){
		m_infinite_loop_timer.Refresh();
		//process any received socket data that may have come in.
		if(b_new_normal_data){
			{//scoping block so we can do a quick lock while we transfer bytes into our analysis buffer
				CSingleLock sl(&m_tcp_buffer_lock,TRUE);
				m_tcp_buffer_post.WriteBytes(m_tcp_buffer_pre.GetBufferPtr(),m_tcp_buffer_pre.GetLength());
				m_tcp_buffer_pre.Clear();
			}
			ClearUnwantedPackets();
			ProcessHashQueryResponse();  //always check for some source queries, because we never know when we will receive them.
			if(m_state==S_INIT_RESPONSE){
				m_connected=CTime::GetCurrentTime();
				CString log_msg;
				ChangeState(S_READY_PAUSE);
				//TRACE("MetaMachine:  ServerObject %s going into S_READY_PAUSE for search mode.\n",m_ip.c_str());
				log_msg.Format("Connection Established to %s.\n",m_ip.c_str());
				MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
			}
			if(m_state==S_SEARCH_RESPONSE){
				//The server has responded to our search query
				m_hash_probe=0;
				
				//TRACE("MetaMachine:  Received Normal Data in S_SEARCH_RESPONSE MODE.  Checking if packet 0x33 exists.\n");
				if(HasFullResponse(0x33)){  //looks for our search results, and clears garbage packets out before it.  the method of keeping the buffer from building up infinitely large with extraneous messages that we don't seem to have a use for.
					//TRACE("MetaMachine:  Received Normal Data in S_SEARCH_RESPONSE MODE.  Packet 0x33 does exist.  Analyzing.\n");	
					ProcessSearchResults();
					//TRACE("MetaMachine:  Received Normal Data in S_SEARCH_RESPONSE MODE.  Done analyzing packet, going into S_HASH_STROBE mode to query sources.\n");	
				}
			}
			b_new_normal_data=false;
		}
		else if(b_new_firewall_data){
			CSingleLock sl(&m_firewall_buffer_lock,TRUE);
			ProcessFirewallBuffer();
			b_new_firewall_data=false;
		}

		Update();  //update the state machine

		Sleep(50);
	}

	return 0;
}

bool ServerObject::IsThreadLockedUp(void)
{
	if(m_infinite_loop_timer.HasTimedOut(180)){
		return true;
	}
	else return false;
}
