#include "StdAfx.h"
#include "aresinterdictionmanager.h"

#define ARESINTERDICTIONSERVERPORT 4690
#define ARESINTERDICTIONCLIENTPORT 4691
#define ARESINTERDICTIONSERVERCODE 55
#define ARESINTERDICTIONCLIENTCODE 56
#define ARESINTERDICTIONGETTARGETS 100
#define ARESINTERDICTIONTARGETRESPONSE 101

AresInterdictionManager::AresInterdictionManager(void)
{
	//make sure windows sockets is running
    WSADATA info;
	if (WSAStartup(MAKEWORD(2,0), &info)){
		TRACE("AresInterdictionManager Couldn't start windows sockets.\n");
	}

	//setup the receive socket
	m_rcv_socket=socket(AF_INET,SOCK_DGRAM,0);

	if (m_rcv_socket == INVALID_SOCKET) {
		int error=GetLastError();
		TRACE("Couldn't create UDP AresInterdictionManager Receive Socket %d\n",error);
	}

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = PF_INET;             
	sa.sin_port = htons(ARESINTERDICTIONCLIENTPORT);

	if (bind(m_rcv_socket, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		closesocket(m_rcv_socket);
		int error=GetLastError();
		TRACE("Couldn't bind UDP AresInterdictionManager Receive Socket %d\n",error);
	}

	m_send_socket=socket(AF_INET,SOCK_DGRAM,0);

	if (m_send_socket == INVALID_SOCKET) {
		int error=GetLastError();
		TRACE("Couldn't create UDP AresInterdictionManager Send Socket %d\n",error);
	}

	/*
	AfxSocketInit();
	BOOL stat=m_rcv_socket.Create(ARESINTERDICTIONCLIENTPORT,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP AresInterdictionManager Receive Socket %d\n",error);
	}
	stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP AresInterdictionManager Send Socket %d\n",error);
	}*/

	m_banned_client_ratio=0;
	m_index=0;
	//mv_interdiction_targets.Add(new InterdictionTarget("81.3.87.174",36010,4001982,"76041847F0106037BB702A33AAA00D06160DC192",NULL));
}

AresInterdictionManager::~AresInterdictionManager(void)
{
}

bool AresInterdictionManager::GetInterdictionTarget(Vector &v_tmp)
{
	CSingleLock lock(&m_lock,TRUE);
	
	if(m_index>=(int)mv_interdiction_targets.Size())
		m_index=0;

	if(m_index>=(int)mv_interdiction_targets.Size())
		return false;

	InterdictionTarget* it=(InterdictionTarget*)mv_interdiction_targets.Get(m_index++);
	v_tmp.Add(it);
	return true;
}

UINT AresInterdictionManager::Run(void)
{
	srand(GetTickCount());
	while(!this->b_killThread){
		Sleep(50);
		byte buf[512];
		CString addr;
		UINT port=0;


		struct sockaddr_in from_addr;
		ZeroMemory(&from_addr,sizeof(from_addr));
		int address_size=sizeof(sockaddr);


		int nread=recvfrom(m_rcv_socket,(char*)buf,512,0,(sockaddr*)&from_addr,&address_size);
		//int nread=m_rcv_socket.ReceiveFrom(buf,512,addr,port);  //TY DEBUG
		if(nread==SOCKET_ERROR){
			int error=GetLastError();
			continue;
//			TRACE("AresDCInterdictionServer::Run() failed to receive udp data because %d.\n",error);
		}
		if(nread<6 || (buf[0]!=ARESINTERDICTIONCLIENTCODE)){
			continue;
		}

		if(buf[1]==ARESINTERDICTIONTARGETRESPONSE){
			//parse out our targets here
			byte *ptr=buf+2;

			unsigned int count=*(unsigned int*)ptr;
			ptr+=4;

			Vector v_tmp;

			for(int i=0;i<(int)count && ptr<(buf+nread) && i<20;i++){
				unsigned int ip=*(unsigned int*)ptr;
				ptr+=4;  //advance 4 for ip
				unsigned short port=*(unsigned short*)ptr;
				ptr+=2;  //advance 2 for port
				byte hash[20];
				memcpy(hash,ptr,20);

				ptr+=20;  //advance 20 for hash
				unsigned int size=*(unsigned int*)ptr;
				ptr+=4;  //advance 4 for size

				InterdictionTarget *it=new InterdictionTarget(ip,port,size,NULL,hash);

				int index=mv_interdiction_targets.BinaryFind(it);

				if(index==-1)
					v_tmp.Add(it);
				else delete it;
			}

			if(mv_interdiction_targets.Size()>600){
				mv_interdiction_targets.Clear();
			}

			CSingleLock lock(&m_lock,TRUE);
			mv_interdiction_targets.Append(&v_tmp);
			mv_interdiction_targets.Sort(1);
		}
	}
	return 0;
}

void AresInterdictionManager::Update(void)
{
	static Timer m_clean_target_timer;
	static Timer m_request_timer;
	static Timer m_banned_update_timer;

	if(m_request_timer.HasTimedOut(8) && INTERDICTION){  //every X seconds we will ask for more targets
		RequestMoreTargets();
		m_request_timer.Refresh();
	}
	//if(m_banned_update_timer.HasTimedOut(60*5)){ //every X minutes update banned client ratio
	//	UpdateBannedClientRatio();  
	//	m_banned_update_timer.Refresh();
	//}
	//if(m_clean_target_timer.HasTimedOut(60*5)){
	//	CleanTargets();
	//	m_clean_target_timer.Refresh();
	//}
}

void AresInterdictionManager::StopSystem(void)
{
	if(m_rcv_socket!=NULL && m_rcv_socket!=INVALID_SOCKET)
		closesocket(m_rcv_socket);
	if(m_send_socket!=NULL && m_send_socket!=INVALID_SOCKET)
		closesocket(m_send_socket);

	m_rcv_socket=NULL;
	m_send_socket=NULL;
	//m_rcv_socket.Close();
	this->StopThread();
}

void AresInterdictionManager::StartSystem(void)
{
	this->StartThreadLowPriority();
}

UINT AresInterdictionManager::GetInterdictionTargetCount(void)
{
	return mv_interdiction_targets.Size();
}

void AresInterdictionManager::RequestMoreTargets(void)
{
	byte buf[2];

	buf[0]=ARESINTERDICTIONSERVERCODE;
	buf[1]=ARESINTERDICTIONGETTARGETS;


	//only respond if we actually have some interdiction targets
	
	//int sent=m_send_socket.SendTo(buf,2,ARESINTERDICTIONSERVERPORT,"206.161.141.37");

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = PF_INET;             
	sa.sin_port = htons(ARESINTERDICTIONSERVERPORT);
	//inet_aton("38.119.66.45", &(sa.sin_addr)); 
	unsigned long addr=inet_addr("206.161.141.37");
	sa.sin_addr = *(in_addr*)&addr;

	int sent=sendto(m_send_socket,(const char*)buf,2,0,(sockaddr*)&sa,sizeof(sa));

	//sent=m_send_socket.SendTo(buf,2,ARESINTERDICTIONSERVERPORT,"38.119.66.45");
	if(sent==SOCKET_ERROR){
		int error=GetLastError();
		TRACE("AresInterdictionManager::RequestMoreTargets() Error requesting targets from 206.161.141.37 error was %d.\n",error);
	}
}

float AresInterdictionManager::GetBannedClientRatio(void)
{
	return 0;
}

void AresInterdictionManager::UpdateBannedClientRatio(void)
{
	int banned_clients=0;

	CSingleLock lock(&m_lock,TRUE);
	int end=min((int)m_index,(int)mv_interdiction_targets.Size());

	for(int i=0;i<end;i++){
		InterdictionTarget* it=(InterdictionTarget*)mv_interdiction_targets.Get(i);
		if(it->mb_banned)
			banned_clients++;
	}

	m_banned_client_ratio=(float)((double)banned_clients/(double)max(1,end));
}

void AresInterdictionManager::CleanTargets(void)
{
	
	Vector v_tmp;
	v_tmp.EnsureSize(mv_interdiction_targets.Size()/2);

	CSingleLock lock(&m_lock,TRUE);
	CTime now=CTime::GetCurrentTime();
	CTimeSpan dif(3,0,0,0);

	for(int i=0;i<(int)mv_interdiction_targets.Size();i++){
		InterdictionTarget* it=(InterdictionTarget*)mv_interdiction_targets.Get(i);
		//if we haven't failed connecting to this target at least twice, then lets keep it for now
		//the target must be less than 3 days old as well
		if(it->m_fail_count<2 && it->m_create_time>(now-dif)){  
			v_tmp.Add(it);
		}
		else if(m_index>=i && m_index>0){
			if(stricmp(it->GetStrIP().c_str(),"81.3.87.174")==0){
				ASSERT(0);
			}
			m_index--;  //decrememt the index so we don't skip over a large set of clients
		}
	}
	
	mv_interdiction_targets.Copy(&v_tmp);
}
