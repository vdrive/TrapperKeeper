#include "StdAfx.h"
#include "kademliaprotectorsystem.h"
#include "..\tkcom\timer.h"
#include "..\tkcom\PeerSocket.h"
#include "..\MetaMachineSpoofer\DonkeyFile.h"
#include "..\tkcom\TinySQL.h"
#include "KadUtilityFunctions.h"

UINT KademliaProtectorSystem::sm_system_count=0;
KademliaProtectorSystem* KademliaProtectorSystem::sm_system=NULL;
CCriticalSection KademliaProtectorSystem::sm_system_lock;

HANDLE sm_log_file_handle=NULL;
UINT sm_log_bytes_written=0;
CCriticalSection sm_log_lock;


KademliaProtectorSystem::KademliaProtectorSystem(void)
{
}

KademliaProtectorSystem::~KademliaProtectorSystem(void)
{
	CloseHandle(sm_log_file_handle);
	sm_log_file_handle=NULL;
}

UINT KademliaProtectorSystem::Run(void)
{
	//Timer contact_update;

	m_kad_contact_manager.StartUp();


	//add some bootstrap contacts just in case we don't have any
	//byte hashes[50][16];
	//for(int i=0;i<50;i++){
	//	for(int j=0;j<16;j++){
	//		hashes[i][j]=rand()%255;
	//	}
	//}

	//Vector v_boot_contacts;
	//CTime now=CTime::GetCurrentTime();
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[0]),inet_addr("69.196.3.158"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[1]),inet_addr("82.213.150.150"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[2]),inet_addr("213.96.217.19"),4872,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[3]),inet_addr("141.30.11.33"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[4]),inet_addr("151.44.118.147"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[5]),inet_addr("164.77.103.250"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[6]),inet_addr("164.77.241.236"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[7]),inet_addr("172.211.225.148"),4672,now));
	//v_boot_contacts.Add(new KadContact(CUInt128(hashes[8]),inet_addr("200.56.142.212"),4672,now));

	//m_kad_contact_manager.AddContacts(v_boot_contacts);

	bool b_has_loaded_swarms=false;
	Timer swarm_update;

	srand(GetTickCount());

	int initial_swarm_load_delay=rand()%1800;

	while(!this->b_killThread){
		Sleep(100);
		//if(contact_update.HasTimedOut(10)){
		//	LogToFile("KademliaProtectorSystem::Run(void) A BEGIN");
		//	contact_update.Refresh();
		//	m_kad_contact_manager.Update();
		//	LogToFile("KademliaProtectorSystem::Run(void) A END");
		//}
		if(!b_has_loaded_swarms || swarm_update.HasTimedOut(60*60*18) || (swarm_update.HasTimedOut(60*60*3) && mv_swarm_files.Size()==0)){
			if(swarm_update.HasTimedOut(initial_swarm_load_delay)){
				LogToFile("KademliaProtectorSystem::Run(void) B BEGIN");
				ReloadSwarms();
				swarm_update.Refresh();
				b_has_loaded_swarms=true;
				LogToFile("KademliaProtectorSystem::Run(void) B END");
			}
		}
	}
	return 0;
}

void KademliaProtectorSystem::StartSystem(void)
{
	srand(GetTickCount());
	for(int i=0;i<16;i++){
		m_my_hash[i]=rand()%255;
	}


	UINT ipCount=0;
	//char *ip;
	PHOSTENT hostinfo;
	char hostname[1024];

	//in_addr hostIPs[128];
	if( gethostname ( hostname, sizeof(hostname)) == 0)
	{
		TRACE("Host name: %s\n", hostname);

		if((hostinfo = gethostbyname(hostname)) != NULL)
		{
			//int nCount = 0;
			while(hostinfo->h_addr_list[ipCount] && ipCount<MAXSYSTEMIPS)
			{
				mv_system_ips.push_back(inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[ipCount]));
				//memcpy(&hostIPs[ipCount],(struct in_addr *)hostinfo->h_addr_list[ipCount],sizeof(in_addr));
				
				TRACE("KademliaProtectorSystem system IP #%d: %s\n", ipCount, mv_system_ips[ipCount].c_str());
				++ipCount;
			}
		}
	}
	
	for(int i=0;i<NUMBERPORTS;i++)
		m_server_sockets[i].Init(BASEPORT+i,this);


	//specify this as an emule hash
	m_my_hash[5] = 14;  //say we are emule
	m_my_hash[14] = 111;
	m_my_hash[6] = rand()%240;  //encode a value that will denote this as a MD hash so we don't add ourselves to our contact list
	m_my_hash[7] = m_my_hash[6]+7;
	m_my_hash[8] = m_my_hash[6]+13;

	m_kad_udp_system.StartKadUDPSystem();
	m_client_manager.StartThreadLowPriority();

	this->StartThreadLowPriority();

	m_decoy_pool.StartThreadLowPriority();
}

void KademliaProtectorSystem::StopSystem(void)
{
	m_decoy_pool.b_killThread=1;
	m_kad_udp_system.StopKadUDPSystem();
	m_kad_contact_manager.Shutdown();
	m_client_manager.Shutdown();
	this->StopThread();
	m_decoy_pool.StopThread();
}

KadContactManager* KademliaProtectorSystem::GetContactManager(void)
{
	return &m_kad_contact_manager;
}

const char* KademliaProtectorSystem::GetMyIP(void)
{
	if(mv_system_ips.size()>0)
		return mv_system_ips[0].c_str();
	else return "0.0.0.0";
}

byte* KademliaProtectorSystem::GetMyHash(void)
{
	return m_my_hash;
}

void KademliaProtectorSystem::OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection)
{
	
	PeerSocket *ps=(PeerSocket*)new_connection;
	TRACE("Accepted new connection from %s\n",ps->GetIP());

	CString csip=ps->GetIP();

	int first_bar=csip.Find(".");
	if(first_bar!=-1){
		int second_bar=csip.Find(".",first_bar+1);
		if(second_bar!=-1){
			int third_bar=csip.Find(".",second_bar+1);
			if(third_bar!=-1){

				CString sclassA=csip.Mid(0,first_bar);
				CString sclassB=csip.Mid(first_bar+1,second_bar-first_bar-1);
				CString sclassC=csip.Mid(second_bar+1,third_bar-second_bar-1);
				CString sclassD=csip.Mid(third_bar);

				int a=atoi(sclassA);
				int b=atoi(sclassB);
				int c=atoi(sclassC);
				int d=atoi(sclassD);

				//do something to cut down on the client load we have

				if(a!=24 && a!=216 && a!=64 && a!=38 && a!=62 && a!=66 && a!=68 && a!=213 && a!=208 && a!=216 && a!=81){
					TRACE("Destroying new connection from %s due to bad IP\n",ps->GetIP());
					delete ps;
					return;
				}

//				if( a==195 || a>216){
//					delete ps;
//					TRACE("Destroying new connection from %s due to insufficient credits\n",ps->GetIP());
//					return;
//				}
			}
		}
	}


	//do something to cut down on the client load we have
	if(!m_client_manager.HasEnoughCreditToConnect(ps->GetIP())){
		delete ps;
		TRACE("Destroying new connection from %s due to insufficient credits\n",ps->GetIP());
		return;
	}

	unsigned short server_port=BASEPORT;
	for(int i=0;i<NUMBERPORTS;i++){
		if(&m_server_sockets[i]==server_socket){
			server_port=BASEPORT+i;
			break;
		}
	}

	CString my_name;
	int len=3+rand()%6;
	for(int i=0;i<len;i++)
		my_name+=(char)('a'+(rand()%25));
	CString tmp_name;
	tmp_name.Format("%s%d",my_name,len);

	m_client_manager.UseCredit(ps->GetIP());
	m_client_manager.UseCredit(ps->GetIP());

	m_client_manager.AddClient(new KadFTConnection(new_connection,server_port,m_my_hash,string((LPCSTR)tmp_name)));
	//delete ps;
}

void KademliaProtectorSystem::ReloadSwarms(void)
{
//ed2k://|file|Eminem%20-%20Like%20Toy%20Soldiers%20(15).mp3|5919568|3522F4E18070B244F09943F4EB3481B8|/
	TRACE("KademliaProtectorSystem::ReloadSwarms(void) BEGIN\n");
	static Timer last_successful_query;
	TinySQL sql;
	if(!sql.Init("63.216.246.62","onsystems","tacobell","metamachine",3306)){
		if(last_successful_query.HasTimedOut(60*60*48)){
			//we haven't had access to the database for some time, so we should stop protection to prevent 'lost' racks from protecting old content
			m_kad_udp_system.GetPublisher()->UnpublishFiles();
		}
		TRACE("KademliaProtectorSystem::ReloadSwarms(void) Database connection failed\n");
		return;
	}

//***** IMPORT
	if(!sql.Query("select count(*) from swarm_table",true)){  //TYDEBUG
		TRACE("KademliaProtectorSystem::ReloadSwarms(void) Database connection failed [0]\n");
		return;
	}

	if(sql.m_num_rows<1){
		TRACE("KademliaProtectorSystem::ReloadSwarms(void) Database connection failed [0]\n");
		return;
	}

	int NUMSWARMS=10000;

	int offset=0;

	
	int count=atoi(sql.mpp_results[0][0].c_str());

	if(count>NUMSWARMS){
		offset=rand()%(max(1,max(1,count-(NUMSWARMS/3))/100));
		offset*=100;  //offset was scaled down by 100 because rand() only goes up to 65k

		offset=max(0,offset-(NUMSWARMS/3)); //we do this so that we have a good chance of selecting the ones in front and the back, since we are randoming for a range.  If we didn't do this there would be about 1/150000 chance that the start would be picked, not good
		offset=min(offset,max(0,count-NUMSWARMS));
	}

	CString query;
	query.Format("select file_name,size,hash,project,hash_set from swarm_table limit %u,%u",offset,NUMSWARMS);
//***** IMPORT


	if(!sql.Query(query,true)){
		if(last_successful_query.HasTimedOut(60*60*48)){
			//we haven't had access to the database for some time, so we should stop protection to prevent 'lost' racks from protecting old content
			m_kad_udp_system.GetPublisher()->UnpublishFiles();
		}
		TRACE("KademliaProtectorSystem::ReloadSwarms(void) Database query failed\n");
		return;
	}

	TRACE("KademliaProtectorSystem::ReloadSwarms(void) Swarm query successful\n");


	last_successful_query.Refresh();

	m_kad_udp_system.GetPublisher()->UnpublishFiles();

	Vector v_tmp_swarms;

	for(int query_index=0;query_index<(int)sql.m_num_rows;query_index++){
		const char *hash=sql.mpp_results[query_index][2].c_str();

		int file_size=atoi(sql.mpp_results[query_index][1].c_str());
		const char *file_name=sql.mpp_results[query_index][0].c_str();
		const char* project=sql.mpp_results[query_index][3].c_str();
		const char* hash_set=sql.mpp_results[query_index][4].c_str();

		

	//	for(int i=0;i<32;i++){
	//		hash[i]='B';
	//	}

		CString str_hash=hash;

		if(str_hash.GetLength()<32)
			return;  //not valid data
		byte byte_hash[16];
		byte *byte_hash_set=NULL;
		
		for(int j=0;j<16;j++){
			char ch1=str_hash[j*2];
			char ch2=str_hash[j*2+1];
			byte val1=KadUtilityFunctions::ConvertCharToInt(ch1);
			byte val2=KadUtilityFunctions::ConvertCharToInt(ch2);
			byte hash_val=((val1&0xf)<<4)|(val2&0xf);
			byte_hash[j]=hash_val;
		}
		UINT the_size=file_size;

		int num_parts=the_size/PARTSIZE;
		if(num_parts>0){
			num_parts++;
		}

		if(num_parts>0 && the_size>=PARTSIZE){  //try to set the hash set data
			CString str_hash_set=hash_set;
			if(str_hash_set.GetLength()!=num_parts*32){
				continue;  //we don't want to add this swarm, since we don't have all the data we need (hashset)
			}
			byte_hash_set=new byte[num_parts*16];
			for(int part_index=0;part_index<num_parts;part_index++){
				for(int j=0;j<16;j++){
					char ch1=str_hash_set[part_index*32+j*2];
					char ch2=str_hash_set[part_index*32+j*2+1];
					byte val1=KadUtilityFunctions::ConvertCharToInt(ch1);
					byte val2=KadUtilityFunctions::ConvertCharToInt(ch2);
					byte hash_val=((val1&0xf)<<4)|(val2&0xf);
					byte_hash_set[part_index*16+j]=hash_val;
				}				
			}				
		}

		bool b_add=true;

		for(int i=0;i<(int)v_tmp_swarms.Size();i++){  //check if we have already added this swarm
			DonkeyFile *df=(DonkeyFile*)v_tmp_swarms.Get(i);
			if(memcmp(df->m_hash,byte_hash,16)==0){
				b_add=false;
				break;
			}
		}
		
		if(b_add){
			//TRACE("KademliaProtectorSystem::ReloadSwarms(void) Loaded New Swarm %s, %d\n",file_name,the_size);
			v_tmp_swarms.Add(new DonkeyFile(file_name,byte_hash,the_size,byte_hash_set));
			m_kad_udp_system.GetPublisher()->AddFileToPublishFileList(new KadFile(file_name,byte_hash,the_size));
		}

		if(byte_hash_set)
			delete []byte_hash_set;
	}

	//end of loop, copy and sort the vector
	CSingleLock lock(&m_swarm_lock,TRUE);
	mv_swarm_files.Copy(&v_tmp_swarms);
	mv_swarm_files.Sort(1);

	TRACE("KademliaProtectorSystem::ReloadSwarms(void) FINISHED\n");
}


bool KademliaProtectorSystem::GetDecoyFile(Vector& v, byte* hash){
	return m_decoy_pool.GetDecoyFile(v,hash);
}

bool KademliaProtectorSystem::GetSwarmFile(Vector& v, byte* hash)
{
	CSingleLock lock(&m_swarm_lock,TRUE);

	DonkeyFile df("x",hash,1,NULL);  //a reference we can use to find what we are looking for
	int index=-1;
	index=mv_swarm_files.BinaryFind(&df);

	if(index==-1)
		return false;
	else{
		v.Add(mv_swarm_files.Get(index));
		return true;
	}
}

UINT KademliaProtectorSystem::GetFTClientCount(void)
{
	return m_client_manager.GetClientCount();
}

//call this only from the gui thread
void KademliaProtectorSystem::GuiClearTerminalFTClients(void)
{
	m_client_manager.GuiClearTerminalClients();
}

UINT KademliaProtectorSystem::GetSwarmCount(void)
{
	return mv_swarm_files.Size();
}

void KademliaProtectorSystem::LogToFile(const char* str)
{
	//maintain synchronous access to this function
/*
	CSingleLock lock(&sm_log_lock,TRUE);

	if(sm_log_bytes_written>6000000){  //every X amount of data, reset the log to prevent uncontrollable growth.
		CloseHandle(sm_log_file_handle);
		sm_log_file_handle=NULL;
		sm_log_bytes_written=0;
	}
	if(sm_log_file_handle==NULL){
		CreateDirectory("c:\\KadProtector",NULL);
		CreateDirectory("c:\\KadProtector\\Logs",NULL);
		sm_log_file_handle = CreateFile("c:\\KadProtector\\Logs\\debug_log.txt",			// open file at local_path 
				GENERIC_WRITE,              // open for writing 
				FILE_SHARE_READ,							// we don't like to share
				NULL,						// no security 
				CREATE_ALWAYS,				// create new whether file exists or not
				FILE_ATTRIBUTE_NORMAL,		// normal file 
				NULL);						// no attr. template 
	}
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s:  %s\r\n",cur_time.Format("%A %I:%M %p"),str);
	DWORD tmp;
	sm_log_bytes_written+=output.GetLength();
	WriteFile(sm_log_file_handle,output.GetBuffer(output.GetLength()),output.GetLength(),&tmp,NULL);
	*/
}
ClientManager* KademliaProtectorSystem::GetClientManager(void)
{
	return &m_client_manager;
}

KadUDPListener* KademliaProtectorSystem::GetKadUDPSystem(void)
{
	return &m_kad_udp_system;
}
