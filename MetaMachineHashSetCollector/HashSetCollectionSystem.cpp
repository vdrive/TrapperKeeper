#include "StdAfx.h"
#include "HashSetCollectionsystem.h"
#include "SpoofHost.h"
#include "MetaSpooferTCP.h"
#include "MetaMachineSpooferDll.h"
#include "HashSetClientConnection.h"
#include "..\tkcom\Timer.h"

HashSetCollectionSystem* HashSetCollectionSystem::sm_system=NULL;
UINT HashSetCollectionSystem::sm_system_count=0;

HashSetCollectionSystem::HashSetCollectionSystem(void)
{
	mb_needs_decoy_load=true;
}

HashSetCollectionSystem::~HashSetCollectionSystem(void)
{
}

void HashSetCollectionSystem::NewConnection(UINT handle, const char* source_ip)
{
	//figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		ServerObject *sh=(ServerObject*)mv_servers.Get(i);
		if(sh->IsServer(source_ip)){
			TRACE("HashSetCollectionSystem::OnAcceptedConnection() from %s.\n",source_ip);
			sh->NewConnection(handle);
			return;
		}
	}

	//We don't know who this is.  Let them know what we think of this...
	//TRACE("MetaMachine :  MetaSystem::NewConnection()  A rogue connection from %s has been received.  It is probably a request for a file, we will let it dangle for several minutes.\n",source_ip);
//	mv_rogue.push_back(handle);

	//we don't accept any rogue connections...
	TRACE("New client connection from %s.\n",source_ip);
	//mv_client_connections.Add(new ClientConnection(handle,source_ip));  //we have a new client connection
	//MetaMachineSpooferDll::GetDlg()->GotConnection();
}

void HashSetCollectionSystem::NewData(UINT handle, const char* source_ip, Buffer2000& data)
{
	//figure out which client sent us this
	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		if(cc->IsConnection(handle)){
			cc->NewData(data);
			return;
		}
	}


	//it wasn't a client so it must be a server, figure out which server sent us this
	for(UINT i=0;i<mv_servers.Size();i++){
		ServerObject *sh=(ServerObject*)mv_servers.Get(i);
		if(sh->IsServer(source_ip)){
			sh->NewData(handle,data);
			return;
		}
	}

	//We don't know who this is.  Let them know what we think of this...
	//This shouldn't be possible... but just in case.
	//TRACE("MetaMachine :  MetaSystem::NewData()  rogue data from %s has been received!!!!  This isn't possible by the way.  Terminating this freak connection now.\n",source_ip);
	SpooferTCPReference tcp;
	tcp.TCP()->CloseConnection(handle);  
}

void HashSetCollectionSystem::LostConnection(UINT handle, const char* source_ip)
{
	//figure out which server lost its connection
	for(UINT i=0;i<mv_servers.Size();i++){
		ServerObject *sh=(ServerObject*)mv_servers.Get(i);
		if(sh->IsServer(source_ip)){
			sh->LostConnection(handle);
			return;
		}
	}

	//figure out which client this is for
	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		if(cc->IsConnection(handle)){
			cc->Shutdown();  //lets destroy this client connection
			mv_client_connections.Remove(i);
			return;
		}
	}

}

void HashSetCollectionSystem::Init(void)
{
	TRACE("MataMachineSpooofer:  HashSetCollectionSystem::Init().\n");
	
//	LoadDecoys("c:\\syncher\\rcv\\edonkey.decoys");
	this->StartThread();
}

void HashSetCollectionSystem::Update(void)
{

}

void HashSetCollectionSystem::GetConnectedServers(vector <string>& v_servers)
{
	for(UINT i=0;i<mv_servers.Size();i++){
		ServerObject *sh=(ServerObject*)mv_servers.Get(i);
		if(sh->IsConnected()){
			CString str;
			CString locked="";
			if(sh->IsThreadLockedUp()){
				locked="Server Thread Is Stuck!";
			}
			str.Format("%s : %d : (%d files uploaded) (mode=%d) %s",sh->GetIP(),sh->GetPort(),sh->GetUploadedFiles(),sh->GetState(),locked);	
			v_servers.push_back(string((LPCSTR)str));
		}
	}
}

void HashSetCollectionSystem::GetDisconnectedServers(vector <string>& v_servers)
{
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		if(!sh->IsConnected()){
			CString str;
			CString locked="";
			if(sh->IsThreadLockedUp()){
				locked="Server Thread Is Stuck!";
			}
			str.Format("%s : %d (mode=%d) %s",sh->GetIP(),sh->GetPort(),sh->GetState(),locked);	
			v_servers.push_back(string((LPCSTR)str));
		}
	}
}

void HashSetCollectionSystem::AddServer(const char* ip, UINT port)
{
	mv_servers.Add(new SpoofHost(ip,port));
}

void HashSetCollectionSystem::Shutdown(void)
{
	this->StopThread();
	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		sh->b_killThread=1;
	}

	for(UINT i=0;i<mv_client_connections.Size();i++){
		ClientConnection *cc=(ClientConnection*)mv_client_connections.Get(i);
		cc->Shutdown();
	}
}

void HashSetCollectionSystem::AddHash(byte hash, string name, int size)
{
}

void HashSetCollectionSystem::LoadDecoys(const char* in_directory,Vector &v_decoy_vector)
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
			Sleep(20);  //sleep for a little bit so we don't hog the cpu running md4 calculations on edonkey files
		}
	}
	FindClose(hFind);
}

/*
void HashSetCollectionSystem::LoadSwarms(const char* in_directory,Vector &v_swarm_vector)
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

UINT HashSetCollectionSystem::Run(){
	TRACE("HashSetCollectionSystem::Run()");
	Timer
	bool b_inited_servers=false;
	Sleep(5000);  //dont start crunching until a moment after trapper keeper finishes launching.
	while(!this->b_killThread){
		if(!b_inited_servers){  //a one time server init, done after the decoys have finished loading up.
			TRACE("HashSetCollectionSystem::Run() Initing Servers\n");
			b_inited_servers=true;
			InitServers();
		}

		//Request a set of hashes to search for from the database

		Sleep(100);
	}

	return 0;
}

void HashSetCollectionSystem::GetSomeDecoys(int chance, int limit,Vector& v)
{
	CSingleLock lock(&m_decoys_lock,TRUE);

	for(int i=0;i<(int)mv_decoys.Size();i++){
		if((rand()%100) < chance){	//a random chance this decoy will be included
			v.Add(mv_decoys.Get(i));  
		}
	}

	while((int)v.Size()>max(1,limit)){
		v.Remove(rand()%v.Size());  //randomly remove items from this vector until its less than the limit
	}
}

void HashSetCollectionSystem::InitServers(void)
{
	AddServer("38.119.96.17",3000);
	AddServer("38.119.96.15",4661);
	AddServer("24.198.82.55",4661);
	AddServer("61.71.81.100",4661);
	AddServer("62.193.128.32",4661);
	AddServer("62.241.35.15",4242);
	AddServer("63.105.207.29",4242);
	AddServer("63.246.128.90",4661);
	AddServer("63.246.131.50",6969);
	AddServer("64.241.35.16",4242);
	AddServer("64.246.38.98",6667);
	AddServer("64.246.48.20",4661);
	AddServer("64.246.54.12",6667);
	AddServer("66.58.18.82",4661);
	AddServer("66.111.36.40",4242);
	AddServer("66.111.54.190",4242);	
	AddServer("66.111.39.20",4242);
	AddServer("66.111.52.200",4224);
	AddServer("66.111.54.50",3000);
	AddServer("66.111.54.100",4242);
	AddServer("66.118.189.240",4242);
	AddServer("66.227.96.151",4661);
	AddServer("66.227.96.252",4661);	
	AddServer("69.57.138.74",6667);
	AddServer("140.123.108.139",7654);
	AddServer("193.111.198.139",4242);
	
	AddServer("193.111.198.138",4242);
	
	AddServer("193.111.198.137",4242);

	AddServer("193.111.199.179",4661);
	AddServer("193.111.199.183",4661);
	AddServer("193.111.199.187",4661);
	AddServer("193.111.199.211",4242);
	AddServer("194.97.40.162",4242);
	AddServer("195.245.244.243",4661);
	AddServer("195.112.128.222",6667);
	AddServer("195.69.196.58",4242);
	AddServer("207.44.200.40",4242);
	AddServer("207.44.222.47",4661);
	AddServer("207.44.170.12",4661);
	AddServer("207.44.206.27",4242);
	AddServer("209.0.207.220",4661);
	AddServer("210.91.73.198",5555);
	AddServer("210.17.189.98",8888);
	AddServer("211.20.189.140",4661);
	AddServer("211.233.41.235",4661);
	AddServer("211.43.207.149",4661);
	AddServer("211.233.39.38",4661);
	AddServer("211.227.178.134",4242);
	AddServer("216.40.240.53",4661);
	AddServer("216.17.103.14",69);
	AddServer("217.160.176.57",9999);
	AddServer("218.160.176.57",9999);
	AddServer("218.234.22.154",4242);

	for(UINT i=0;i<mv_servers.Size();i++){
		SpoofHost *sh=(SpoofHost*)mv_servers.Get(i);
		sh->StartThread();
		Sleep(15);
	}
}