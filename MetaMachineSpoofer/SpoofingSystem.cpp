#include "StdAfx.h"
#include "spoofingsystem.h"
#include "SpoofHost.h"
//#include "MetaSpooferTCP.h"
#include "MetaMachineSpooferDll.h"
#include "ClientConnection.h"
#include "InterdictionConnection.h"
#include "..\tkcom\tinysql.h"
#include "Spoof.h"
#include "..\tkcom\PeerSocket.h"
//#include <afxmt.h>

#define CONNECTIONLIMIT 250

UINT SpoofingSystem::sm_system_count=0;
SpoofingSystem* SpoofingSystem::sm_system=NULL;
CCriticalSection SpoofingSystem::sm_system_lock;
SpoofingSystem MetaSpooferReference::mp_system;

#define ED2KGETSWARMS					1
#define ED2KADDSOURCE					2
#define ED2KREMOVESOURCE				3
#define ED2KGETINTERDICTIONTARGETS		4
#define ED2KGETSOURCEHASHES				5
#define ED2KGETFALSEDECOYS				6
#define ED2KGETSPOOFS					7
#define ED2KRACKSERVERLOG				8
#define ED2KSERVERENTRY					9
#define ED2KDECOYLOG					10
#define ED2KSEARCHTASK					11
#define ED2KSEARCHRESULTS				12
#define ED2KCONTROLKEY					0xe4

HANDLE sm_log_file_handle=NULL;
UINT sm_log_bytes_written=0;
CCriticalSection sm_log_lock;

SpoofingSystem::SpoofingSystem(void)
{
	//m_log_bytes_written=0;
	mb_halt_system=true;
	m_emule_version=44160;
//	m_queued_data_size=0;
//	mb_needs_decoy_load=true;
//	mb_needs_false_decoy_creation=true;
//	mb_done_initing_servers=false;
	CreateDirectory("c:\\syncher",NULL);
	CreateDirectory("c:\\syncher\\rcv",NULL);
	CreateDirectory("c:\\syncher\\rcv\\Edonkey False_Decoys",NULL);

	mb_weekend=false;
	mb_spoofing_only=false;
//	mb_has_decoys_loaded=false;
///	for(int i=0;i<MULTICONNECTIONCOUNT;i++){
//		m_server_sockets[i].Init(BASEPORT+i,this);
//	}
}

SpoofingSystem::~SpoofingSystem(void)
{
	CloseHandle(sm_log_file_handle);
	sm_log_file_handle=NULL;
}

/*
bool SpoofingSystem::NewConnection(TKTCPConnection &con,UINT server_handle)
{
	//figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(sh->IsServer(con,server_handle)){
			TRACE("MetaSpooferTCP::OnAcceptedConnection() from %s.\n",con.GetPeer());
			sh->NewConnection(con);
			return true;
		}
	}

	SpooferTCPReference reference;
	//figure out if a client is connection back to us to upload to us
	if(server_handle==reference.TCP()->m_servers[0]){
		CSingleLock lock(&m_interdiction_connection_lock,TRUE);
		for(UINT i=0;i<mv_inactive_interdiction_connections.Size();i++){
			InterdictionConnection *cc=(InterdictionConnection*)mv_inactive_interdiction_connections.Get(i);
			if(cc->IsConnection(con.GetPeer()) && cc->IsAwaitingCallBack()){
				cc->ReestablishedConnection(con.GetHandle());
				mv_interdiction_connections.Add(cc);  //add it back into the active list
				mv_inactive_interdiction_connections.Remove(i);  //remove it from the inactive list
				return true;
			}
		}
	}


	if(mv_servers.Size()<1)  // are the servers even initialized yet??
		return false;

	//We don't know who this is.  Let them know what we think of this...
	//TRACE("MetaMachine :  MetaSystem::NewConnection()  A rogue connection from %s has been received.  It is probably a request for a file, we will let it dangle for several minutes.\n",source_ip);
//	mv_rogue.push_back(handle);
	TRACE("New client connection from %s.\n",con.GetPeer());
	if(mb_done_initing_servers && mv_client_connections.Size()<CONNECTIONLIMIT && stricmp(con.GetPeer(),"66.54.78.16")==0){  //only allow so many connections
		SpooferTCPReference reference;
		for(int i=0;i<MULTI_CONNECTION_COUNT;i++){
			if(server_handle==reference.TCP()->m_servers[i]){
				mv_client_connections.Add(new ClientConnection(con,m_my_hashes[i],m_my_user_names[i],BASEPORT+i));  //we have a new client connection
				break;
			}
		}
	}
	else
		return false;

	MetaMachineSpooferDll::GetDlg()->GotConnection();

	return true;
	//SpooferTCPReference tcp;
	//tcp.TCP()->CloseConnection(handle);  
}*/

/*
void SpoofingSystem::NewData(TKTCPConnection &con, Buffer2000& data)
{
	//figure out which client sent us this
	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		if(cc->IsConnection(con)){
			cc->NewData(data);
			return;
		}
	}

	//figure out which client sent us this
	
	for(UINT i=0;i<mv_interdiction_connections.Size();i++){
		InterdictionConnection *cc=(InterdictionConnection*)mv_interdiction_connections.Get(i);
		if(cc->IsConnection(con)){
			cc->NewData(data);
			return;
		}
	}
	

	//it wasn't a client so it must be a server, figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(sh->IsServer(con)){
			sh->NewData(con,data);
			return;
		}
	}

	//We don't know who this is.  Let them know what we think of this...
	//This shouldn't be possible... but just in case.
	//TRACE("MetaMachine :  MetaSystem::NewData()  rogue data from %s has been received!!!!  This isn't possible by the way.  Terminating this freak connection now.\n",source_ip);
	SpooferTCPReference tcp;
	tcp.TCP()->CloseConnection(con);  
}*/

/*
void SpoofingSystem::LostConnection(TKTCPConnection &con)
{
	//figure out which server lost its connection
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(sh->IsServer(con)){
			sh->LostConnection(con);
			return;
		}
	}

	//figure out which client this is for
	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		if(cc->IsConnection(con)){
			cc->Shutdown();  //lets destroy this client connection
			mv_client_connections.Remove(i);
			return;
		}
	}

	//figure out which client sent us this
	CSingleLock lock(&m_interdiction_connection_lock,TRUE);
	for(UINT i=0;i<mv_interdiction_connections.Size();i++){
		InterdictionConnection *cc=(InterdictionConnection*)mv_interdiction_connections.Get(i);
		if(cc->IsConnection(con)){
			if(!cc->IsAwaitingCallBack()){
				cc->Shutdown();
				mv_interdiction_connections.Remove(i);
				return;
			}
			else{
				mv_inactive_interdiction_connections.Add(cc);  //add it to a seperate list of inactive connections, since this list will be much larger.  Its more efficient when we get new data calls
				mv_interdiction_connections.Remove(i);  //remove it from our main one
				return;
			}
		}
	}
}
*/

void SpoofingSystem::Init(Dll *pDLL)
{
	TRACE("MataMachineSpooofer:  SpoofingSystem::Init().\n");

	InitEvilSeedData();  //initialize the header for all the ultimate decoys

	mb_spoofing_only=false;

	OFSTRUCT of;
	ZeroMemory(&of,sizeof(of));
	if( OpenFile( "c:\\syncher\\rcv\\executables\\ed2k_spoofing_only.txt" , &of , OF_EXIST ) != HFILE_ERROR ){
		mb_spoofing_only=true;  //the file exists, we shouldn't do any file transfers from this client
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
				
				TRACE("SpoofingSystem IP #%d: %s\n", ipCount, mv_system_ips[ipCount].c_str());
				++ipCount;
			}
		}
	}
	
	m_com.Register(pDLL,1105);

	//m_request_logger.StartThread();

	for(int i=0;i<MULTI_CONNECTION_COUNT;i++){
		m_server_sockets[i].Init(BASEPORT+i,this,mv_system_ips);
	}

	srand(10);  //fixed seed so we generate the same garbage data on each machine

	for(int i=0;i<500000;i++){  //make some uncompressible data
		m_garbage_data.WriteByte(rand());
	}

	srand(timeGetTime());  //true random seed

	//if(rand()&1){
	//	m_emule_version=43520;
	//}
	//else{
		m_emule_version=44160;
	//}
	
	//init my hash and user name

	for(int ip_index=0;ip_index<MAXSYSTEMIPS;ip_index++){
		for(int i=0;i<MULTI_CONNECTION_COUNT;i++){

			//we want to look exactly like a real user
			m_my_user_names[ip_index][i]="http://emule-project.net";
			/*
			int rand_num=rand()%71;
			switch(rand_num){
				case 0:  m_my_user_names[ip_index][i]="zIoN"; break;
				case 1:  m_my_user_names[ip_index][i]="JoeDark"; break;
				case 2:  m_my_user_names[ip_index][i]="craig"; break;
				case 3:  m_my_user_names[ip_index][i]="ZeroG"; break;
				case 4:  m_my_user_names[ip_index][i]="JediMaster"; break;
				case 5:  m_my_user_names[ip_index][i]="BomB"; break;
				case 6:  m_my_user_names[ip_index][i]="algore"; break;
				case 7:  m_my_user_names[ip_index][i]="johnb"; break;
				case 8:  m_my_user_names[ip_index][i]="sarahlee"; break;
				case 9:  m_my_user_names[ip_index][i]="twinkie"; break;
				case 10:  m_my_user_names[ip_index][i]="jackdaniels"; break;
				case 11:  m_my_user_names[ip_index][i]="russian"; break;
				case 12:  m_my_user_names[ip_index][i]="guitarist63"; break;
				case 13:  m_my_user_names[ip_index][i]="george"; break;
				case 14:  m_my_user_names[ip_index][i]="blackangelxx"; break;
				case 15:  m_my_user_names[ip_index][i]="raiders"; break;
				case 16:  m_my_user_names[ip_index][i]="chocolate"; break;
				case 17:  m_my_user_names[ip_index][i]="darkelf"; break;
				case 18:  m_my_user_names[ip_index][i]="robotech"; break;
				case 19:  m_my_user_names[ip_index][i]="sable78"; break;
				case 20:  m_my_user_names[ip_index][i]="chan"; break;
				case 21:  m_my_user_names[ip_index][i]="madcowdisease"; break;
				case 22:  m_my_user_names[ip_index][i]="badass"; break;
				case 23:  m_my_user_names[ip_index][i]="pirate"; break;
				case 24:  m_my_user_names[ip_index][i]="ninja"; break;
				case 25:  m_my_user_names[ip_index][i]="sinner"; break;
				case 26:  m_my_user_names[ip_index][i]="pinhead"; break;
				case 27:  m_my_user_names[ip_index][i]="movin"; break;
				case 28:  m_my_user_names[ip_index][i]="cowboy"; break;
				case 29:  m_my_user_names[ip_index][i]="edonkeyisbest"; break;
				case 30:  m_my_user_names[ip_index][i]="www.bittorrent.com"; break;
				case 31:  m_my_user_names[ip_index][i]="www.mit.edu"; break;
				case 32:  m_my_user_names[ip_index][i]="police"; break;
				case 33:  m_my_user_names[ip_index][i]="madmax"; break;
				case 34:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 35:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 36:  m_my_user_names[ip_index][i]="opensource"; break;
				case 37:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 38:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 39:  m_my_user_names[ip_index][i]="opensource"; break;
				case 40:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 41:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 42:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 43:  m_my_user_names[ip_index][i]="opensource"; break;
				case 44:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 45:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 46:  m_my_user_names[ip_index][i]="destinyschild"; break;
				case 47:  m_my_user_names[ip_index][i]="49cents"; break;
				case 48:  m_my_user_names[ip_index][i]="epa"; break;
				case 49:  m_my_user_names[ip_index][i]="fbi"; break;
				case 50:  m_my_user_names[ip_index][i]="dps"; break;
				case 51:  m_my_user_names[ip_index][i]="circe"; break;
				case 52:  m_my_user_names[ip_index][i]="kaazama"; break;
				case 53:  m_my_user_names[ip_index][i]="l33t"; break;
				case 54:  m_my_user_names[ip_index][i]="badman"; break;
				case 55:  m_my_user_names[ip_index][i]="gramps"; break;
				case 56:  m_my_user_names[ip_index][i]="mammal"; break;
				case 57:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 58:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 59:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 60:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 61:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 62:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 63:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 64:  m_my_user_names[ip_index][i]="superbitch"; break;
				case 65:  m_my_user_names[ip_index][i]="gwb"; break;
				case 66:  m_my_user_names[ip_index][i]="teletubby"; break;
				case 67:  m_my_user_names[ip_index][i]="shedevil"; break;
				case 68:  m_my_user_names[ip_index][i]="argh"; break;
				case 69:  m_my_user_names[ip_index][i]="lyre"; break;
				case 70:  m_my_user_names[ip_index][i]="robin"; break;
				default:  m_my_user_names[ip_index][i]="isuck";
			}
			CString tmp;
			tmp.Format("%s%d",m_my_user_names[ip_index][i].c_str(),rand());
			m_my_user_names[ip_index][i]=tmp;
			*/
		}
	}

	CreateDirectory("c:\\MetaMachine",NULL);
	CreateDirectory("c:\\MetaMachine\\Send_Queue",NULL);
	CreateDirectory("c:\\MetaMachine\\User_Hashes",NULL);

	for(ip_index=0;ip_index<MAXSYSTEMIPS;ip_index++){
		for(UINT j=0;j<MULTI_CONNECTION_COUNT;j++){
			CString file_name;
			file_name.Format("c:\\MetaMachine\\User_Hashes\\hash_v6_%d_%d.dat",ip_index,j);
			int b_recreate=(rand()%5)==0;

			if(b_recreate || !GetUserHash(m_my_hashes[ip_index][j],file_name)){  //lets try to use our old user hash if possible, as various parts of emule are starting to track user hash changes
				
				TRACE("SpoofingSystem::Init() Couldn't open user hash %s, so it is creating one randomly.\n",file_name);
				for(UINT i=0;i<16;i++){  //lets make an md4 hash!  sweet.
					m_my_hashes[ip_index][j][i]=(byte)(rand()%255);
				}
				
				//specify this as an emule hash
				m_my_hashes[ip_index][j][5] = 14;//'M';  //if M,L that implies we are mldonkey
				m_my_hashes[ip_index][j][14] = 111;//'L';

				//save this one that we just randomly created
				WriteUserHash(m_my_hashes[ip_index][j],file_name);
				
			}
		}
	}

	CleanDirectory("c:\\metamachine\\send_queue");  //delete any unprocessed send queue files due to a crash etc.
	
	this->StartThread();
	_ASSERTE( _CrtCheckMemory( ) );
	//_CrtDumpMemoryLeaks();
}

//work function that controls the entire program
void SpoofingSystem::Update(void)
{
	_ASSERTE( _CrtCheckMemory( ) );
//	return;
	//TRACE("SpoofingSystem::Update() STARTED.\r\n");
	LogToFile("SpoofingSystem::Update() BEGIN.\r\n");
	
//	static Timer last_queued_data_size_update;

//	if(last_queued_data_size_update.HasTimedOut(15)){	//every 20 seconds we will query the com system and see how much data is really queued up.
//		SpooferTCPReference tcp;
//		m_queued_data_size=tcp.TCP()->GetQueuedDataSize();
//		last_queued_data_size_update.Refresh();
//	}
	if(mb_halt_system){
		LogToFile("SpoofingSystem::Update() END[0].\r\n");
		return;
	}
	srand(timeGetTime());

	//LogToFile("SpoofingSystem::Update() STARTED.\r\n");

	static Timer create_decoys_timer;
	static Timer m_search_timer;
	static bool b_first_search_task=false;
	static int new_search_task_delay=60*(18+(rand()%15));

	if(m_search_timer.HasTimedOut(new_search_task_delay) || !b_first_search_task){  //TYDEBUG
		LogToFile("SpoofingSystem::Update() Requesting new search task BEGIN.");
		RequestNewSearchTaskFromServer();
		LogToFile("SpoofingSystem::Update() Requesting new search task END.");
		b_first_search_task=true;
		new_search_task_delay=60*(18+(rand()%15));  //make each rack request search tasks slightly randomly so no cyclic pattern results.  this stems from some racks having better searching capabilities because they are less banned.
		m_search_timer.Refresh();
	}
	

//	b_update_clients=!b_update_clients;

	LogToFile("SpoofingSystem::Update() Updating Clients BEGIN.");

	ml_client_connections.StartIteration();
	ClientConnection *cc=NULL;
	while((cc=(ClientConnection*)ml_client_connections.GetCurrent())!=NULL){
		if(cc->IsOld()){
			LogToFile("SpoofingSystem::Update() Removing Client BEGIN.");
			cc->Shutdown();
			ml_client_connections.RemoveCurrentAndAdvance();
			LogToFile("SpoofingSystem::Update() Removing Client END.");
		}
		else{
			cc->Update();
			ml_client_connections.Advance();
		}
	}

	LogToFile("SpoofingSystem::Update() Updating Clients END.");


	for(UINT i=0;i<mv_interdiction_connections.Size();i++){
		InterdictionConnection *cc=(InterdictionConnection*)mv_interdiction_connections.Get(i);
		if(!cc->IsDead()){
			cc->Update();
		}
		else{
			cc->Shutdown();
			mv_interdiction_connections.Remove(i);
			i--;
		}
	}

	

	for(UINT i=0;i<mv_inactive_interdiction_connections.Size();i++){
		InterdictionConnection *cc=(InterdictionConnection*)mv_inactive_interdiction_connections.Get(i);
		if(cc->IsDead()){
			cc->Shutdown();
			mv_inactive_interdiction_connections.Remove(i);
			i--;
		}
	}

	static int query_process_delay=60;//60*20+rand()%1200;   //CHANGEDEBUG
	static Timer last_query_process;
	static unsigned long last_interdiction_add=timeGetTime();

	LogToFile("SpoofingSystem::Update() Updating Servers BEGIN.");
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		sh->Update();

		if(sh->IsOld()){
			LogToFile("SpoofingSystem::Update() Removing Old Server BEGIN.");
			LogToFile("SpoofingSystem::Update() Creating Replacement Server BEGIN.");
			SpoofHost *nsh=new SpoofHost(sh->GetIP(),sh->GetPort(),sh->GetBasePort(),sh->GetMyHash(),sh->GetMyUserName(),rand()%1200);
			nsh->SetBindIP(sh->GetBindIP());
			LogToFile("SpoofingSystem::Update() Creating Replacement Server END.");
			mv_servers.Remove(i);
			i--;
			LogToFile("SpoofingSystem::Update() Adding Replacement Server.");
			mv_servers.Add(nsh);
			LogToFile("SpoofingSystem::Update() Removing Old Server END.");
		}
	}
	LogToFile("SpoofingSystem::Update() Updating Servers END.");

	LogToFile("SpoofingSystem::Update() SpoofData Update BEGIN.");
	m_spoof_data.Update();
	LogToFile("SpoofingSystem::Update() SpoofData Update END.");
	LogToFile("SpoofingSystem::Update() SwarmData Update BEGIN.");
	m_swarm_data.Update();
	LogToFile("SpoofingSystem::Update() SwarmData Update END.");
	
	//every half a second we want to try to inderdict another user
	if(last_interdiction_add+250<timeGetTime() && mv_interdiction_connections.Size()<50 && mv_inactive_interdiction_connections.Size()<3000){  
		for(int queue_index=0;queue_index<(int)mv_interdiction_queue.Size();queue_index++){
			InterdictionConnection *nic=(InterdictionConnection *)mv_interdiction_queue.Get(queue_index);
			bool b_found=false;

			//don't duplicate interdiction attempts
			for(int j=0;j<(int)mv_interdiction_connections.Size();j++){
				InterdictionConnection *ic=(InterdictionConnection *)mv_interdiction_connections.Get(j);
				if(ic->IsConnection(nic->GetIP())){
					b_found=true;
					break;
				}
			}

			//INEFFICIENCY.  This vector can theoretically become rather large, we need to be hashing these connections for quick lookup or this will use up a bit of cpu
			//don't duplicate interdiction attempts
			for(int j=0;j<(int)mv_inactive_interdiction_connections.Size() && !b_found;j++){
				InterdictionConnection *ic=(InterdictionConnection *)mv_inactive_interdiction_connections.Get(j);
				if(ic->IsConnection(nic->GetIP())){
					b_found=true;
					break;
				}
			}			

			if(!b_found){
				mv_interdiction_connections.Add(nic);  //add it to our list of interdiction connections
				mv_interdiction_queue.Remove(queue_index);  //remove it from the queue
				queue_index--;
				//TRACE("MetaMachineSpoofer SpoofingSystem::Run()  Added another active interdiction connection to %s, which brings the total to %d active, %d inactive.\n",nic->GetIP(),mv_interdiction_connections.Size(),mv_inactive_interdiction_connections.Size());
				nic->Init();  //init this new connection
				break;
			}
			else{  //remove it from the queue
				mv_interdiction_queue.Remove(queue_index);
				queue_index--;  
				//keep going until we add a new one
			}
		}
		last_interdiction_add=timeGetTime();//.Refresh();
	}
	if(last_query_process.HasTimedOut(query_process_delay)){
		query_process_delay=(60*45)+(60*(rand()%15));  //about an hour and a half

		LogToFile("SpoofingSystem::Update() GetHashesThatNeedSources BEGIN.");
		GetHashesThatNeedSources();
		LogToFile("SpoofingSystem::Update() GetHashesThatNeedSources END.");

		if(mv_interdiction_queue.Size()<3000 && !this->b_killThread){
			//GetInterdictionTargets();
		}

		last_query_process.Refresh();
	}

	//TRACE("SpoofingSystem::Update() FINISHED.\r\n");
	LogToFile("SpoofingSystem::Update() END.\r\n");
	_ASSERTE( _CrtCheckMemory( ) );
	//_CrtDumpMemoryLeaks();
}

void SpoofingSystem::GetConnectedServers(vector <string>& v_servers)
{
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(sh->IsConnected()){
			CString str;
			CString locked="";
//			if(sh->IsThreadLockedUp()){
//				locked="Server Thread Is Stuck!";
//			}
			str.Format("%s : %d : (%d files uploaded) (mode=%d) %s",sh->GetIP(),sh->GetPort(),sh->GetUploadedFiles(),sh->GetState(),locked);	
			v_servers.push_back(string((LPCSTR)str));
		}
	}
}

void SpoofingSystem::GetDisconnectedServers(vector <string>& v_servers)
{
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(!sh->IsConnected()){
			CString str;
			CString locked="";
//			if(sh->IsThreadLockedUp()){
	//			locked="Server Thread Is Stuck!";
//			}
			str.Format("%s : %d (mode=%d) %s",sh->GetIP(),sh->GetPort(),sh->GetState(),locked);	
			v_servers.push_back(string((LPCSTR)str));
		}
	}
}

void SpoofingSystem::AddServer(const char* ip, UINT port,bool b_must_add,bool b_low_priority)
{
	int add_probability=100;
//	if(b_low_priority)
//		add_probability=6;

	for(UINT i=0;i<mv_servers.Size() && !this->b_killThread;i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(stricmp(sh->GetIP(),ip)==0)
			return;
	}
	if(mv_system_ips.size()==0){

		if((rand()%100)<add_probability || b_must_add ){  //80 percent chance we'll add this server.  This keeps every rack from being hooked to every server.
			
			//add multiple instances of this server
//			SpooferTCPReference reference;

			int connection_count=MULTI_CONNECTION_COUNT;

			//add multiple instances, each one will pretend its a client listening on a different port on a NAT system.
			for(int i=0;i<connection_count;i++){
				TRACE("ADDING SERVER %s:%d on port %d.\n",ip,port,BASEPORT+i);
				SpoofHost *nsp=new SpoofHost(ip,port,BASEPORT+i,m_my_hashes[0][i],m_my_user_names[0][i],(rand()%((TYDEBUGMODE)?(10+(rand()%60)):7200)));
				mv_servers.Add(nsp);
			}
		}
	}
	else{
		for(int system_ip_index=0;system_ip_index<(int)mv_system_ips.size();system_ip_index++){
			if((rand()%100)<add_probability || b_must_add ){  //80 percent chance we'll add this server.  This keeps every rack from being hooked to every server.
				
				//add multiple instances of this server
	//			SpooferTCPReference reference;

				int connection_count=MULTI_CONNECTION_COUNT;
				//add multiple instances, each one will pretend its a client listening on a different port on a NAT system.
				for(int i=0;i<connection_count;i++){
					TRACE("ADDING SERVER %s:%d bound to %s on port %d.\n",ip,port,mv_system_ips[system_ip_index].c_str(),BASEPORT+i);
					SpoofHost *nsp=new SpoofHost(ip,port,BASEPORT+i,m_my_hashes[system_ip_index][i],m_my_user_names[system_ip_index][i],(rand()%((TYDEBUGMODE)?(10+(rand()%60)):7200)));  // TY DEBUG
					nsp->SetBindIP(mv_system_ips[system_ip_index].c_str());
					mv_servers.Add(nsp);
				}
			}
		}
	}
	
}

void SpoofingSystem::Shutdown(void)
{
//	m_request_logger.StopThread();
	this->StopThread();
	m_false_decoy_data.StopThread();

//	CSingleLock lock(&m_interdiction_connection_lock,TRUE);
//	for(UINT i=0;i<mv_servers.Size();i++){
//		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
//		sh->b_killThread=1;
//	}

	for(UINT i=0;i<MULTI_CONNECTION_COUNT;i++){
		m_server_sockets[i].Close();
	}

	ClientConnection *cc=NULL;
	ml_client_connections.StartIteration();
	while((cc=(ClientConnection*)ml_client_connections.GetCurrent())!=NULL){
		cc->Shutdown();
		ml_client_connections.Advance();
	}

	for(UINT i=0;i<mv_interdiction_connections.Size();i++){
		InterdictionConnection *cc=(InterdictionConnection*)mv_interdiction_connections.Get(i);
		cc->Shutdown();
	}

	mv_interdiction_connections.Clear();
	mv_inactive_interdiction_connections.Clear();
	mv_servers.Clear();
	ml_client_connections.Clear();
}

void SpoofingSystem::LoadDecoys(const char* in_directory,Vector &v_decoy_vector)
{
	WIN32_FIND_DATA info;

	CString tmp=in_directory;
	CString orig_path=tmp;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	CString name;//=path+info.cFileName;
	while(FindNextFile(hFind,&info) && !this->b_killThread){ //add all the rest and check for trapper keeper closing in the middle
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		name=path+info.cFileName;
		if(((GetFileAttributes(name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){  //it is a directory
			LoadDecoys(name,v_decoy_vector);
		}
		else{ //it is a file
			v_decoy_vector.Add(new DonkeyFile(orig_path,info.cFileName));
			Sleep(5);  //sleep for a little bit so we don't hog the cpu running md4 calculations on edonkey files
		}
	}
	FindClose(hFind);
}

/*
void SpoofingSystem::LoadSwarms(const char* in_directory,Vector &v_swarm_vector)
{
	WIN32_FIND_DATA info;

	CString tmp=in_directory;
	CString orig_path=tmp;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	CString name;//=path+info.cFileName;
	while(FindNextFile(hFind,&info) && !this->b_killThread){ //add all the rest and check for trapper keeper closing in the middle
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		name=path+info.cFileName;
		if(((GetFileAttributes(name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){  //it is a directory
			LoadSwarms(name,v_swarm_vector);
		}
		else{ //it is a file
			v_swarm_vector.Add(new DonkeyFile(orig_path,info.cFileName));
			Sleep(20);  //sleep for a little bit so we don't hog the cpu running md4 calculations on edonkey files
		}
	}
	FindClose(hFind);
}*/

UINT SpoofingSystem::Run(){

	LogToFile("SpoofingSystem::Run() BEGIN");
	_ASSERTE( _CrtCheckMemory( ) );
	TRACE("SpoofingSystem::Run() STARTED.\n");

	Sleep(1500);  //dont start crunching until a moment after trapper keeper finishes launching.
	srand(timeGetTime());
	int pause_time=rand()%1800;  //between 0 and 30 minute delay.  so that lots of racks don't rush the servers all at once
	
	Timer initial_pause;

	//where the startup delay happens.
	while(!initial_pause.HasTimedOut(pause_time) && !this->b_killThread && !TYDEBUGMODE){
		Sleep(50);
	}

	TinySQL sql;
	sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);

	sql.Query("select name from user_names order by rand() limit 100",true);

	vector <string>v_user_names;

	for(int i=0;i<(int)sql.m_num_rows;i++){
		v_user_names.push_back(sql.mpp_results[i][0].c_str());
	}

	if(v_user_names.size()==0){  //if no available names, add default name
		v_user_names.push_back("http://emule-project.net");
	}

	for(int ip_index=0;ip_index<MAXSYSTEMIPS;ip_index++){
		for(int i=0;i<MULTI_CONNECTION_COUNT;i++){

			int rand_index=rand()%(int)v_user_names.size();
			//we want to look exactly like a real user
			m_my_user_names[ip_index][i]=v_user_names[rand_index].c_str();

			/*
			int rand_num=rand()%71;
			switch(rand_num){
				case 0:  m_my_user_names[ip_index][i]="zIoN"; break;
				case 1:  m_my_user_names[ip_index][i]="JoeDark"; break;
				case 2:  m_my_user_names[ip_index][i]="craig"; break;
				case 3:  m_my_user_names[ip_index][i]="ZeroG"; break;
				case 4:  m_my_user_names[ip_index][i]="JediMaster"; break;
				case 5:  m_my_user_names[ip_index][i]="BomB"; break;
				case 6:  m_my_user_names[ip_index][i]="algore"; break;
				case 7:  m_my_user_names[ip_index][i]="johnb"; break;
				case 8:  m_my_user_names[ip_index][i]="sarahlee"; break;
				case 9:  m_my_user_names[ip_index][i]="twinkie"; break;
				case 10:  m_my_user_names[ip_index][i]="jackdaniels"; break;
				case 11:  m_my_user_names[ip_index][i]="russian"; break;
				case 12:  m_my_user_names[ip_index][i]="guitarist63"; break;
				case 13:  m_my_user_names[ip_index][i]="george"; break;
				case 14:  m_my_user_names[ip_index][i]="blackangelxx"; break;
				case 15:  m_my_user_names[ip_index][i]="raiders"; break;
				case 16:  m_my_user_names[ip_index][i]="chocolate"; break;
				case 17:  m_my_user_names[ip_index][i]="darkelf"; break;
				case 18:  m_my_user_names[ip_index][i]="robotech"; break;
				case 19:  m_my_user_names[ip_index][i]="sable78"; break;
				case 20:  m_my_user_names[ip_index][i]="chan"; break;
				case 21:  m_my_user_names[ip_index][i]="madcowdisease"; break;
				case 22:  m_my_user_names[ip_index][i]="badass"; break;
				case 23:  m_my_user_names[ip_index][i]="pirate"; break;
				case 24:  m_my_user_names[ip_index][i]="ninja"; break;
				case 25:  m_my_user_names[ip_index][i]="sinner"; break;
				case 26:  m_my_user_names[ip_index][i]="pinhead"; break;
				case 27:  m_my_user_names[ip_index][i]="movin"; break;
				case 28:  m_my_user_names[ip_index][i]="cowboy"; break;
				case 29:  m_my_user_names[ip_index][i]="edonkeyisbest"; break;
				case 30:  m_my_user_names[ip_index][i]="www.bittorrent.com"; break;
				case 31:  m_my_user_names[ip_index][i]="www.mit.edu"; break;
				case 32:  m_my_user_names[ip_index][i]="police"; break;
				case 33:  m_my_user_names[ip_index][i]="madmax"; break;
				case 34:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 35:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 36:  m_my_user_names[ip_index][i]="opensource"; break;
				case 37:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 38:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 39:  m_my_user_names[ip_index][i]="opensource"; break;
				case 40:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 41:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 42:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 43:  m_my_user_names[ip_index][i]="opensource"; break;
				case 44:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 45:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 46:  m_my_user_names[ip_index][i]="destinyschild"; break;
				case 47:  m_my_user_names[ip_index][i]="49cents"; break;
				case 48:  m_my_user_names[ip_index][i]="epa"; break;
				case 49:  m_my_user_names[ip_index][i]="fbi"; break;
				case 50:  m_my_user_names[ip_index][i]="dps"; break;
				case 51:  m_my_user_names[ip_index][i]="circe"; break;
				case 52:  m_my_user_names[ip_index][i]="kaazama"; break;
				case 53:  m_my_user_names[ip_index][i]="l33t"; break;
				case 54:  m_my_user_names[ip_index][i]="badman"; break;
				case 55:  m_my_user_names[ip_index][i]="gramps"; break;
				case 56:  m_my_user_names[ip_index][i]="mammal"; break;
				case 57:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 58:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 59:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 60:  m_my_user_names[ip_index][i]="xDonkey"; break;
				case 61:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 62:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 63:  m_my_user_names[ip_index][i]="mldonkey"; break;
				case 64:  m_my_user_names[ip_index][i]="superbitch"; break;
				case 65:  m_my_user_names[ip_index][i]="gwb"; break;
				case 66:  m_my_user_names[ip_index][i]="teletubby"; break;
				case 67:  m_my_user_names[ip_index][i]="shedevil"; break;
				case 68:  m_my_user_names[ip_index][i]="argh"; break;
				case 69:  m_my_user_names[ip_index][i]="lyre"; break;
				case 70:  m_my_user_names[ip_index][i]="robin"; break;
				default:  m_my_user_names[ip_index][i]="isuck";
			}
			CString tmp;
			tmp.Format("%s%d",m_my_user_names[ip_index][i].c_str(),rand());
			m_my_user_names[ip_index][i]=tmp;
			*/
		}
	}

	if(!this->b_killThread){
		CreateDecoys();
		m_false_decoy_data.StartThread();
	}

	initial_pause.Refresh();

	//wait awhile for some decoys to be created before we start connecting to servers
	while(!this->b_killThread){
		if(m_false_decoy_data.mb_has_inited)
			break;
		Sleep(100);
	}
	if(!this->b_killThread)
		InitServers();

	mb_halt_system=false;

	TRACE("SpoofingSystem::Run() FINISHED.\n");
	_ASSERTE( _CrtCheckMemory( ) );

	LogToFile("SpoofingSystem::Run() END");
	return 0;
}

void SpoofingSystem::CleanDirectory(const char* directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	
	//make damn sure we are working in the syncher's or the fasttrack directory.
	if(path.Find("c:\\syncher")==-1 && path.Find("c:\\metamachine")==-1)  
		return;

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			CleanDirectory(full_name);  //if it is a directory descend into it and clean it out.
		}

		if(b_directory){
			RemoveDirectory(full_name);
		}
		else{
			DeleteFile(full_name);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}


void SpoofingSystem::GetSomeDecoys(int chance, int limit,Vector& v)
{
//	CSingleLock lock(&m_decoys_lock,TRUE);

	for(int i=0;i<(int)mv_decoys.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(mv_decoys.Get(i));  
		}
	}

	CSingleLock lock(&m_false_decoy_data.m_data_lock,TRUE);
	for(int i=0;i<(int)m_false_decoy_data.mv_false_decoys.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(m_false_decoy_data.mv_false_decoys.Get(i));  
		}
	}


	while((int)v.Size()>max(1,limit)){
		v.Remove(rand()%v.Size());  //randomly remove items from this vector until its less than the limit
	}
}




void SpoofingSystem::InitServers(void)
{
	TRACE("MetaMachine::SpoofingSystem InitServers()\n");
	srand(timeGetTime());

	mv_servers.Clear();



	if(TYDEBUGMODE){
		//ed2k://",38.119.64.100",4661");
		//ed2k://",81.23.250.167",4242");
		//ed2k://",66.90.68.2",6565");
		//ed2k://",193.19.227.24",4661");
		//ed2k://",140.123.108.139",7654");

		AddServer("38.119.64.100",4661,true);
		//AddServer("81.23.250.167",4242,true);
		//AddServer("66.90.68.2",6565,true);
		//AddServer("193.19.227.24",4661,true);
		//AddServer("140.123.108.139",7654,true);
	}
	//AddServer("64.246.54.71",4661,true);
	//AddServer("195.245.244.243",4661,true);
	//AddServer("69.50.187.210",4661,true);
	//AddServer("64.246.54.71",4661,true);
	//AddServer("64.246.54.76",4660,true);
	//AddServer("193.151.73.64",4661,true);
	//AddServer("62.241.53.2",4242,true);
	//AddServer("193.19.227.24",4661,true);
	//AddServer("38.118.163.138",4661,true);
	//AddServer("211.233.41.235",4661,true);  //korea only

	const bool b_add_all_servers=true;  //to toggle debug on and off easily
	if(b_add_all_servers && !TYDEBUGMODE){

		//These servers compare IPs, so its important not to connect to more than one at once
		vector<string> v_suspect_ips;
		vector<unsigned int> v_suspect_ports;

		v_suspect_ips.push_back("195.245.244.243"); v_suspect_ports.push_back(4661);  //razorback 2
		v_suspect_ips.push_back("62.241.53.2"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("69.50.187.210"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("193.151.74.81"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("62.241.53.17"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("62.241.53.16"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("213.158.119.104"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("62.241.53.4"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("193.19.227.24"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("62.241.53.15"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("213.186.60.106"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("213.186.47.84"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("66.111.51.80"); v_suspect_ports.push_back(2305);
		v_suspect_ips.push_back("69.50.161.58"); v_suspect_ports.push_back(4661);
		v_suspect_ips.push_back("80.144.122.36"); v_suspect_ports.push_back(8888);
		v_suspect_ips.push_back("70.84.28.212"); v_suspect_ports.push_back(4242);
		v_suspect_ips.push_back("217.235.214.178");v_suspect_ports.push_back(9955);

		int rand_index=rand()%(int)v_suspect_ips.size();

		//Add only one of the suspect servers
		AddServer(v_suspect_ips[rand_index].c_str(),v_suspect_ports[rand_index]);


		AddServer("38.119.64.100",4661);// our server
		AddServer("61.152.93.254",4661);
		AddServer("61.172.197.197",19780);
		AddServer("62.158.91.130",50135);
		AddServer("63.115.148.102",4661);
		AddServer("63.115.148.103",4661);
		AddServer("63.246.128.110",3306);
		AddServer("63.246.128.120",3306);
		AddServer("63.246.128.140",3306);
		AddServer("63.246.128.150",3306);
		AddServer("63.246.128.160",3306);
		AddServer("65.110.51.200",4232);
		AddServer("66.194.40.98",700);
		AddServer("67.18.131.194",4661);
		AddServer("67.18.131.196",4661);
		AddServer("67.18.197.82",4661);
		AddServer("69.50.243.2",4646);
		AddServer("69.57.144.98",14533);
		AddServer("70.84.24.50",4242);
		AddServer("70.85.10.162",6565);
		AddServer("80.14.0.2",2525);
		AddServer("80.64.187.1",4661);
		AddServer("80.144.83.110",4662);
		AddServer("80.144.183.37",50135);
		AddServer("81.23.250.167",4242);
		AddServer("81.23.250.169",4242);
		AddServer("81.169.176.23",6565);
		AddServer("140.123.108.139",7654);
		AddServer("193.41.142.148",10000);
		AddServer("193.225.80.117",4661);
		AddServer("194.146.227.8",4642);
		AddServer("195.222.32.62",4661);
		AddServer("202.97.173.139",4661);
		AddServer("205.209.136.190",4661);
		AddServer("205.209.176.10",4593);
		AddServer("205.209.178.170",12933);
		AddServer("207.150.166.150",3306);
		AddServer("210.51.191.152",4661);
		AddServer("211.50.137.96",4661);
		AddServer("211.144.142.147",9876);
		AddServer("211.167.240.54",3888);
		AddServer("211.214.161.107",4661);
		AddServer("211.233.41.235",4661);//  Korea Only ed2k://",211.233.41.235",4661");
		AddServer("213.8.141.91",4661);
		AddServer("213.158.119.104",4661);//	Project Grande	Austria	29740	1993597	4days, 0:45 h	59	62	Yes	Yes	16.49	51	55	No	Yes	Project Grande
		AddServer("213.202.245.74",4661);
		AddServer("213.202.245.77",4661);
		AddServer("217.31.16.70",8080);
		AddServer("217.31.16.71",8080);
		AddServer("217.88.240.169",6969);
		AddServer("217.91.45.71",4242);
		AddServer("217.91.102.96",4661);
		AddServer("217.106.18.150",4661);
		AddServer("217.106.18.217",4661);
		AddServer("218.78.211.62",4661);
		AddServer("218.87.20.106",1888);


//70.84.28.212:4242

//Address:Port	Name	CC	Users	Files	Uptime	Min	Max	High	Low	Version	Refs	Known	Bot	MLD	Description

		//NORMAL SERVERS
	}
	

	/*
	for(UINT i=0;i<mv_servers.Size() && !this->b_killThread;i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		
		sh->StartThread();
		for(int j=0;j<5 && !this->b_killThread;j++){  //sleep for .5 seconds before initing another server
			Sleep(10);
		}
	}*/
	
}

bool SpoofingSystem::GetSwarmFile(Vector& v, byte* hash)
{

	if(mb_spoofing_only)
		return false;

//	CSingleLock lock(&m_decoys_lock,TRUE);

	DonkeyFile df("x",hash,1,NULL);  //a reference we can use to find what we are looking for
	int index=-1;
	index=mv_swarms.BinaryFind(&df);

	if(index==-1)
		return false;
	else{
		v.Add(mv_swarms.Get(index));
		return true;
	}

	/*
	//see if its a swarm
	for(int i=0;i<(int)mv_swarms.Size();i++){
		DonkeyFile *df=(DonkeyFile*)mv_swarms.Get(i);
		if(memcmp(df->m_hash,hash,16)==0){
			v.Add(df);
			return true;
		}
	}

	return false;
	*/
	
}


bool SpoofingSystem::GetFalseDecoyFile(Vector& v, byte* hash)
{
	if(mb_spoofing_only)
		return false;

	CSingleLock lock(&m_false_decoys_lock,TRUE);
	//see if its a decoy

	DonkeyFile df("x",hash,1,NULL);  //a reference we can use to find what we are looking for
	int index=mv_false_decoys.BinaryFind(&df);
	if(index==-1)
		return false;
	else{
		v.Add(mv_false_decoys.Get(index));
		return true;
	}


	/*
	for(int i=0;i<(int)mv_false_decoys.Size();i++){
		DonkeyFile *df=(DonkeyFile*)mv_false_decoys.Get(i);
		if(memcmp(df->m_hash,hash,16)==0){
			v.Add(df);
			return true;
		}
	}

	return false;*/
}


bool SpoofingSystem::GetDecoyFile(Vector& v, byte* hash)
{
	if(mb_spoofing_only)
		return false;

//	CSingleLock lock(&m_decoys_lock,TRUE);

	DonkeyFile df("x",hash,1,NULL);  //a reference we can use to find what we are looking for
	int index=mv_decoys.BinaryFind(&df);
	if(index==-1)
		return false;
	else{
		v.Add(mv_decoys.Get(index));
		return true;
	}

/*
	//see if its a decoy
	for(int i=0;i<(int)mv_decoys.Size();i++){
		DonkeyFile *df=(DonkeyFile*)mv_decoys.Get(i);
		if(memcmp(df->m_hash,hash,16)==0){
			v.Add(df);
			return true;
		}
	}
	return false;
	*/
}

void SpoofingSystem::AddSwarm(byte* hash, const char* file_name, byte* hash_set, int file_size)
{
	Vector v_tmp;
	if(!GetSwarmFile(v_tmp,hash)){
		mv_swarms.Add(new DonkeyFile(file_name,hash,file_size,hash_set));
		mv_swarms.Sort(1);
		TRACE("SpoofingSystem::AddSwarm() Now has %d/%d swarms loaded.\n",mv_swarms.Size(),m_swarm_data.mv_swarms.Size());
	}
}

//FileRequestLogger* SpoofingSystem::GetRequestLogger(void)
//{
//	return &m_request_logger;
//}

void SpoofingSystem::GetSomeSpoofs(int chance, int limit,Vector& v)
{
	/*
	CString tmp;
	srand(timeGetTime());

	CSingleLock lock(&m_spoof_data.m_data_lock,TRUE);
	for(int i=0;i<(int)m_spoof_data.mv_spoofs.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(m_spoof_data.mv_spoofs.Get(i));  
		}
	}

	while((int)v.Size()>max(1,limit)){
		v.Remove(rand()%v.Size());  //randomly remove items from this vector until its less than the limit
	}*/
}

void SpoofingSystem::GetSomeSwarms(int chance, int limit,Vector& v)
{
	CSingleLock lock(&m_swarm_data.m_data_lock,TRUE);
	for(int i=0;i<(int)m_swarm_data.mv_swarms.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(m_swarm_data.mv_swarms.Get(i));  
		}
	}

	while((int)v.Size()>max(1,limit)){
		v.Remove(rand()%v.Size());  //randomly remove items from this vector until its less than the limit
	}
}

void SpoofingSystem::GetSomeAntiLeakFiles(int chance, int limit,Vector& v)
{
	/*
	CSingleLock lock(&m_anti_leak_data.m_data_lock,TRUE);
	for(int i=0;i<(int)m_anti_leak_data.m_popular_anti_leak_files.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(m_anti_leak_data.mv_swarms.Get(i));  
		}
	}

	while((int)v.Size()>max(1,limit)){
		v.Remove(rand()%v.Size());  //randomly remove items from this vector until its less than the limit
	}*/
}

/*
void SpoofingSystem::OnSend(TKTCPConnection &con)
{
	//figure out which client sent us this
	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		if(cc->IsConnection(con)){
			cc->OnSend();
			return;
		}
	}

}*/

bool SpoofingSystem::GetUserHash(byte* hash, const char* file_name)
{
	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_READ,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				OPEN_EXISTING,             // existing file only 
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 


	
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		TRACE("SpoofingSystem GetUserHash: Could not open user hash with filename %s because of %d.\n",file_name,GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	
	DWORD high_size=0;
	DWORD file_size=GetFileSize(file_handle,&high_size);

	if(file_size!=16){
		TRACE("SpoofingSystem GetUserHash: Could not open user hash with filename %s as the file size wasn't equal to 16.\n",file_name);
		CloseHandle(file_handle);
		DeleteFile(file_name);
		return false;
	}

	DWORD tmp;
	
	ReadFile(file_handle,hash,16,&tmp,NULL);
	CloseHandle(file_handle);

	if(tmp!=file_size){
		TRACE("SpoofingSystem GetUserHash: Could not open user hash with filename %s, it couldn't read the 16 bytes because of %d.\n",file_name,GetLastError());
		DeleteFile(file_name);
 		return false;
	}

	return true;
}

void SpoofingSystem::WriteUserHash(byte* hash, const char* file_name)
{
	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_WRITE,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				CREATE_ALWAYS,             // always create a new file
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		TRACE("SpoofingSystem WriteUserHash: Could not write user hash with filename %s.\n",file_name);
		CloseHandle(file_handle);
		return;
	}

	DWORD tmp;
	WriteFile(file_handle,hash,16,&tmp,NULL);
	CloseHandle(file_handle);
}

string SpoofingSystem::GetHashThatNeedsSources(void)
{
//	CSingleLock lock(&m_hashes_that_need_sources_lock,TRUE);
	//srand(timeGetTime());
	if(mv_hashes_that_need_sources.size()<1)
		return "";
	int rand_index=rand()%(max(1,(int)mv_hashes_that_need_sources.size()));

	return mv_hashes_that_need_sources[rand_index];
}

void SpoofingSystem::AddHashThatNeedsSources(const char* hash)
{
//	CSingleLock lock(&m_hashes_that_need_sources_lock,TRUE);
	if(mv_hashes_that_need_sources.size()>300)
		mv_hashes_that_need_sources.clear();
	mv_hashes_that_need_sources.push_back(string(hash));
}

void SpoofingSystem::AddSourceForHash(const char* ip, unsigned short int port, const char* hash)
{
	if(ip==NULL || port==0 ||hash==NULL || strlen(ip)==0 || strlen(hash)==0)
		return;

	CString cip=ip;
	if(cip.Find("38.118.15")==0){
		return;
	}
	else if(cip.Find("38.118.16")==0){
		return;
	}
	else if(cip.Find("38.118.144")==0){
		return;
	}
	else if(cip.Find("38.118.139")==0){
		return;
	}
	else if(cip.Find("38.119.64")==0){
		return;
	}
	else if(cip.Find("38.119.7")==0){
		return;
	}
	else if(cip.Find("66.54.7")==0){
		return;
	}
	else if(cip.Find("65.19.175")==0){
		return;
	}
	else if(cip.Find("66.160.14")==0){
		return;
	}
	else if(cip.Find("66.117.3.")==0){
		return;
	}

	TRACE("SpoofingSystem::AddSourceForHash(void).\n");

	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(ADDSOURCE);  //swarms
	response_packet.WriteByte(key);  //swarms

	response_packet.WriteWord(port);
	response_packet.WriteWord((WORD)strlen(ip));
	response_packet.WriteString(ip);
	response_packet.WriteWord((WORD)strlen(hash));
	response_packet.WriteString(hash);

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());

	//CString query;

	//insert new
	//query.Format("insert ignore into file_sources values ('%s','%s',%u,now(),1)",hash,ip,port);	
	//mv_needed_queries.Add(new QueryObject(query));
}

void SpoofingSystem::RemoveSourceForHash(const char * hash, const char * ip, unsigned short port)
{
	TRACE("SpoofingSystem::RemoveSourceForHash(void).\n");
	//update it if it exists
	/*
	if((rand()%100)>3)  //only a small chance we'll try to remove this hash.  We need to cut down on the database workload
		return;
	CString query;
	query.Format("delete from file_sources where hash='%s' and ip='%s' and port=%u and auto_clean=1",hash,ip,port);
	mv_needed_queries.Add(new QueryObject(query));

	query.Format("delete from interdiction_targets where hash='%s' and ip='%s' and port=%u and auto_clean=1",hash,ip,port);
	mv_needed_queries.Add(new QueryObject(query));

	//insert new
	query.Format("delete from ban_table where ip_start='%s' and expires=1",ip,port);	
	mv_needed_queries.Add(new QueryObject(query));
	*/

	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;

	byte key=0xe4;  //a special byte that encodes a message as having come from this plug-in

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(REMOVESOURCE);  //swarms
	response_packet.WriteByte(key);  //swarms

	response_packet.WriteWord(port);
	response_packet.WriteWord((WORD)strlen(ip));
	response_packet.WriteString(ip);
	response_packet.WriteWord((WORD)strlen(hash));
	response_packet.WriteString(hash);

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
}

void SpoofingSystem::AddUserName(const char* name)
{
	/*
	static int user_name_index=0;
	//every now and then steal a user name.  Not all the time as that would be a lot of database load
	if(((user_name_index++)%500)==0){
		CString tmp=name;
		CString query;
		query.Format("insert ignore into user_names values ('%s',now())",name);	
		mv_needed_queries.Add(new QueryObject(query));
	}*/
}

void SpoofingSystem::LogInterdiction(const char* project, const char* hash, const char* ip, unsigned short port, UINT total_downloaded)
{
	/*
	//insert new
	CString query;
	query.Format("insert into interdiction_log values ('%s','%s','%s',%u,now(),%u)",project,hash,ip,port,total_downloaded);
	mv_needed_queries.Add(new QueryObject(query));
	*/
}

void SpoofingSystem::ComDataReceived(const char* source_ip, byte* data, UINT data_length)
{
	/*
	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;
	*/

	_ASSERTE( _CrtCheckMemory( ) );
	LogToFile("SpoofingSystem::ComDataReceived() BEGIN.\r\n");

	if(data_length<4){
		LogToFile("SpoofingSystem::ComDataReceived() END.\r\n");
		return;
	}
	byte version=data[0];

	if(version!=1){  //we only know how to interpret version 1
		LogToFile("SpoofingSystem::ComDataReceived() END.\r\n");
		return;
	}

	byte b_for_server=data[1];

	if(b_for_server){  //we only care about this message if its for a client
		LogToFile("SpoofingSystem::ComDataReceived() END.\r\n");
		return;
	}

	
	byte b_type=data[2];
	byte key=data[3];
	if(key!=ED2KCONTROLKEY){
		LogToFile("SpoofingSystem::ComDataReceived() END.\r\n");
		return;
	}

	_ASSERTE( _CrtCheckMemory( ) );
	byte *p_data=data+4;

	//TRACE("SpoofingSystem::ComDataReceived() Received %d message type.\n",b_type);

	if(b_type==ED2KGETSWARMS){  //they want a list of files to swarm
		CSingleLock lock(&m_swarm_data.m_data_lock,TRUE);

		UINT num_swarms=*(UINT*)p_data;
		p_data+=sizeof(UINT);
		m_swarm_data.Reset();
		
		//m_swarm_data.AddSwarm("Kanye West","21B9864EE73B5F997769B849B2D27950","","Kanye.West.-.Through.the.Wire.mp3",3638045);

		for(int i=0;i<(int)num_swarms;i++){
			WORD slen=*(WORD*)p_data;
			p_data+=2;
			string project;
			for(int j=0;j<slen;j++) project+=(char)*(p_data++);

			slen=*(WORD*)p_data;
			p_data+=2;
			string file_name;
			for(int j=0;j<slen;j++) file_name+=(char)*(p_data++);

			slen=*(WORD*)p_data;
			p_data+=2;
			string hash;
			for(int j=0;j<slen;j++) hash+=(char)*(p_data++);

			slen=*(WORD*)p_data;
			p_data+=2;
			string hash_set;
			for(int j=0;j<slen;j++) hash_set+=(char)*(p_data++);

			UINT file_size=*(UINT*)p_data;
			p_data+=4;
			//TRACE("Adding swarm with name %s, size %u, hash %s, project %s.\n",file_name.c_str(),file_size,hash.c_str(),project.c_str());
	
			m_swarm_data.AddSwarm(project.c_str(),hash.c_str(),hash_set.c_str(),file_name.c_str(),file_size);
		}

		
		TRACE("SpoofingSystem::ComDataReceived() Received %d swarms.\n",num_swarms);
		/*
		//write in data for each of the swarms
		response_packet.WriteDWord(0);  //we will set this later to the number of swarms we added
		int swarms_added=0;
		for(int count=0;count<1500 && mv_swarms.Size()>0;count++,swarm_offset++,swarms_added++){
			SwarmObject* so=(SwarmObject*)mv_swarms.Get(swarm_offset%mv_swarms.Size());
			response_packet.WriteWord((WORD)so->m_project.size());
			response_packet.WriteString(so->m_project.c_str());
			response_packet.WriteWord((WORD)so->m_file_name.size());
			response_packet.WriteString(so->m_file_name.c_str());
			response_packet.WriteWord((WORD)so->m_hash.size());
			response_packet.WriteString(so->m_hash.c_str());
			response_packet.WriteWord((WORD)so->m_hash_set.size());
			response_packet.WriteString(so->m_hash_set.c_str());
			response_packet.WriteDWord(so->m_file_size);
		}
		response_packet.WriteDWord(3,swarms_added);

		m_com.SendReliableData((char*)source_ip,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
		return;
		*/
	}

	if(b_type==ED2KGETINTERDICTIONTARGETS){  //they want a list of interdiction_targets
		UINT num_targets=*(UINT*)p_data;
		p_data+=sizeof(UINT);
		
		for(int i=0;i<(int)num_targets;i++){

			WORD slen=*(WORD*)p_data;
			p_data+=2;
			string project;
			for(int j=0;j<slen;j++) project+=(char)*(p_data++);

			slen=*(WORD*)p_data;
			p_data+=2;
			string hash;
			for(int j=0;j<slen;j++) hash+=(char)*(p_data++);

			slen=*(WORD*)p_data;
			p_data+=2;
			string ip;
			for(int j=0;j<slen;j++) ip+=(char)*(p_data++);

			WORD port=*(WORD*)p_data;
			p_data+=sizeof(WORD);

			string user_name=m_my_user_names[0][0].c_str();

			//TRACE("Adding interdiction target with ip %s, port %u, hash %s, project %s.\n",ip.c_str(),port,hash.c_str(),project.c_str());
			InterdictionConnection *nic=new InterdictionConnection(user_name.c_str(),m_my_hashes[0][0],BASEPORT+1,hash.c_str(),ip.c_str(),port,project.c_str());
			mv_interdiction_queue.Add(nic);
		}

		TRACE("SpoofingSystem::ComDataReceived() Received %d interdiction targets.\n",num_targets);
	}

	if(b_type==ED2KGETSOURCEHASHES){  //they want a list of hashes that they need to ask for sources for
		UINT num_hashes=*(UINT*)p_data;
		p_data+=sizeof(UINT);
		
		for(int i=0;i<(int)num_hashes;i++){

			WORD slen=*(WORD*)p_data;
			p_data+=2;
			string hash;
			mv_hashes_that_need_sources.clear();
			for(int j=0;j<slen;j++) hash+=(char)*(p_data++);
			
			//TRACE("Adding source hash hash %s.\n",hash.c_str());
			mv_hashes_that_need_sources.push_back(hash);
		}

		TRACE("SpoofingSystem::ComDataReceived() Received source hash list, %d hashes added.\n",mv_hashes_that_need_sources.size());
	}

	if(b_type==ED2KSEARCHTASK){
		//read the search task from the data stream.
		WORD slen;

		slen=*(WORD*)p_data;
		p_data+=2;
		string project;
		for(int j=0;j<slen;j++) project+=(char)*(p_data++);
		
		slen=*(WORD*)p_data;
		p_data+=2;
		string search_str;
		for(int j=0;j<slen;j++) search_str+=(char)*(p_data++);

		UINT minimum_size=*(UINT*)p_data;
		p_data+=4;

		m_current_search_task.m_project=project;
		m_current_search_task.m_search_string=search_str;
		m_current_search_task.m_minimum_size=minimum_size;
		
		TRACE("SpoofingSystem::ComDataReceived() Received search task from server project='%s', search string='%s', minimum_size='%d'\n",project.c_str(),search_str.c_str(),minimum_size);
		//how it was written
		//response_packet.WriteWord((WORD)st->m_project.size());  //project
		//response_packet.WriteString(st->m_project.c_str());
		//response_packet.WriteWord((WORD)st->m_search_str.size());  //search string
		//response_packet.WriteString(st->m_search_str.c_str());
		//response_packet.WriteDWord(st->m_minimum_size);  //minimum size
	}
	

	LogToFile("SpoofingSystem::ComDataReceived() END.\r\n");
	_ASSERTE( _CrtCheckMemory( ) );
}

void SpoofingSystem::GetInterdictionTargets(void)
{
	TRACE("SpoofingSystem::GetInterdictionTargets(void).\n");
	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(GETINTERDICTIONTARGETS);  //request some interdiction targets
	response_packet.WriteByte(key);  //swarms

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
}

void SpoofingSystem::GetHashesThatNeedSources(void)
{
	TRACE("SpoofingSystem::GetHashesThatNeedSources(void).\n");
	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(GETSOURCEHASHES);  //request some interdiction targets
	response_packet.WriteByte(key);  //swarms

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
}

void SpoofingSystem::LogServerConnection(const char* server_ip)
{
	if(server_ip==NULL || strlen(server_ip)==0 )
		return;

	CString cip=server_ip;
	if(cip.Find("38.118.15")==0){
		return;
	}
	else if(cip.Find("38.118.16")==0){
		return;
	}
	else if(cip.Find("38.118.144")==0){
		return;
	}
	else if(cip.Find("38.118.139")==0){
		return;
	}
	else if(cip.Find("38.119.64")==0){
		return;
	}
	else if(cip.Find("38.119.7")==0){
		return;
	}
	else if(cip.Find("66.54.7")==0){
		return;
	}
	else if(cip.Find("65.19.175")==0){
		return;
	}
	else if(cip.Find("66.160.14")==0){
		return;
	}
	else if(cip.Find("66.117.3.")==0){
		return;
	}

	TRACE("SpoofingSystem::LogServerConnection(void). STARTED\n");

	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;
	static const int RACKSERVERLOG			= 8;
	static const int SERVERENTRY			= 9;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(RACKSERVERLOG);  //swarms
	response_packet.WriteByte(key);  //swarms

	response_packet.WriteWord((WORD)strlen(server_ip));
	response_packet.WriteString(server_ip);

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	TRACE("SpoofingSystem::LogServerConnection(void). FINISHED\n");
}	

void SpoofingSystem::LogNewServer(const char* ip, unsigned short port)
{
	if(ip==NULL || port==0  || strlen(ip)==0 )
		return;

	CString cip=ip;
	if(cip.Find("38.118.15")==0){
		return;
	}
	else if(cip.Find("38.118.16")==0){
		return;
	}
	else if(cip.Find("38.118.144")==0){
		return;
	}
	else if(cip.Find("38.118.139")==0){
		return;
	}
	else if(cip.Find("38.119.64")==0){
		return;
	}
	else if(cip.Find("38.119.7")==0){
		return;
	}
	else if(cip.Find("66.54.7")==0){
		return;
	}
	else if(cip.Find("65.19.175")==0){
		return;
	}
	else if(cip.Find("66.160.14")==0){
		return;
	}
	else if(cip.Find("66.117.3.")==0){
		return;
	}

	TRACE("SpoofingSystem::LogNewServer(void).\n");

	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;
	static const int RACKSERVERLOG			= 8;
	static const int SERVERENTRY			= 9;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(SERVERENTRY);  //swarms
	response_packet.WriteByte(key);  //swarms

	response_packet.WriteWord((WORD)port);
	response_packet.WriteWord((WORD)strlen(ip));
	response_packet.WriteString(ip);

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());

}

UINT SpoofingSystem::GetClientConnectionCount(void)
{
	return ml_client_connections.Size();
}

UINT SpoofingSystem::GetInactiveConnectionCount(void)
{
	return mv_inactive_interdiction_connections.Size();
}

UINT SpoofingSystem::GetActiveConnectionCount(void)
{
	return mv_interdiction_connections.Size();
}

void SpoofingSystem::OnAccept(CAsyncSocket *cserver_socket,CAsyncSocket* new_connection)
{
	LogToFile("SpoofingSystem::OnAccept() BEGIN [0]");
	//figure out which server sent us this
	CString log_msg;
	log_msg.Format("SpoofingSystem::OnAccept() BEGIN %s.",((PeerSocket*)new_connection)->GetIP());
	LogToFile(log_msg);
	MetaMachineSpooferDll::GetDlg()->GotConnection();

	while(ml_client_connections.Size()>=CONNECTIONLIMIT){  //we are at capacity
		log_msg.Format("SpoofingSystem::OnAccept() Connection limit reached, removing oldest connection to make room for %s.",((PeerSocket*)new_connection)->GetIP());
		LogToFile(log_msg);
		ClientConnection *cc=(ClientConnection*)ml_client_connections.GetHead();
		cc->Shutdown();
		ml_client_connections.RemoveHead();  //kill the oldest connection
	}

	PeerSocket *new_peer=(PeerSocket*)new_connection;

	if(mb_halt_system){
		log_msg.Format("SpoofingSystem::OnAccept() END (System Halted) %s.",((PeerSocket*)new_connection)->GetIP());
		LogToFile(log_msg);
		delete new_peer;
		return;
	}

	ServerSocket *server_socket=(ServerSocket*)cserver_socket;
	int bound_ip_index=0;
	for(int i=0;i<(int)mv_system_ips.size();i++){
		if(stricmp(new_peer->GetBindIP(),mv_system_ips[i].c_str())==0){
			bound_ip_index=i;
			break;
		}
	}

	//if(stricmp("195.245.244.241",new_peer->GetIP())!=0 && stricmp("66.54.78.16",new_peer->GetIP())!=0){
	//	delete new_peer;
	//	return;
	//}

	
	/*
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(sh->IsServer(new_peer->GetIP(),new_peer->GetPort())){
			TRACE("SpoofingSystem::OnAccept() from %s for server %s.\n",new_peer->GetIP(),sh->GetIP());
			sh->NewConnection(new_peer);
			return;
		}
	}*/

//	SpooferTCPReference reference;
	//figure out if a client is connection back to us to upload to us
	if(server_socket->GetPort()==BASEPORT+1){
		//CSingleLock lock(&m_interdiction_connection_lock,TRUE);
		for(UINT i=0;i<mv_inactive_interdiction_connections.Size();i++){
			InterdictionConnection *cc=(InterdictionConnection*)mv_inactive_interdiction_connections.Get(i);
			if(cc->IsConnection(new_peer->GetIP()) && cc->IsAwaitingCallBack()){
				cc->ReestablishedConnection(new_peer);
				mv_interdiction_connections.Add(cc);  //add it back into the active list
				mv_inactive_interdiction_connections.Remove(i);  //remove it from the inactive list
				log_msg.Format("SpoofingSystem::OnAccept() END (Interdiction) %s.",((PeerSocket*)new_connection)->GetIP());
				LogToFile(log_msg);
				return;
			}
		}
	}

	TRACE("New client connection from %s with local address of %s.\n",new_peer->GetIP(),new_peer->GetBindIP());
	
	for(int i=0;i<MULTI_CONNECTION_COUNT;i++){
		if(m_server_sockets[i].GetPort()==server_socket->GetPort()){
			ClientConnection *ncl=new ClientConnection(new_peer,BASEPORT+i,m_my_hashes[bound_ip_index][i],m_my_user_names[bound_ip_index][i]);
			//ncl->StartThread();
			ml_client_connections.Add(ncl);  //we have a new client connection
			log_msg.Format("SpoofingSystem::OnAccept() END (Normal) %s.",((PeerSocket*)new_connection)->GetIP());
			LogToFile(log_msg);
			return;
		}
	}

	log_msg.Format("SpoofingSystem::OnAccept() END (Unwanted?) %s.",((PeerSocket*)new_connection)->GetIP());
	LogToFile(log_msg);
	delete new_peer;  //nobody wants this connection
}

void SpoofingSystem::SwitchInterdictionConnectionToInactive(Object* con)
{
	mv_inactive_interdiction_connections.Add(con);
	int count_removed=mv_interdiction_connections.RemoveByReference(con);
	//if(count_removed>0){
		//TRACE("Toggled another active interdiction connection to now awaiting callback.\n",count_removed);
	//}
}

//static function
void SpoofingSystem::LogToFile(const char* str)
{
	//maintain synchronous access to this function
	CSingleLock lock(&sm_log_lock,TRUE);

	if(sm_log_bytes_written>6000000){  //every X amount of data, reset the log to prevent uncontrollable growth.
		CloseHandle(sm_log_file_handle);
		sm_log_file_handle=NULL;
		sm_log_bytes_written=0;
	}
	if(sm_log_file_handle==NULL){
		CreateDirectory("c:\\MetaMachine",NULL);
		CreateDirectory("c:\\MetaMachine\\Swarmer_Decoyer",NULL);
		sm_log_file_handle = CreateFile("c:\\MetaMachine\\Swarmer_Decoyer\\debug_log.txt",			// open file at local_path 
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
}

void SpoofingSystem::CreateDecoys(void)
{
	srand(timeGetTime());

	mv_decoys.Clear();
	LoadDecoys("c:\\syncher\\rcv\\Edonkey.decoys",mv_decoys);


//	CreateFalseDecoy("Test","ByteSeven.mp3",8765021,1);

	//mv_false_decoys.Clear();
	//LoadDecoys("c:\\syncher\\rcv\\Edonkey False_Decoys",mv_false_decoys);

	//go through and assign projects based on file paths we previously stored in a vector
	/*
	for(int i=0;i<(int)mv_false_decoys.Size();i++){
		DonkeyFile* df=(DonkeyFile*)mv_false_decoys.Get(i);
		CString tmp_name;
		tmp_name.Format("%s\\%s",df->m_file_path.c_str(),df->m_file_name.c_str());
		for(int j=0;j<(int)v_files.size();j++){
			CString path;
			path.Format("c:\\syncher\\rcv\\Edonkey False_Decoys\\%s\\%s",v_projects[j].c_str(),v_files[j].c_str());
			if(stricmp(path,tmp_name)==0){  //we have found it
				df->SetProject(v_projects[j].c_str());
				break; 
			}
		}
	}*/

	//**** END FALSE DECOY LOAD ****
//	mv_false_decoys.Sort(1);
	mv_decoys.Sort(1);
}

void SpoofingSystem::LogDecoyRequest(const char* file_name,const char* ip)
{
	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;
	static const int RACKSERVERLOG			= 8;
	static const int SERVERENTRY			= 9;
	static const int DECOYLOG				= 10;
	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(DECOYLOG);  //DECOYLOG
	response_packet.WriteByte(key);  //DECOYLOG
	response_packet.WriteWord((WORD)strlen(file_name));  
	response_packet.WriteString(file_name);  
	response_packet.WriteWord((WORD)strlen(ip));  
	response_packet.WriteString(ip);  

	m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
}

UINT SpoofingSystem::GetEmuleVersion(void)
{
	return m_emule_version;
}

void SpoofingSystem::InitEvilSeedData(void)
{
	HRSRC hrsrc=FindResource(GetModuleHandle(DLLNAME),MAKEINTRESOURCE(IDR_EVILSEEDDATA),"RAWDATA");
	int error=GetLastError();
	DWORD the_size=SizeofResource(GetModuleHandle(DLLNAME),hrsrc);

	HGLOBAL global_handle=LoadResource(GetModuleHandle(DLLNAME),hrsrc);
	byte *the_data=(byte*)LockResource(global_handle);

	if(hrsrc!=NULL && the_size>0 && global_handle!=NULL)
		m_evil_seed_data.WriteBytes(the_data,the_size);
}

Buffer2000* SpoofingSystem::GetEvilSeedData(void)
{
	return &m_evil_seed_data;
}

SearchTask SpoofingSystem::GetSearchTask(void)
{
	return m_current_search_task;
}

void SpoofingSystem::ReportSearchResults(Vector &v_results)
{
	if(v_results.Size()<1)
		return;
	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(ED2KSEARCHRESULTS);  //request a search task
	response_packet.WriteByte(ED2KCONTROLKEY);  //control code

	response_packet.WriteDWord(v_results.Size());
	for(int i=0;i<(int)v_results.Size();i++){
		SearchResult *result=(SearchResult*)v_results.Get(i);
		if(result->m_availability<2) //ignore the availabilities of 1 since there are 8 billion of them and we don't even care about them
			continue;
		response_packet.WriteWord((WORD)result->m_project.size());  
		response_packet.WriteString(result->m_project.c_str());  

		response_packet.WriteWord((WORD)result->m_server_ip.size());  
		response_packet.WriteString(result->m_server_ip.c_str());  

		response_packet.WriteWord((WORD)result->m_hash.size());  
		response_packet.WriteString(result->m_hash.c_str());  

		response_packet.WriteWord((WORD)result->m_file_name.size());  
		response_packet.WriteString(result->m_file_name.c_str());  

		response_packet.WriteDWord(result->m_size);  

		response_packet.WriteDWord(result->m_availability);  
	}

//	TRACE("SpoofingSystem::ReportSearchResult() project='%s', server_ip='%s', file_name='%s', size='%d', availability='%d', hash='%s'\n",result.m_project.c_str(),result.m_server_ip.c_str(),result.m_file_name.c_str(),result.m_size,result.m_availability,result.m_hash.c_str());

	MetaSpooferReference ref;
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
}

void SpoofingSystem::RequestNewSearchTaskFromServer(void)
{
	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(ED2KSEARCHTASK);  //request a search task
	response_packet.WriteByte(ED2KCONTROLKEY);  //control code

	MetaSpooferReference ref;
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	TRACE("SpoofingSystem::RequestNewSearchTaskFromServer()\n");
}

/*
void SpoofingSystem::PostFakeSearchResults(void)
{
	Vector v_tmp;
	SearchResult *sr=new SearchResult();
	sr->m_availability=1;
	sr->m_file_name="test_file_name.avi";
	sr->m_project="test_project";
	sr->m_server_ip="x.x.x.x";
	sr->m_size=100000000;
	sr->m_hash="aaaabbbbccccddddaaaabbbbccccdddd";

	v_tmp.Add(sr);
	sr=new SearchResult();
	sr->m_availability=1;
	sr->m_file_name="test_file_name2.avi";
	sr->m_project="test_project";
	sr->m_server_ip="x.x.x.y";
	sr->m_size=100000000;
	sr->m_hash="aaaabbbbccccbbbbaaaabbbbccccdddd";
	v_tmp.Add(sr);

	ReportSearchResults(v_tmp);
}
*/

void SpoofingSystem::AddFalseDecoy(DonkeyFile* df)
{
	Vector v_tmp;
	if(!GetFalseDecoyFile(v_tmp,df->m_hash)){
		mv_false_decoys.Add(df);
		mv_false_decoys.Sort(1);
		TRACE("SpoofingSystem::AddFalseDecoy() Now has %d/%d false decoys loaded.\n",mv_false_decoys.Size(),m_false_decoy_data.mv_false_decoys.Size());
	}
}

void SpoofingSystem::AddAntiLeakFile(const char* file_name,byte* hash,UINT size)
{
	/*
	Vector v_tmp;
	if(!GetAntiLeakFile(v_tmp,hash)){
		mv_anti_leak_files.Add(new DonkeyFile(file_name,hash,file_size,NULL));
		mv_anti_leak_files.Sort(1);
		TRACE("SpoofingSystem::AddAntiLeakFile() Now has %d/%d anti leak files loaded.\n",mv_anti_leak_files.Size(),m_anti_leak_data.mv_swarms.Size());
	}*/
}



void SpoofingSystem::ClearFalseDecoys(void)
{
	CSingleLock lock(&m_false_decoys_lock,TRUE);
	mv_false_decoys.Clear();
}

Buffer2000* SpoofingSystem::GetGarbageData(void)
{
	return &m_garbage_data;
}

//Call to determine if the current day is a saturday or a sunday
bool SpoofingSystem::IsWeekend(void)
{
	static Timer last_check;
	static bool b_has_checked=false;
	CSingleLock lock(&m_weekend_lock,TRUE);

	if(!last_check.HasTimedOut(60*60*3) && b_has_checked)
		return mb_weekend;  //not time to check yet, return cached value

	b_has_checked=true;
	last_check.Refresh();

	TinySQL sql;
	if(!sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306))
		return mb_weekend;  //error, return cached value

	//get the time from the server, because thats the only time we can assume is accurate
	if(!sql.Query("select DAYNAME(now())",true))
		return mb_weekend;  //error, return cached value
	
	if(sql.m_num_rows<1)
		return mb_weekend;  //error, return cached value

	string day_name=sql.mpp_results[0][0].c_str();
	if(stricmp(day_name.c_str(),"Saturday")==0 || stricmp(day_name.c_str(),"Sunday")==0){
		mb_weekend=true;
		return true;
	}
	else{
		mb_weekend=false;
		return false;
	}
}

bool SpoofingSystem::IsSpoofingOnly(void)
{
	return mb_spoofing_only;
}
