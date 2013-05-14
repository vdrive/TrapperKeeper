#include "StdAfx.h"
#include "SpoofHost.h"
#include "Buffer2000.h"
#include <mmsystem.h>
//#include "MetaSpooferTCP.h"
#include "MetaMachineSpooferDll.h"
//#include "MetaMachineDll.h"
#include "..\tkcom\tinysql.h"
#include "DonkeyFile.h"
#include "Spoof.h"
#include "Swarm.h"
#include "zlib.h"
#include "MetaSpooferUtilityFunctions.h"

//#define	PARTSIZE				9728000 //hash part sizes
//#define BLOCKSIZE				184320  //request block sizes


/*
vector <string> v_master_result_set_col0;
vector <string> v_master_result_set_col1;
vector <string> v_master_result_set_col2;
vector <string> v_master_result_set_col3;
vector <string> v_master_result_set_col4;

Timer last_result_set_update;
bool b_updating_result_set=false;
bool b_first_update=false;
*/
#define	TAGTYPE_HASH			0x01
#define	TAGTYPE_STRING			0x02
#define	TAGTYPE_UINT32			0x03
#define	TAGTYPE_FLOAT32			0x04
#define	TAGTYPE_BOOL			0x05
#define	TAGTYPE_BOOLARRAY		0x06
#define	TAGTYPE_BLOB			0x07
#define	TAGTYPE_UINT16			0x08
#define	TAGTYPE_UINT8			0x09

#define TAGTYPE_STR1			0x11
#define TAGTYPE_STR2			0x12
#define TAGTYPE_STR3			0x13
#define TAGTYPE_STR4			0x14
#define TAGTYPE_STR5			0x15
#define TAGTYPE_STR6			0x16
#define TAGTYPE_STR7			0x17
#define TAGTYPE_STR8			0x18
#define TAGTYPE_STR9			0x19
#define TAGTYPE_STR10			0x1A
#define TAGTYPE_STR11			0x1B
#define TAGTYPE_STR12			0x1C
#define TAGTYPE_STR13			0x1D
#define TAGTYPE_STR14			0x1E
#define TAGTYPE_STR15			0x1F
#define TAGTYPE_STR16			0x20
#define TAGTYPE_STR17			0x21	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all
#define TAGTYPE_STR18			0x22	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all
#define TAGTYPE_STR19			0x23	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all
#define TAGTYPE_STR20			0x24	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all
#define TAGTYPE_STR21			0x25	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all
#define TAGTYPE_STR22			0x26	// accepted by eMule 0.42f (02-Mai-2004) in receiving code only because of a flaw, those tags are handled correctly, but should not be handled at all


SpoofHost::SpoofHost(const char* ip,unsigned short port, unsigned short my_port,byte *my_hash, string my_user_name,UINT connection_delay)
//: m_tcp_connection(ip,port), m_firewall_connection(ip,0)
{
	m_search_count=0;
	m_file_limit=1000;
	m_ip=ip;
	m_port=port;
	m_my_port=my_port;
	memcpy(m_my_hash,my_hash,16);
	m_my_user_name=my_user_name;
	m_connection_delay=connection_delay;

	m_state=S_INIT;
	m_retry=0;
	m_connected=CTime(1971,1,1,1,1,1);

	b_new_normal_data=false;
	b_new_firewall_data=false;

	mb_active_server=true;
	m_uploaded_files=0;

	respoof_time=60*60*24+(rand()%(60*60));
	m_get_sources_delay=60*45+rand()%7200;
	//m_get_sources_delay=180;
	b_down=false;  //down or up;   flag signaling whether we wish to remain disconnected for now.

	mp_main_connection=NULL;
	mp_firewall_connection=NULL;
	m_expiration_time=60*60*24+(rand()%7200);
	
	if(TYDEBUGMODE)
		m_search_delay=60*4+(rand()%240);  //TYDEBUG  //a search period of X minutes
	else
		m_search_delay=60*60+(rand()%3600);  //TYDEBUG  //a search period of X minutes

	m_dead_time=60*60*4+(rand()%7200);  //stay dead a random amount of time

	m_emule_version=44160;
}

SpoofHost::~SpoofHost(void)
{
	if(mp_main_connection) delete mp_main_connection;
	if(mp_firewall_connection) delete mp_firewall_connection;
}

//to identify by tcp handle
bool SpoofHost::IsServer(const char* ip)
{
	return stricmp(m_ip.c_str(),ip) == 0;
}

//to identify by server handle
bool SpoofHost::IsServer(const char* ip, unsigned short listen_port)
{
	bool stat1=(stricmp(m_ip.c_str(),ip) == 0);
	bool stat2=(m_my_port==listen_port);
	bool stat3 = stat1 && stat2;
	return ( stat3 );
}


void SpoofHost::Init(void)
{
	m_uploaded_files=0;
	m_connected=CTime(1971,1,1,1,1,1);
	TRACE("MetaMachineSpoofer:  SpoofHost::Init() %s %d\n",m_ip.c_str(),m_port);
	srand(timeGetTime());

	UINT my_id=0;
	UINT my_port=m_my_port;//4661;  

	MetaSpooferReference sysref;
	const char* my_name=m_my_user_name.c_str();
	UINT my_name_length=(UINT)strlen(my_name);


	Buffer2000 packet;
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	//HELLO MOTHERFUCKER
	packet.WriteByte(0x01);  //Set the command byte equal to a HELLO

	packet.WriteBytes(m_my_hash,16);

	//write in my ip one byte at a time
	if(m_bind_ip.size()>0){
		int a1=0,a2=0,a3=0,a4=0;
		sscanf( m_bind_ip.c_str(), "%d.%d.%d.%d", &a1,&a2,&a3,&a4 );
		packet.WriteByte(a1);
		packet.WriteByte(a2);
		packet.WriteByte(a3);
		packet.WriteByte(a4);
	}
	else{
		packet.WriteDWord(my_id);  //write in a zero for id, if no bind ip specified.  although i suspect this marks us.
	}

	packet.WriteWord(my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	packet.WriteDWord(5); //5 metatags

	//metatag - name
	packet.WriteByte(0x02);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x01);	//compression
	packet.WriteWord(my_name_length);		//my name length
	packet.WriteString(my_name);		//my name

	//metatag - version
	packet.WriteByte(0x03);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x11);	//version
	packet.WriteDWord(60);		//my name length

	//metatag - port
	packet.WriteByte(0x03);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x0f);	//port we are listening on
	packet.WriteDWord(my_port);		//my name length

	//metatag - compression
	packet.WriteByte(0x03);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0x20);	//compression
	packet.WriteDWord(9);		//my name length

	//metatag - emule version
	packet.WriteByte(0x03);	//type
	packet.WriteWord(0x01);	//size
	packet.WriteByte(0xfb);	//emule version

	MetaSpooferReference ref;
	m_emule_version=ref.System()->GetEmuleVersion();
	packet.WriteDWord(m_emule_version);		//my version

	/*
	const UINT name_nemonic=0x01000102;
	const UINT version_nemonic=0x11000103;
	const UINT port_nemonic=0x0f000103;
	const UINT zlib_nemonic=0x20000103;

	Buffer2000 packet;
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x01);  //Set the command byte equal to a HELLO

	packet.WriteBytes(m_my_hash,16);

	packet.WriteDWord(my_id);
	packet.WriteWord(my_port);  //why edonkey writes the port twice in two different formats, is something I'd like to know

	packet.WriteDWord(4); //4 metatags

	//encode name
	packet.WriteDWord(name_nemonic);
	packet.WriteWord(my_name_length);
	packet.WriteString(my_name);

	//encode version
	packet.WriteDWord(version_nemonic);
	packet.WriteDWord(0x3c);  //must be 0x3c

	//encode the port .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(port_nemonic);
	packet.WriteDWord(my_port);  	

	//encode the zlib compression flag .... again because we are retarded like edonkey and like to waste bandwidth.
	packet.WriteDWord(zlib_nemonic);
	packet.WriteDWord(1);  	
	*/

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);  //fix the length

	//Lets login to this server
//	SpooferTCPReference reference;


	if(mp_main_connection!=NULL){
		delete mp_main_connection;
	}

	if(mp_firewall_connection!=NULL){
		delete mp_firewall_connection;
		mp_firewall_connection=NULL;
	}

	m_init_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	mp_main_connection=new PeerSocket();
	mp_main_connection->SetBindIP(m_bind_ip.c_str());
	mp_main_connection->Init(m_ip.c_str(),m_port,this);
	/*
	if(SOCKETERROR==mp_main_connection->Send(packet.GetBufferPtr(),packet.GetLength(),0);

	//close connections if they already exist.
	if(m_tcp_connection.GetHandle()){
		reference.TCP()->CloseConnection(m_tcp_connection);
		m_tcp_connection.SetHandle(NULL);
	}
	if(m_firewall_connection.GetHandle()){
		reference.TCP()->CloseConnection(m_firewall_connection);
		m_firewall_connection.SetHandle(NULL);
	}
	*/


	/*
	const byte *data=packet.GetBufferPtr();
	for(UINT i=0;i<packet.GetLength();i++){
		if(isalnum(data[i])){
			TRACE("SpoofHost::Init sent[%d] \t = \t 0x%x \t %c \n",i,data[i],data[i]);
		}
		else{//0x%x
			TRACE("SpoofHost::Init sent[%d] \t = \t 0x%x \n",i,data[i]);
		}
	}*/
}

void SpoofHost::NewConnection(PeerSocket *src)
{
	if(mp_firewall_connection)
		delete mp_firewall_connection;
	mp_firewall_connection=src;
	mp_firewall_connection->SetSocketEventListener(this);
	/*
	if(m_firewall_connection.GetHandle()){
		TRACE("MetaMachineSpoofer:  SpoofHost::NewConnection()  We already have a connection???  This isn't right.\n");
		SpooferTCPReference tcp;
		tcp.TCP()->CloseConnection(m_firewall_connection);
	}
	m_firewall_connection=con;*/
}


//called when data has been received by the socket system
/*
void SpoofHost::NewData(TKTCPConnection &con, Buffer2000 &data)
{
	if(m_tcp_connection==con){
		CSingleLock sl(&m_tcp_buffer_lock,TRUE);
		//TRACE("Received normal data from %s.\n",m_ip.c_str());
		if(m_state!=S_READY){
			MetaMachineSpooferDll::GetDlg()->LogData(m_ip.c_str(),&data);
		}
		m_tcp_buffer_pre.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_normal_data=true;
	}
	else if(m_firewall_connection==con){
		CSingleLock sl(&m_firewall_buffer_lock,TRUE);
		//TRACE("Received firewall data from %s.\n",m_ip.c_str());
		m_firewall_buffer.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_firewall_data=true;
	}
	else{
		TRACE("Receiving data from a wierd ass handle from %s.\n",m_ip.c_str());
//		CString log_msg;
//		log_msg.Format("Receiving data on an invalid handle connected to %s.\n",m_ip.c_str());
//		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
	}
}
*/
//REGULAR DATA
void SpoofHost::ProcessMainBuffer(void)
{
	if(m_tcp_buffer_post.GetLength()<5)
		return;

	const byte* p_data=m_tcp_buffer_post.GetBufferPtr();
	if(p_data[0]!=0xe3){
		//ASSERT(0);
		m_tcp_buffer_post.Clear();  //discard this disgustingly foul data
		return;
	}

	DWORD len=m_tcp_buffer_post.GetDWord(1);  //get the length of the packet
	if(len>m_tcp_buffer_post.GetLength()-5){  //have we received enough of the data yet?
		return;
	}
}

//FIREWALL DATA
void SpoofHost::ProcessFirewallBuffer(void)
{
	if(m_firewall_buffer.GetLength()<5 || mp_firewall_connection==NULL)
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
		//CString log_msg;
		//log_msg.Format("Server %s Responding to firewall query from %s.\n",m_ip.c_str(),m_ip.c_str());
		//MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);	

		UINT my_id=0;
		UINT my_port=4661;
		MetaSpooferReference sysref;
		const char* my_name=m_my_user_name.c_str();
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

		if(SOCKET_ERROR==mp_firewall_connection->Send(packet.GetBufferPtr(),packet.GetLength(),0)){
			Reset(true);
			return;
		}
		//Lets login to this server
		//SpooferTCPReference reference;
		//reference.TCP()->SendData(mp_firewall_connection,packet.GetBufferPtr(),packet.GetLength());
	}

	//we've had our chance to analyze this packet.  lets discard it whether we read it or not.
	m_firewall_buffer.RemoveRange(0,len+5);  //remove packet data we have already analysed.
	if(m_firewall_buffer.GetLength()>6){  //see if there is anymore relevant data available.
		ProcessFirewallBuffer();
	}
}

void SpoofHost::Reset(bool b_change_state)
{
	/*
	m_connected=CTime(1971,1,1,1,1,1);
	if(m_firewall_connection.GetHandle()){
		SpooferTCPReference tcp;
		tcp.TCP()->CloseConnection(m_firewall_connection);
		m_firewall_connection.SetHandle(NULL);
	}
	if(m_tcp_connection.GetHandle()){
		SpooferTCPReference tcp;
		tcp.TCP()->CloseConnection(m_tcp_connection);
		m_tcp_connection.SetHandle(NULL);
	}

	CSingleLock sl(&m_tcp_buffer_lock,TRUE);
	CSingleLock sl2(&m_firewall_buffer_lock,TRUE);
	m_tcp_buffer_post.Clear();
	m_tcp_buffer_pre.Clear();
	m_firewall_buffer.Clear();
	m_retry=0;

	if(b_change_state)  //to avoid an infinite recursive call from the change state method itself
		ChangeState(S_INIT);
		*/

	m_connected=CTime(1971,1,1,1,1,1);
	if(mp_firewall_connection!=NULL){
		delete mp_firewall_connection;
		mp_firewall_connection=NULL;
	}
	if(mp_main_connection!=NULL){
		delete mp_main_connection;
		mp_main_connection=NULL;
	}

//	CSingleLock sl(&m_tcp_buffer_lock,TRUE);
//	CSingleLock sl2(&m_firewall_buffer_lock,TRUE);
	m_tcp_buffer_post.Clear();
	m_tcp_buffer_pre.Clear();
	m_firewall_buffer.Clear();
	m_retry=0;

	b_down=true;
	down_time.Refresh();

	if(b_change_state)  //to avoid an infinite recursive call from the change state method itself
		ChangeState(S_INIT);
}

/*
void SpoofHost::LostConnection(TKTCPConnection &con)
{
	CString str;
	if(m_tcp_connection==con){
		CString log_msg;
		//log_msg.Format("LOST CONNECTION TO %s!\n",m_ip.c_str());
		//MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
		TRACE("MetaMachine:  SpoofHost::LostConnection() %s LOST it's MAIN CONNECTION!  This can't be good.\n",m_ip.c_str());
		str.Format("%s lost connection.\n",m_ip.c_str());
		MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Lost Connection");
		m_tcp_connection.SetHandle(NULL);
		ChangeState(S_INIT);
		m_retry=0;
	}
	else if(m_firewall_connection==con){
		//TRACE("MetaMachine:  SpoofHost::LostConnection() %s lost it's firewall back check!  This can't stop us now.\n",m_ip.c_str());
		m_firewall_connection.SetHandle(NULL);
	}
}*/

/*
void SpoofHost::Update(void)
{
	TRACE("SpoofHost::Update() %s:%d\n",m_ip.c_str(),m_port);
	srand(timeGetTime());
	CString str;
	bool b_updated_first_time=false;
//	while(!this->b_killThread){
//		m_infinite_loop_timer.Refresh();
		//process any received socket data that may have come in.
	if(b_new_normal_data){
		{//scoping block so we can do a quick lock while we transfer bytes into our analysis buffer
			//CSingleLock sl(&m_tcp_buffer_lock,TRUE);
			m_tcp_buffer_post.WriteBytes(m_tcp_buffer_pre.GetBufferPtr(),m_tcp_buffer_pre.GetLength());
			m_tcp_buffer_pre.Clear();
		}
		ClearUnwantedPackets();
		//ProcessHashQueryResponse();  //always check for some source queries, because we never know when we will receive them.
		if(m_state==S_INIT_RESPONSE){
			MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Connected");
			TRACE("MetaMachineSpoofer:  poofHost::Run() Host %s connected.\n",m_ip.c_str());
			m_connected=CTime::GetCurrentTime();
			
			ChangeState(S_READY_UPLOAD);
		}
		if(m_state==S_READY){
			m_tcp_buffer_post.Clear(); //we don't even want any data anyways.
		}
		b_new_normal_data=false;
	}
	else if(b_new_firewall_data){
		//CSingleLock sl(&m_firewall_buffer_lock,TRUE);
		ProcessFirewallBuffer();
		b_new_firewall_data=false;
	}
	
	if(!b_updated_first_time && m_created.HasTimedOut(m_connection_delay)){
		TRACE("Updating server %s for the first time.\n",m_ip.c_str());
		b_updated_first_time=true;
	}
	if(m_created.HasTimedOut(m_connection_delay))
		Update();  //update the state machine

	Sleep(100);
//	}
}
*/
void SpoofHost::ChangeState(SERVERSTATE new_state)
{
	SERVERSTATE old_state=m_state;
	m_state_time.Refresh();
	m_state=new_state;

	if(m_state==S_INIT){
		Reset(false);
		//TRACE("MetaMachine:  SpoofHost %s dropping back to S_INIT state from %d state.\n",m_ip.c_str(),old_state);
	}

	if(m_state==S_READY){

	}
}

//checks for a specific message, and removes any messages in the buffer that aren't this specific message
bool SpoofHost::HasFullResponse(byte code)
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

void SpoofHost::ClearUnwantedPackets(void)
{
	if(m_tcp_buffer_post.GetLength()<6)
		return;

	UINT index=0;
	CString log_msg;
	while(m_tcp_buffer_post.GetLength()>0){
		const byte* p_data=m_tcp_buffer_post.GetBufferPtr();
		if(p_data[index+0]!=0xe3 && p_data[index+0]!=0xd4 && p_data[index+0]!=0xc5){  //all packets had better start with this
			byte b=p_data[0]; //debug
			//ASSERT(0);
			m_tcp_buffer_post.Clear();  //discard this disgustingly foul data
			return;
		}

		DWORD len=m_tcp_buffer_post.GetDWord(index+1);  //get the length of the packet
		if(len>m_tcp_buffer_post.GetLength()-5){  //have we received enough of the data yet?
			return;
		}

		if(p_data[index+5]==0x42){  //is there a message indicating the ip/ports for a particular hash?  if so then yay because we need these
			TRACE("MetaMachine:  SpoofHost::ClearUnwantedPackets() %s has Hash Query Results!.\n",m_ip.c_str());
			Buffer2000 data;
			data.WriteBytes(m_tcp_buffer_post.GetBufferPtr()+index,5+len);
			if(p_data[index]==0xd4)
				DecompressPacket(data);
			
			log_msg.Format("SpoofHost::ClearUnwantedPackets() %s ProcessHashQueryResponse BEGIN",m_ip.c_str());
			SpoofingSystem::LogToFile(log_msg);
			ProcessHashQueryResponse(data);
			log_msg.Format("SpoofHost::ClearUnwantedPackets() %s ProcessHashQueryResponse END",m_ip.c_str());
			SpoofingSystem::LogToFile(log_msg);
			m_tcp_buffer_post.RemoveRange(index,5+len);  //extract this packet from our buffer.
		}
		if(p_data[index+5]==0x33){  //is this a search response?
			TRACE("MetaMachine:  SpoofHost::ClearUnwantedPackets() %s has Search Results!.\n",m_ip.c_str());
			Buffer2000 data;
			data.WriteBytes(m_tcp_buffer_post.GetBufferPtr()+index,5+len);
			if(p_data[index]==0xd4)
				DecompressPacket(data);
			ProcessSearchResults(data);
			m_tcp_buffer_post.RemoveRange(index,5+len);  //extract this packet from our buffer.
		}
		else if(p_data[index+5]==0x41){  //is there a message containing server information?
			//server info data
			TRACE("MetaMachine:  SpoofHost::ClearUnwantedPackets() %s has received server info\n",m_ip.c_str());
			/*Buffer2000 data;
			data.WriteBytes(m_tcp_buffer_post.GetBufferPtr()+index,5+len);
			if(p_data[index]==0xd4)
				DecompressPacket(data);*/
			m_tcp_buffer_post.RemoveRange(index,5+len);  //extract this packet from our buffer.
		}
		else{
			log_msg.Format("SpoofHost::ClearUnwantedPackets() %s UnhandledMessage %d",m_ip.c_str(),p_data[index+5]);
			SpoofingSystem::LogToFile(log_msg);
			m_tcp_buffer_post.RemoveRange(index,5+len);  //extract this packet from our buffer.
		}
	}
}

CTime SpoofHost::ConnectionTime(void)
{
	return m_connected;
}


void SpoofHost::Update(){
	//TRACE("SpoofHost::Update() %s:%d\n",m_ip.c_str(),m_port);
	//srand(timeGetTime());
	MetaSpooferReference ref;
	CString log_msg;
	bool b_weekend=ref.System()->IsWeekend();
	if(b_weekend && m_state!=S_INIT){
		
		log_msg.Format("SpoofHost::Update() %s ChangeState(S_INIT) BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		ChangeState(S_INIT);
		log_msg.Format("SpoofHost::Update() %s ChangeState(S_INIT) END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}


	CString str;
	CheckSendBuffer();
	m_infinite_loop_timer.Refresh();
	//process any received socket data that may have come in.
	if(b_new_normal_data){
		log_msg.Format("SpoofHost::Update() %s new normal data processing BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		{//scoping block so we can do a quick lock while we transfer bytes into our analysis buffer
			//CSingleLock sl(&m_tcp_buffer_lock,TRUE);
			m_tcp_buffer_post.WriteBytes(m_tcp_buffer_pre.GetBufferPtr(),m_tcp_buffer_pre.GetLength());
			m_tcp_buffer_pre.Clear();
		}
		log_msg.Format("SpoofHost::Update() %s ClearUnwantedPackets BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		ClearUnwantedPackets();
		log_msg.Format("SpoofHost::Update() %s ClearUnwantedPackets END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		//ProcessHashQueryResponse();  //always check for some source queries, because we never know when we will receive them.
		if(m_state==S_INIT_RESPONSE){
			MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Connected");
			TRACE("MetaMachineSpoofer:  SpoofHost::Update() Host %s %d bound to %s connected.\n",m_ip.c_str(),m_my_port, m_bind_ip.c_str());
			m_connected=CTime::GetCurrentTime();
			
			ChangeState(S_READY_UPLOAD);
		}
		if(m_state==S_READY){
			m_tcp_buffer_post.Clear(); //we don't even want any data anyways.
		}
		b_new_normal_data=false;
		log_msg.Format("SpoofHost::Update() %s new normal data processing END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
	}
	else if(b_new_firewall_data){
		log_msg.Format("SpoofHost::Update() %s new firewall data processing BEGIN.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		//CSingleLock sl(&m_firewall_buffer_lock,TRUE);
		ProcessFirewallBuffer();
		log_msg.Format("SpoofHost::Update() %s new firewall data processing END.",m_ip.c_str());
		ref.System()->LogToFile(log_msg);
		b_new_firewall_data=false;
	}
	
	if(!b_weekend && m_created.HasTimedOut(m_connection_delay) && m_restart_delay.HasTimedOut(m_connection_delay))
		UpdateState();  //update the state machine
}

/*
bool SpoofHost::IsThreadLockedUp(void)
{
	if(m_infinite_loop_timer.HasTimedOut(90)){
		return true;
	}
	else return false;
}
*/



void SpoofHost::UploadFiles(void)
{
	TRACE("SpoofHost::UploadFiles()[1] %s:%d\n",m_ip.c_str(),m_port);
	CString log_msg;
	log_msg.Format("SpoofHost::UploadFiles() %s BEGIN",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);

	srand(timeGetTime());

	Vector v_decoys;
//	Vector v_spoofs;
	Vector v_swarms;

	MetaSpooferReference sysref;

//	if(SPOOFER){  //priorities of what kind of files we get
//		sysref.System()->GetSomeDecoys(80,100,v_decoys);  //get up to x decoys
//		sysref.System()->GetSomeSpoofs(80,680,v_spoofs);  //get up to x spoofs
//		sysref.System()->GetSomeSwarms(80,40,v_swarms);  //get a few swarms, too many causes an extremely heavy load
//	}
//	else{
//	sysref.System()->GetSomeDecoys(80,400,v_decoys);  //get up to x decoys
//	sysref.System()->GetSomeSpoofs(80,200,v_spoofs);  //get up to x spoofs
//	sysref.System()->GetSomeSwarms(80,250,v_swarms);  //get a few swarms, too many causes an extremely heavy load
//	}

	if(DECOYER){
		//sysref.System()->GetSomeSwarms(95,0,v_swarms);  //get a few swarms, too many causes an extremely heavy load
	}
	else{
		sysref.System()->GetSomeSwarms(95,200,v_swarms);  //get a few swarms, too many causes an extremely heavy load
	}

	int tlimit=600;
	if(stricmp(m_ip.c_str(),"81.23.250.167")==0)
		tlimit=400;

	sysref.System()->GetSomeDecoys(95,tlimit-v_swarms.Size()-(rand()%(tlimit/3)),v_decoys);  //get up to x decoys
	//sysref.System()->GetSomeSpoofs(80,60,v_spoofs);  //get up to x spoofs
	
	log_msg.Format("%s SpoofHost::UploadFiles() [0]",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);


	//Sleep(5);
	

	DWORD num_files=(DWORD)v_swarms.Size();
	num_files+=(int)v_decoys.Size();

	int files_to_go=num_files;

	while(files_to_go>0){
		log_msg.Format("%s SpoofHost::UploadFiles() Files To Go %d",m_ip.c_str(),files_to_go);
		SpoofingSystem::LogToFile(log_msg);
		//we now have the list of fake files we want to upload to our server.  lets dispatch them.
		Buffer2000 packet;
		packet.WriteByte(0xe3);

		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

		//HELLO MOTHERFUCKER
		packet.WriteByte(0x15);  //Set the command byte equal to a HELLOANSWER

		packet.WriteDWord(min(200,files_to_go));  //write how many files we'll be uploading to the server

		int files_added=0;

		while(v_decoys.Size()>0 && files_added<200){
			DonkeyFile *df=(DonkeyFile*)v_decoys.Get(0);
			AddFileToUploadList(packet,df->m_hash,df->m_size,df->m_file_name);  //insert a real file (a decoy anyway) for upload here.
			v_decoys.Remove(0);
			files_added++;
			files_to_go--;
		}

		while(v_swarms.Size()>0 && files_added<200){
			Swarm* swarm=(Swarm*)v_swarms.Get(0);
			
			sysref.System()->AddSwarm(swarm->m_hash,swarm->m_file_name.c_str(),swarm->m_hash_set,swarm->m_file_size);
			AddFileToUploadList(packet,swarm->m_hash,swarm->m_file_size,swarm->m_file_name.c_str());
			v_swarms.Remove(0);
			files_to_go--;
			files_added++;
		}

		UINT packet_length=packet.GetLength();
		packet.WriteDWord(1,packet_length-5);

		/*
		const byte* buffer=packet.GetBufferPtr();
		TRACE("Dump of upload transmission to SpoofHost %s.\n",m_ip.c_str());
		for(UINT i=0;i<packet.GetLength();i++){
			if(isalnum(buffer[i])){
				TRACE("packet[%d] \t = \t 0x%x \t %c \n",i,buffer[i],buffer[i]);
			}
			else{//0x%x
				TRACE("packet[%d] \t = \t 0x%x \n",i,buffer[i]);
			}
		}
		TRACE("END Dump of upload transmission to SpoofHost %s.\n",m_ip.c_str());
		*/

		//Lets let them know about all the wonderful files we will be sharing today.
		if(files_added>0){
			/*
			const byte* buffer=packet.GetBufferPtr();
			TRACE("Dump of upload transmission to SpoofHost %s.\n",m_ip.c_str());
			for(UINT i=0;i<packet.GetLength();i++){
				if(isalnum(buffer[i])){
					TRACE("packet[%d] \t = \t 0x%x \t %c \n",i,buffer[i],buffer[i]);
				}
				else{//0x%x
					TRACE("packet[%d] \t = \t 0x%x \n",i,buffer[i]);
				}
			}
			TRACE("END Dump of upload transmission to SpoofHost %s.\n",m_ip.c_str());
			*/

			CompressPacket(packet);
			//SpooferTCPReference reference;
			//reference.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());  
			//int sent_len=mp_main_connection->Send(packet.GetBufferPtr(),packet.GetLength(),0);
			m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
		}
	}
	TRACE("SpoofHost %s %d bound to %s has had %d files uploaded to it.\n",m_ip.c_str(),m_my_port,m_bind_ip.c_str(),num_files);
	m_uploaded_files=num_files;
	CString str;
	str.Format("Uploaded %d files.",m_uploaded_files);
	MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),str);

	sysref.System()->LogServerConnection(m_ip.c_str());

	log_msg.Format("SpoofHost::UploadFiles() %s %d files uploaded END",m_ip.c_str(),m_uploaded_files);
	SpoofingSystem::LogToFile(log_msg);
}

/*
byte SpoofHost::ConvertCharToInt(char ch)
{
	switch(ch){
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'a': return 10;
		case 'B': return 11;
		case 'b': return 11;
		case 'C': return 12;
		case 'c': return 12;
		case 'D': return 13;
		case 'd': return 13;
		case 'E': return 14;
		case 'e': return 14;
		case 'F': return 15;
		case 'f': return 15;
	}
	return 0;
}*/

void SpoofHost::AskForServerInfo(void)
{
	Buffer2000 packet;
	packet.WriteByte(0xe3);

	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x14);  //Set the command byte equal to a GETSERVERLIST

	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);

	m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	//SpooferTCPReference reference;

	//reference.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
	
}

bool SpoofHost::IsConnected(void)
{
	if(m_state==S_READY && mp_main_connection!=NULL)
		return true;
	else
		return false;
}

int SpoofHost::GetUploadedFiles(void)
{
	return m_uploaded_files;
}

int SpoofHost::GetState(void)
{
	return (int)m_state;
}

void SpoofHost::AddFileToUploadList(Buffer2000 &packet,byte* hash, int size , string name)
{
	//TRACE("Server %s adding file %s with size %d to file upload list\n",m_ip.c_str(),name.c_str(),size);
	static const UINT name_nemonic = 0x01000102;
	static const UINT size_nemonic = 0x02000103;

	//write out file hash
	packet.WriteBytes(hash,16);

	// publishing an incomplete file
//	nClientID = 0xFCFCFCFC;
//	nClientPort = 0xFCFC;

	// publishing a complete file
	//nClientID = 0xFBFBFBFB;
	//nClientPort = 0xFBFB;
	
	bool b_complete=false;
	if(size>50000000){
		if((rand()%100)>69){
			b_complete=true;
		}
	}
	else{
		if((rand()%100)>20){
			b_complete=true;
		}
	}

	if(b_complete){  //30% chance we will say we have the complete file
		//complete file
		packet.WriteDWord(0xFBFBFBFB); //my id  - should be zero cus it emule always reports it as zero, even though it seems like it should be set to my actual ip.
		packet.WriteWord(0xFBFB);  //my port - same
	}
	else{
		//incomplete file
		packet.WriteDWord(0xFCFCFCFC); //my id  - should be zero cus it emule always reports it as zero, even though it seems like it should be set to my actual ip.
		packet.WriteWord(0xFCFC);  //my port - same
	}

	
    int media_length=0;
    string extension=MetaSpooferUtilityFunctions::GetExtension(name.c_str());

    media_length=MetaSpooferUtilityFunctions::CalculateMediaLength(extension.c_str(),name.c_str(),size);
    string file_type=MetaSpooferUtilityFunctions::GetFileType(extension.c_str());
    int bitrate=MetaSpooferUtilityFunctions::GetBitrate(extension.c_str(),name.c_str());
	string codec=MetaSpooferUtilityFunctions::GetCodec(extension.c_str(),name.c_str());

    int num_tags=2;
    if(file_type.length()>0)
      num_tags++;
    if(extension.length()>0)
      num_tags++;
    if(codec.length()>0)
      num_tags++;
    if(media_length!=0)
      num_tags++;
    if(bitrate>0)
      num_tags++;

	packet.WriteDWord(num_tags);  //we will be providing two metatags with this here file

    //output.writeInt(num_tags); //2 metatags

    //write out name
    MetaSpooferUtilityFunctions::WriteEmuleTag(name.c_str(),0x01,packet,m_emule_version);

    //write out file size
    MetaSpooferUtilityFunctions::WriteEmuleTag(size,0x02,packet,m_emule_version);

    if(file_type.size()>0){
      MetaSpooferUtilityFunctions::WriteEmuleTag(file_type.c_str(),0x03,packet,m_emule_version);
    }
    if(extension.size()>0){
      MetaSpooferUtilityFunctions::WriteEmuleTag(extension.c_str(),0x04,packet,m_emule_version);
    }
    if(media_length>0){
      MetaSpooferUtilityFunctions::WriteEmuleTag(media_length,0xD3,packet,m_emule_version);
    }
    if(codec.length()>0){
      MetaSpooferUtilityFunctions::WriteEmuleTag(codec.c_str(),0xD5,packet,m_emule_version);
    }
    if(bitrate>0){
      MetaSpooferUtilityFunctions::WriteEmuleTag(bitrate,0xd4,packet,m_emule_version);
    }



	//write in the metatag corresponding to the name
//	packet.WriteDWord(name_nemonic);  //write out the code signifying a string, filename
//	packet.WriteWord((WORD)name.size());  //write out the length of the string
//	packet.WriteBytes((byte*)name.c_str(),(UINT)name.size());
//	packet.WriteDWord(size_nemonic);  //write the code signifying an int, file size.
//	packet.WriteDWord(size);  //write in our file size	
}

int SpoofHost::AddSpoofFiles(Buffer2000& packet, Vector &v_spoof_files, int cur_num_files, int file_limit)
{
	Vector v_hashes;

	int slen=(int)strlen(m_ip.c_str());
	int name_sum=0;
	for(int i=0;i<slen;i++){
		name_sum+=(int)*(m_ip.c_str()+i);
	}
	slen+=m_port;
	
	int rand_seed=slen;  //a seed that is a function of the server

	UINT emergency_seed=rand();

	int spoofs_added=0;

	while(cur_num_files+spoofs_added<file_limit && v_spoof_files.Size()>0){
		srand(emergency_seed+=57);  //we have to reseed back to a truely random number, since we've probably reseeded to something to do with the server/filename

		//pick a spoof, pick any spoof
		Spoof *spoof=(Spoof*)v_spoof_files.Get(rand()%v_spoof_files.Size());

		byte hash[16];
		int attempt=0;

		bool b_hash_valid=false;

		int orig_file_size=max(1000000,spoof->m_file_size);
		int file_size=orig_file_size;
		//try up to 15 times to generate a hash for it, if that failes just make a totally random hash
		while(attempt++<15){
			srand(emergency_seed+=3);  //seed to randomly pick one of 15 hashes, rand_seed will make sure its ONE of THOSE 15 hashes
			file_size=orig_file_size;
			GenerateHash(spoof->m_file_name.c_str(),hash,false,rand_seed,file_size);
			bool b_found=false;
			for(int i=0;i<(int)v_hashes.Size();i++){
				HashObject *hash_object=(HashObject*)v_hashes.Get(i);
				if(memcmp(hash_object->m_hash,hash,16)==0){
					b_found=true;
					break;
				}
			}
			if(!b_found){
				b_hash_valid=true;
				break;
			}
		}

		if(!b_hash_valid){
			srand(emergency_seed+=63);
			file_size=orig_file_size;
			GenerateHash(spoof->m_file_name.c_str(),hash,true,rand_seed,file_size);
		}
		v_hashes.Add(new HashObject(hash));

		AddFileToUploadList(packet,hash,file_size,spoof->m_file_name);
		spoofs_added++;
	}

	srand(timeGetTime()+rand());

	return spoofs_added;
}

void SpoofHost::GenerateHash(const char* file_name,byte* buffer,bool b_total_random,int seed,int &file_size)
{
	int slen=(int)strlen(file_name);
	int name_sum=0;
	for(int i=0;i<slen;i++){
		name_sum+=(int)*(file_name+i);
	}

	//slen+=rand()%15;  //generate one of 10 random hashes/sizes for this particular file name/server combo

	if(!b_total_random)
		srand(seed+name_sum+file_size+(rand()%10));

	//we are also going to end up with one of 10 random file sizes
	if(file_size>800000){
		int dif=(file_size>>3);
		int rand_val=(rand()<<16)|rand();
		if(rand_val<0)
			rand_val=-rand_val;
		int add_num=rand_val%(dif+1);
		int change=( -dif + 2*(add_num));
		file_size+=change;
	}
	if(file_size<800000)
		file_size=800000+rand()%100000;
	while((file_size%137)!=0)
		file_size++;

	for(int i=0;i<16;i++){
		buffer[i]=0;
		buffer[i]|=(byte)rand();
	}
}

void SpoofHost::CompressPacket(Buffer2000 & packet)
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

void SpoofHost::DecompressPacket(Buffer2000& packet)
{
	if(packet.GetByte(0)!=0xd4)
		return;

	CString log_msg;
	log_msg.Format("SpoofHost::ClearUnwantedPackets() %s DecompressPacket BEGIN",m_ip.c_str(),m_uploaded_files);
	SpoofingSystem::LogToFile(log_msg);
	//TRACE("MetamachineSpoofer  SpoofHost::DecompressPacket() from server %s with command byte of %x.\n",m_ip.c_str(),packet.GetByte(5));

	UINT compressed_size=packet.GetDWord(1)-1;//*(UINT*)(buffer+12);  //get the original size of our data.
	byte *uncompressed_data=new byte[compressed_size*8+(1<<16)];
	UINT uncompress_buf_length=compressed_size*8+(1<<16);
	uncompress ((Bytef*)uncompressed_data, (uLongf *)&uncompress_buf_length, (const Bytef *)(packet.GetBufferPtr()+6), (uLong)packet.GetLength()-6);

	byte tmp_buf[6];
	memcpy(tmp_buf,packet.GetBufferPtr(),6);
	tmp_buf[0]=0xc5;  //we are going to have to pretend we are emule for this.
	tmp_buf[5]=0x40;  //command byte indicates an emule packed packet

	packet.Clear();
	packet.WriteBytes(tmp_buf,6);
	packet.WriteBytes(uncompressed_data,uncompress_buf_length);
	
	//re-encode the length
	UINT packet_length=packet.GetLength();
	packet.WriteDWord(1,packet_length-5);

	delete[] uncompressed_data;

	log_msg.Format("SpoofHost::ClearUnwantedPackets() %s DecompressPacket END",m_ip.c_str(),m_uploaded_files);
	SpoofingSystem::LogToFile(log_msg);
}

void SpoofHost::GetSourcesForHash(void)
{
	if(mp_main_connection==NULL)
		return;
	MetaSpooferReference sys;
	string str_hash=sys.System()->GetHashThatNeedsSources();
	TRACE("MetaMachineSpoofer:  SpoofHost::GetSourcesForHash() Host %s getting sources for hash %s.\n",m_ip.c_str(),str_hash.c_str());
	if(str_hash.size()!=32)
		return;

	byte byte_hash[16];
	
	for(int j=0;j<16;j++){
		char ch1=str_hash[j*2];
		char ch2=str_hash[j*2+1];
		byte val1=ConvertCharToInt(ch1);
		byte val2=ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		byte_hash[j]=hash_val;
	}

	Buffer2000 packet;
	packet.WriteByte(0xe3);  //yeah we are emule.
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x19);  //Set the command byte equal to a GETSOURCES
	packet.WriteBytes(byte_hash,16);
	packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

	//SpooferTCPReference tcp;

	m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	//tcp.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength());
}

CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
{
	CString base16_buff;
	static byte base16_alphabet[] = "0123456789ABCDEF";

	for(unsigned int i = 0; i < bufLen; i++) {
		base16_buff += base16_alphabet[buffer[i] >> 4];
		base16_buff += base16_alphabet[buffer[i] & 0xf];
	}

    return base16_buff;
}


void SpoofHost::ProcessHashQueryResponse(Buffer2000 &data)
{
	
	if(data.GetLength()<6+16+6)
		return;

	UINT index=0;

	byte *p_data=(byte*)(data.GetBufferPtr()+index);
	byte *p_start=p_data;
	if(p_data[index+0]!=0xe3){
		return;
	}

	if(p_data[index+5]!=0x42){
		return;
	}
	
	p_data+=6;
	byte hash[16];
	memcpy(hash,p_data,16);
	p_data+=16;
	byte bcount=(byte)(*p_data);  //number of ips as a byte
	UINT count=bcount;
	TRACE("Adding %d sources.\n",count);
	p_data++;
	for(UINT j=0;j<count;j++){
		byte ip1=*(p_data+0);
		byte ip2=*(p_data+1);
		byte ip3=*(p_data+2);
		byte ip4=*(p_data+3);
		CString sip;
		sip.Format("%d.%d.%d.%d",ip1,ip2,ip3,ip4);  //comment this line out if its already in IP format x.x.x.x
		//UINT ip=*((UINT*)p_data);
		unsigned short port=*((unsigned short*)(p_data+4));

		p_data+=4;  //advance past the ip
		p_data+=2;  //advance past the port

		MetaSpooferReference ref;

		CString shash=EncodeBase16(hash,16);

		ref.System()->AddSourceForHash(sip,port,shash);

		if(p_data>(p_start+data.GetLength()))  //double check to make sure we aren't reading past our data bounds
			return;
	}
}

void SpoofHost::UpdateState(void)
{
	MetaSpooferReference ref;
	CString log_msg;
	if(m_state==S_INIT && mb_active_server){
		m_uploaded_files=0;
		if(m_state_time.HasTimedOut(30)){  //let it wait a bit before trying to reconnect
			m_state_time.Refresh();
			if(b_down && down_time.HasTimedOut(m_dead_time)){  //go down for about m_dead_time
				b_down=false;
			}

			if(b_down){  //we are still down
				return;
			}
				
			b_down=true;
			down_time.Refresh();  //we failed our connection attempt, we don't want to try back for quite some time
			MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Attempting Connection");
			//DWORD delay_load=rand()%(1000);  //CONNECTION DELAY 12 minutes  //comment out for debugging we don't want to wait
			//int delay_load=0;  //No connection delay  
			//Sleep(delay_load);  //delay for a random amount of time so every server on our network doesn't try to connect at once.  This isn't done in the GUI thread, this function is only called from a thread specific to this instance of SpoofHost.
			Init();
			ChangeState(S_INIT_RESPONSE);
		}
		return;
	}

	if(m_state==S_INIT_RESPONSE){  //we are waiting for a connection response from this server
		if(m_state_time.HasTimedOut(40)){  //if nothing happened after 40 seconds, abort the connection attempt

			MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Init Failed, Retrying");
			//TRACE("MetaMachine:  SpoofHost %s init timed out, going back into INIT  mode.\n",m_ip.c_str());
			ChangeState(S_INIT);
		}
		return;
	}

	if(m_state==S_READY_UPLOAD){
		if(m_state_time.HasTimedOut(8)){  //allow a bit of a pause before we begin uploading
			log_msg.Format("SpoofHost::UpdateState() %s UploadFiles BEGIN.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			MetaMachineSpooferDll::GetDlg()->Log(m_ip.c_str(),"Attempting To Send Files");
			UploadFiles();  //upload our files to this server.
			log_msg.Format("SpoofHost::UpdateState() %s UploadFiles END.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			ChangeState(S_READY_ASK_FOR_SERVERLIST);
		}
	}

	if(m_state==S_READY_ASK_FOR_SERVERLIST){
		if(m_state_time.HasTimedOut(5)){  //allow a bit of a pause before we begin searching again
			log_msg.Format("SpoofHost::UpdateState() %s AskForServerInfo BEGIN.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			AskForServerInfo();  //ask our host for info about itself, so we look normal.
			log_msg.Format("SpoofHost::UpdateState() %s AskForServerInfo END.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			ChangeState(S_READY);
		}
	}

	if(m_state==S_READY){  //we are ready to do a search
		if(m_state_time.HasTimedOut(respoof_time)){  //once a day we should pick a new set of files to spoof
			ChangeState(S_INIT);
			respoof_time=(60*60)*24+(rand()%(60*60));
		}
		else if(m_get_sources_timer.HasTimedOut(m_get_sources_delay) && m_state_time.HasTimedOut(60)){
			log_msg.Format("SpoofHost::UpdateState() %s GetSourcesForHash BEGIN.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			GetSourcesForHash();
			log_msg.Format("SpoofHost::UpdateState() %s GetSourcesForHash END.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			m_get_sources_delay=60*45+rand()%3600;
			m_get_sources_timer.Refresh();
		}
		else if(m_search_timer.HasTimedOut(m_search_delay) && m_state_time.HasTimedOut(60)){  //every x minutes we will do a search
			log_msg.Format("SpoofHost::UpdateState() %s DoSearch BEGIN.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			DoSearch();
			log_msg.Format("SpoofHost::UpdateState() %s DoSearch END.",m_ip.c_str());
			ref.System()->LogToFile(log_msg);
			m_search_timer.Refresh();
		}
		return;
	}
}

void SpoofHost::OnClose(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("SpoofHost::OnClose() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	TRACE("SpoofHost::OnClose() %s %d bound to %s.\n",m_ip.c_str(),m_my_port,m_bind_ip.c_str());
	CString str;
	if(mp_main_connection==src){
		m_restart_delay.Refresh();
		Reset(true);
	}
	else if(mp_firewall_connection==src){
		delete mp_firewall_connection;
		mp_firewall_connection=NULL;
	}

	log_msg.Format("SpoofHost::OnClose() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void SpoofHost::OnReceive(CAsyncSocket* src,byte *data,UINT length)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("SpoofHost::OnReceive() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//TRACE("SpoofHost::OnReceive() %s.\n",m_ip.c_str());
	if(mp_main_connection==src){
		m_tcp_buffer_pre.WriteBytes(data,length);
		b_new_normal_data=true;
	}
	else if(mp_firewall_connection==src){
		m_firewall_buffer.WriteBytes(data,length);
		b_new_firewall_data=true;
	}

	log_msg.Format("SpoofHost::OnReceive() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);	

/*
	if(m_tcp_connection==con){
		CSingleLock sl(&m_tcp_buffer_lock,TRUE);
		//TRACE("Received normal data from %s.\n",m_ip.c_str());
		if(m_state!=S_READY){
			MetaMachineSpooferDll::GetDlg()->LogData(m_ip.c_str(),&data);
		}
		m_tcp_buffer_pre.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_normal_data=true;
	}
	else if(m_firewall_connection==con){
		CSingleLock sl(&m_firewall_buffer_lock,TRUE);
		//TRACE("Received firewall data from %s.\n",m_ip.c_str());
		m_firewall_buffer.WriteBytes(data.GetBufferPtr(),data.GetLength());
		b_new_firewall_data=true;
	}
	else{
		TRACE("Receiving data from a wierd ass handle from %s.\n",m_ip.c_str());
//		CString log_msg;
//		log_msg.Format("Receiving data on an invalid handle connected to %s.\n",m_ip.c_str());
//		MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
	}
	*/
}

void SpoofHost::OnSend(CAsyncSocket* src)
{
	TRACE("SpoofHost::OnSend() %s %d bound to %s..\n",m_ip.c_str(),m_my_port,m_bind_ip.c_str());
}

void SpoofHost::OnConnect(CAsyncSocket* src)
{
	MetaSpooferReference ref;
	CString log_msg;
	log_msg.Format("SpoofHost::OnConnect() %s STARTED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
	//reference.TCP()->OpenConnection(m_tcp_connection);
	//if(SOCKET_ERROR==reference.TCP()->SendData(m_tcp_connection,packet.GetBufferPtr(),packet.GetLength())){
	//	Reset(true);
	//}
	TRACE("SpoofHost::OnConnect() %s %d bound to %s.\n",m_ip.c_str(),m_my_port,m_bind_ip.c_str());

	m_send_buffer.WriteBytes(m_init_buffer.GetBufferPtr(),m_init_buffer.GetLength());
//	if(src==mp_main_connection && m_init_buffer.GetLength()>0){
//		if(SOCKET_ERROR==mp_main_connection->Send(m_init_buffer.GetBufferPtr(),m_init_buffer.GetLength(),0))
//			Reset(true);
//	}

	log_msg.Format("SpoofHost::OnConnect() %s FINISHED.",m_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void SpoofHost::CheckSendBuffer(void)
{
	if( m_send_buffer.GetLength()>0 && mp_main_connection){
		MetaSpooferReference ref;
		CString log_msg;
		log_msg.Format("SpoofHost::CheckSendBuffer() %s %d bytes of data to be sent BEGIN.",m_ip.c_str(),m_send_buffer.GetLength());
		ref.System()->LogToFile(log_msg);
		//MetaSpooferReference ref;
		//CString log_msg;
		//log_msg.Format("ClientConnection::CheckSendBuffer() %s STARTED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
//		mb_can_send=false;
		int send_stat=mp_main_connection->Send(m_send_buffer.GetBufferPtr(),min(300+(rand()%200),(int)m_send_buffer.GetLength()));
		//TRACE("ClientConnection::CheckSendBuffer() %s sent %d bytes.\n",m_ip.c_str(),send_stat);
		if(SOCKET_ERROR==send_stat){
		//	TRACE("ClientConnection::CheckSendBuffer() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
			//mb_abort=true;
		}
		else if(send_stat>0 && send_stat<20000000){
			m_send_buffer.RemoveRange(0,send_stat);
		}

		log_msg.Format("SpoofHost::CheckSendBuffer() %s %d bytes of data were sent END.",m_ip.c_str(),send_stat);
		ref.System()->LogToFile(log_msg);
		//log_msg.Format("ClientConnection::CheckSendBuffer() %s FINISHED.",m_ip.c_str());
		//ref.System()->LogToFile(log_msg);
	}
}

void SpoofHost::SetBindIP(const char* bind_ip)
{
	m_bind_ip=bind_ip;
}

bool SpoofHost::IsOld(void)
{
	if(m_age.HasTimedOut(m_expiration_time))
		return true;
	else return false;
}

void SpoofHost::ProcessServerInfoResponse(Buffer2000 &data)
{
	const byte *p_data=data.GetBufferPtr();
	UINT len=data.GetDWord(1);
	const byte *p_end=p_data+len+5;

	p_data+=6; //skip past header, len and command byte
	p_data+=16;  //skip past server hash
	p_data+=6;  //skip past server address

	//now our data pointer references the metadata

	/*
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
			if((sr->m_size%1397)==0 || (sr->m_size%137)==0){
				continue;//yes it was a false decoy
			}
			if(p_search_job->AddResult(sr)){
				p_search_job->AddIP(0,sr,availability,m_ip.c_str());
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
		TKTCPConnection con("",0);
		con.SetHandle(m_tcp_handle);

		reference.TCP()->SendData(con,packet.GetBufferPtr(),packet.GetLength());
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
	*/
}

void SpoofHost::DoSearch(void)
{
	m_search_count=0;
	MetaSpooferReference ref;
	m_search_task=ref.System()->GetSearchTask();
	

	//TYDEBUG
	if(m_search_task.m_search_string.size()<1){
		//m_search_task.m_search_string="seer mp3";
		//m_search_task.m_project="blah";
		//m_search_task.m_minimum_size=1000000;
		return;
	}

//	m_search_task.m_search_string="seer mp3";
//	m_search_task.m_project="blah";
//	m_search_task.m_minimum_size=1000000;

	TRACE("SpoofHost::DoSearch() %s searching for %s\n",m_ip.c_str(),m_search_task.m_search_string.c_str());

	Buffer2000 packet;
	packet.WriteByte(0xe3);  //yeah we are emule.
	packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

	packet.WriteByte(0x16);  //Set the command byte equal to a SEARCHRQST
	packet.WriteByte(0x01);  //We are going to write a string
	
	packet.WriteWord((WORD)m_search_task.m_search_string.size());  //Write in the length of the string
	packet.WriteString(m_search_task.m_search_string.c_str());  //Write in the string
	packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

	m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	//reference.TCP()->SendData(con,packet.GetBufferPtr(),packet.GetLength());
}

CString SpoofHost::EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
{
	CString base16_buff;
	static byte base16_alphabet[] = "0123456789ABCDEF";

	for(unsigned int i = 0; i < bufLen; i++) {
		base16_buff += base16_alphabet[buffer[i] >> 4];
		base16_buff += base16_alphabet[buffer[i] & 0xf];
	}

    return base16_buff;
}

void SpoofHost::ProcessSearchResults(Buffer2000& data)
{
	CString log_msg;
	log_msg.Format("%s SpoofHost::ProcessSearchResults() BEGIN",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);
	MetaSpooferReference ref;

	const byte *p_data=data.GetBufferPtr();
	UINT len=data.GetDWord(1);
	const byte *p_end=p_data+len+5;

	UINT result_count=data.GetDWord(6);
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

	TRACE("SpoofHost::ProcessSearchResults() host %s received %d search results\n",m_ip.c_str(),result_count);

	Vector v_search_results;

	log_msg.Format("%s SpoofHost::ProcessSearchResults() About to Process %d search results",m_ip.c_str(),result_count);
	SpoofingSystem::LogToFile(log_msg);

	bool b_ok=true;
	for(UINT i=0;i<result_count && b_ok;i++){
		SearchResult *sr=new SearchResult();
		sr->m_project=m_search_task.m_project;
		sr->m_server_ip=m_ip;

		sr->m_hash=(LPCSTR)EncodeBase16(p_data,16);

		p_data+=16; //skip past the hash
		ASSERT(p_data<=p_end);  //check just in case.

		p_data+=6; //skip past the bogus clientid and port
		ASSERT(p_data<=p_end);  //check just in case.

		UINT tag_count=*((UINT*)p_data);  //how many metadata tags are attached?
		p_data+=4;
		ASSERT(p_data<=p_end);  //check just in case.
	


		int prev_tag=0;
		int last_after_tag_debug=0;
		CString tag_name;
		//char tag_name[256];
		char string_val[1024];

		log_msg.Format("%s SpoofHost::ProcessSearchResults() Result %d has %d tags",m_ip.c_str(),i,tag_count);
		SpoofingSystem::LogToFile(log_msg);

		for(UINT j=0;j<tag_count;j++){

			byte tag_type = *(p_data++);
			byte specialtag=0;
			//tag_name[0]='\0';
			
			string_val[0]='\0';
			DWORD int_val=0;
			float float_val=0;

			log_msg.Format("%s SpoofHost::ProcessSearchResults() Result %d, tag %d has type 0x%02X",m_ip.c_str(),i,j,tag_type);
			SpoofingSystem::LogToFile(log_msg);

			if (tag_type & 0x80)
			{				
				tag_type &= 0x7F;
				specialtag = *(p_data++);

				log_msg.Format("%s SpoofHost::ProcessSearchResults() Result %d, tag %d, type 0x%02X has name 0x%02X",m_ip.c_str(),i,j,tag_type,specialtag);
				SpoofingSystem::LogToFile(log_msg);
				TRACE("Processing emule specific search result tag.type = 0x%02X and special tag = 0x%02X\n",tag_type,specialtag);
			}
			else
			{
				WORD tag_length = *(WORD*)p_data;//data->ReadUInt16();
				p_data+=2;
				
				if (tag_length == 1){
					specialtag = *(p_data++);
					log_msg.Format("%s SpoofHost::ProcessSearchResults() Result %d, tag %d, type 0x%02X has name 0x%02X",m_ip.c_str(),i,j,tag_type,specialtag);
					SpoofingSystem::LogToFile(log_msg);
				}
				else
				{
					p_data+=tag_length;  //advance past the tag
					log_msg.Format("%s SpoofHost::ProcessSearchResults() Result %d, tag %d, type 0x%02X has variable length tag name of length %d",m_ip.c_str(),i,j,tag_type,tag_length);
					SpoofingSystem::LogToFile(log_msg);
				
				//	memcpy(tag_name,p_data,min(tag_length,255));
				//	p_data+=tag_length;
				//	tag_name[min(tag_length,255)] = '\0';
				}
			}

			// NOTE: It's very important that we read the *entire* packet data, even if we do
			// not use each tag. Otherwise we will get troubles when the packets are returned in 
			// a list - like the search results from a server.
			if (tag_type == TAGTYPE_STRING)
			{
				WORD string_len = *(WORD*)p_data;
				p_data+=2;
				memcpy(string_val,p_data,min(string_len,1023));
				p_data+=string_len;
				string_val[min(string_len,1023)] = '\0';
			}
			else if (tag_type == TAGTYPE_UINT32)
			{
				int_val=*(DWORD*)p_data;
				p_data+=4;
			}
			else if (tag_type == TAGTYPE_UINT16)
			{
				int_val=*(WORD*)p_data;
				p_data+=2;
			}
			else if (tag_type == TAGTYPE_UINT8)
			{
				int_val=*(p_data++);
			}
			else if (tag_type == TAGTYPE_FLOAT32)
			{
				float_val=*(float*)p_data;
				p_data+=4;
			}
			else if (tag_type >= TAGTYPE_STR1 && tag_type <= TAGTYPE_STR16)
			{
				//UINT length = tag.type - TAGTYPE_STR1 + 1;
				//tag.stringvalue = new char[length+1];
				//data->Read(tag.stringvalue, length);
				//tag.stringvalue[length] = '\0';
				
				WORD string_len = tag_type - TAGTYPE_STR1 + 1;
				memcpy(string_val,p_data,min(string_len,1023));
				p_data+=string_len;
				string_val[min(string_len,1023)] = '\0';

				tag_type = TAGTYPE_STRING;
			}
			else if (tag_type == TAGTYPE_HASH)
			{
				p_data+=16;
			}
			else if (tag_type == TAGTYPE_BOOL)
			{
				p_data+=1;
			}
			else if (tag_type == TAGTYPE_BOOLARRAY)
			{
				WORD array_len = *(WORD*)p_data;
				p_data+=2;
				p_data+=(array_len/8)+1;
			}
			else if (tag_type == TAGTYPE_BLOB) // (never seen; <len> <byte>)
			{
				DWORD len=*(DWORD*)p_data;
				p_data+=4;
				p_data+=len;
			}
			else
			{
				//if (tag.specialtag != 0)
				//	TRACE("%s; Unknown tag: type=0x%02X  specialtag=%u\n", __FUNCTION__, tag.type, tag.specialtag);
				//else
				//	TRACE("%s; Unknown tag: type=0x%02X  name=\"%s\"\n", __FUNCTION__, tag.type, tag.tagname);
			}

			if(specialtag==0x01)  //name
				sr->m_file_name=string_val;
			else if(specialtag==0x15)  //availability
				sr->m_availability=int_val;
			else if(specialtag==0x02)  //size
				sr->m_size=int_val;
			
			


/*			UINT tag_type=*((UINT*)p_data);
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
				sr->m_availability=*((UINT*)p_data);
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
			else{
				ASSERT(p_data<=p_end);  //check just in case.
//				CString log_msg;
//				log_msg.Format("WARNING:  A donkey server is feeding us an unknown tag format %d, and the previous tag format was %d.  Ignoring remaining search results since the parser doesn't know how to continue.\n",tag_type,prev_tag);
//				TRACE((LPCSTR)log_msg);
//				MetaMachineDll::GetDlg()->Log((LPCSTR)log_msg);
				UINT tag_length=*((unsigned short*)(p_data-3));
				p_data+=(tag_length-1);  //skip over the tag, hopefully it doesn't have extra data
				last_after_tag_debug=*((int*)p_data);
				ASSERT(p_data<=p_end);  //check just in case.
				//p_data+=4;  //hope its a DWORD
//				b_ok=false;
				//ASSERT(0);
			}
			*/

			prev_tag=tag_type;
		}

		if(sr->m_size==0){
			TRACE("MetaMachine::SpoofHost::ProcessSearchResults unfinished search result file name='%s', avail=%u\n",sr->m_file_name.c_str(),sr->m_availability);
		}

		if( (sr->m_size%137)==0 || (sr->m_size<m_search_task.m_minimum_size) || sr->m_size<500000 || sr->m_availability<2 || sr->m_file_name.size()<2){  //do preliminary filtering
			delete sr;
			continue;//it was a decoy
		}
		else{
			v_search_results.Add(sr);
		}

		/*
		if((((sr->m_hash[13]>>4)&0x0f)+(sr->m_hash[13]&0x0f))==15 && (((sr->m_hash[14]>>4)&0x0f)+(sr->m_hash[14]&0x0f))==15 && (((sr->m_hash[15]>>4)&0x0f)+(sr->m_hash[15]&0x0f))==15)  
			continue;//yes it was a spoof.
		*/

		//
	}

	log_msg.Format("%s SpoofHost::ProcessSearchResults() Finished processing search results, reporting them",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);

	ref.System()->ReportSearchResults(v_search_results);  //report another hash and its availability

	//check to see if the server indicated that there are more search results.
	if((*p_data)==0x01 && m_search_count<5){  //is there more?  we want as many as possible, but we run into extremely diminishing returns on projects like eminem where it keeps returning more forever.
		log_msg.Format("%s SpoofHost::ProcessSearchResults() Requesting More Search Results",m_ip.c_str());
		SpoofingSystem::LogToFile(log_msg);
		m_search_count++;

		Buffer2000 packet;
		packet.WriteByte(0xe3);  //yeah we are emule.
		packet.WriteDWord(0);  //write in a bogous length to be set after we figure out how much we have really written

		packet.WriteByte(0x21);  //Set the command byte equal to a MORE RESULTS
		packet.WriteDWord(1,packet.GetLength()-5);  //set the length of the packet

		m_send_buffer.WriteBytes(packet.GetBufferPtr(),packet.GetLength());
	}

	log_msg.Format("%s SpoofHost::ProcessSearchResults() END",m_ip.c_str());
	SpoofingSystem::LogToFile(log_msg);
}
