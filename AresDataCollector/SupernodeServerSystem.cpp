#include "StdAfx.h"
#include "supernodeserversystem.h"
#include "..\tkcom\tinysql.h"

#define ARESSUPERNODESERVERPORT 4692
#define ARESSUPERNODECLIENTPORT 4693
#define ARESSUPERNODESERVERCODE 57
#define ARESSUPERNODECLIENTCODE 58
#define ARESSUPERNODEGETSUPERNODES 100
#define ARESSUPERNODEREPORTSUPERNODES 101


SupernodeServerSystem::SupernodeServerSystem(void)
{
	m_md_server_count=0;
	m_loaded_supernode_count=0;
	m_bad_supernode_count=0;

	AfxSocketInit();
	BOOL stat=m_rcv_socket.Create(ARESSUPERNODESERVERPORT,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP SupernodeServerSystem Socket %d\n",error);
	}
	stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP SupernodeServerSystem Socket %d\n",error);
	}

	m_forward_index1=0;
	m_forward_index2=0;
}

SupernodeServerSystem::~SupernodeServerSystem(void)
{
}


void SupernodeServerSystem::StartSystem(void)
{
	this->StartThreadLowPriority();

}

void SupernodeServerSystem::StopSystem(void)
{
	m_rcv_socket.Close();
	m_send_socket.Close();
	this->StopThread();

	//signal all threads to die
	for(int i=0;i<(int)mv_supernode_savers.Size();i++){
		SupernodeSaver* sn=(SupernodeSaver*)mv_supernode_savers.Get(i);
		sn->b_killThread=1;
	}

	//make sure they are all dead now
	for(int i=0;i<(int)mv_supernode_savers.Size();i++){
		SupernodeSaver* sn=(SupernodeSaver*)mv_supernode_savers.Get(i);
		sn->StopThread();
	}
	
}

UINT SupernodeServerSystem::Run(void)
{
	//load the supernodes so we have something to start with
	TinySQL sql;
	sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306);

	if(sql.Query("SELECT ip,port from ares_hosts",true)){

		//bin these results for easy lookup
		for(int i=0;i<(int)sql.m_num_rows;i++){
			AresSupernode *as=new AresSupernode(sql.mpp_results[i][0].c_str(),atoi(sql.mpp_results[i][1].c_str()));
			if(as->m_port==4685){
				delete as;
				continue;
			}
			else{
				int index=GetBinFromPeer(as->m_ip);
				mv_supernodes[index].Add(as);
				m_loaded_supernode_count++;
			}
		}
	}


	Timer last_supernode_save;
	while(!this->b_killThread){
		byte buf[512];
		CString addr;
		UINT port=0;
		int nread=m_rcv_socket.ReceiveFrom(buf,512,addr,port);  //TY DEBUG
		if(nread==SOCKET_ERROR){
			int error=GetLastError();
			Sleep(10);
			continue;
//			TRACE("AresDCInterdictionServer::Run() failed to receive udp data because %d.\n",error);
		}
		if(nread<2 || (buf[0]!=ARESSUPERNODESERVERCODE)){
			continue;
		}

		byte *p_data=buf+2;

		if(buf[1]==ARESSUPERNODEGETSUPERNODES){
			RespondToGetSupernodes(addr);  //send X supernodes back at this server
		}
		else if(buf[1]==ARESSUPERNODEREPORTSUPERNODES && nread>=6){
			WORD count=*(WORD*)p_data;
			p_data+=2;
			vector <UINT> v_ips;
			vector <unsigned short> v_ports;
			for(int i=0;i<count && p_data<=((buf+nread)-6);i++){
				v_ips.push_back(*(UINT*)p_data);
				p_data+=4;
				v_ports.push_back(*(unsigned short*)p_data);
				p_data+=2;	
			}
			if(v_ips.size()>0){
				AddSupernodes(v_ips,v_ports,addr);
			}
		}

		if(last_supernode_save.HasTimedOut(60*45)){
			last_supernode_save.Refresh();
			//make sure they are all dead now
			SaveSupernodes();
		}
	}
	return 0;
}

void SupernodeServerSystem::AddSupernodes(vector<UINT>&v_ips,vector<unsigned short>v_ports,const char* src_ip)
{
	UINT src_int_ip=inet_addr(src_ip);

	int src_index=GetBinFromPeer(src_int_ip);
	bool b_found_src=false;
	for(int i=0;i<(int)mv_md_servers[src_index].Size();i++){
		MDServer *md=(MDServer*)mv_md_servers[src_index].Get(i);
		if(md->m_ip==src_int_ip){
			md->m_discovery_time.Refresh();
			b_found_src=true;
			break;
		}
		else if(md->m_discovery_time.HasTimedOut(60*60*24)){
			mv_md_servers[src_index].Remove(i);
			i--;
			m_md_server_count--;
		}
	}

	if(!b_found_src){
		m_md_server_count++;
		mv_md_servers[src_index].Add(new MDServer(src_int_ip));
	}

	vector<UINT> v_new_supernode_ips;
	vector<unsigned short> v_new_supernode_ports;
	for(int ip_index=0;ip_index<(int)v_ips.size();ip_index++){
		int index=GetBinFromPeer(v_ips[ip_index]);
		bool b_found=false;
		bool b_found_same_ip=false;
		for(int i=0;i<(int)mv_supernodes[index].Size();i++){
			AresSupernode *as=(AresSupernode*)mv_supernodes[index].Get(i);
			if(as->m_ip==v_ips[ip_index] && as->m_port==v_ports[ip_index]){
				as->m_discovery_time.Refresh();
				as->mb_farmed=true;
				b_found=true;
			}
			else if(as->m_ip==v_ips[ip_index]){ //same ip but different ports???!
				//flag this supernode as bad
				if(!as->mb_bad){
					m_bad_supernode_count++;
				}
				as->mb_bad=true;
				as->m_discovery_time.Refresh();
				as->mb_farmed=true;
				b_found_same_ip=true;
			}
			else if(as->m_discovery_time.HasTimedOut(60*60*24*3)){
				//only keep current list of supernodes
				if(as->mb_bad)
					m_bad_supernode_count--;
				mv_supernodes[index].Remove(i);
				i--;
				m_loaded_supernode_count--;
			}
		}

		//we have a new supernode
		if(!b_found && !b_found_same_ip && v_ports[ip_index]!=30484){  //don't add multiple supernodes with the same port, also don't add our own supernodes
			m_loaded_supernode_count++;
			AresSupernode *as=new AresSupernode(v_ips[ip_index],v_ports[ip_index]);

			if(b_found_same_ip){
				//this particular combination is new, but we are tracking another supernode with the same ip but different port
				//this leads us to conclude that this is not a valid supernode for whatever reason
				as->mb_bad=true;
				m_bad_supernode_count++;
			}

			as->mb_farmed=true;
			mv_supernodes[index].Add(as);
			if(!b_found_same_ip){
				v_new_supernode_ips.push_back(v_ips[ip_index]);
				v_new_supernode_ports.push_back(v_ports[ip_index]);
			}
		}
	}

	static Timer last_report;

	
	if(v_new_supernode_ips.size()>0 && last_report.HasTimedOut(1)){
		last_report.Refresh();
		ReportNewSupernodes(v_new_supernode_ips,v_new_supernode_ports);  //let all racks know about this one immediately so we can focus on it
	}
}

void SupernodeServerSystem::SaveSupernodes(void)
{
	bool b_running=false;
	while(mv_supernode_savers.Size()>0){
		SupernodeSaver* sn=(SupernodeSaver*)mv_supernode_savers.Get(0);
		if(!sn->b_threadRunning){
			sn->StopThread();
			mv_supernode_savers.Remove(0);
			continue;
		}
		else{
			return;
		}
	}

	Vector *v_tmp=new Vector();
	//make a single list containing all the supernode objects
	for(int i=0;i<NUMCONBINS;i++){  
		for(int j=0;j<(int)mv_supernodes[i].Size();j++){
			v_tmp->Add(mv_supernodes[i].Get(j));
		}
	}

	//offload this work onto another thread
	SupernodeSaver *nss=new SupernodeSaver(v_tmp);
	nss->StartThreadLowPriority();
	mv_supernode_savers.Add(nss);
}

void SupernodeServerSystem::ReportNewSupernodes(vector<UINT> &v_ips,vector<unsigned short> &v_ports)
{
	//broadcast this new supernode to all of our racks

	int start_index=rand()%NUMCONBINS;

	int broadcast_count=0;

	int buf_len=2+2+(int)v_ips.size()*6;
	byte *buf=new byte[buf_len];

	buf[0]=ARESSUPERNODECLIENTCODE;
	buf[1]=ARESSUPERNODEREPORTSUPERNODES;

	byte *ptr=buf+2;  //skip past first two opcodes
	*(WORD*)ptr=(WORD)v_ips.size();  //write in our size
	ptr+=2;
	for(int i=0;i<(int)v_ips.size();i++){
		*(UINT*)ptr=v_ips[i];
		ptr+=4;
		*(unsigned short*)ptr=v_ports[i];
		ptr+=2;
	}


	//send this list of new supernodes to a select group of servers
	for(int i=start_index;i<(start_index+(NUMCONBINS/2)) && broadcast_count<50;i++){  //broadcast new supernode to a lot of our racks
		for(int j=0;j<(int)mv_md_servers[i%NUMCONBINS].Size() && broadcast_count<50;j++){
			broadcast_count++;
			MDServer *md=(MDServer*)mv_md_servers[i%NUMCONBINS].Get(j);

			char md_str_ip[17];
			sprintf( md_str_ip , "%u.%u.%u.%u" , (md->m_ip>>0)&0xff , (md->m_ip>>8)&0xff , (md->m_ip>>16) & 0xff ,(md->m_ip>>24) & 0xff );  //little endian

			int sent=m_send_socket.SendTo(buf,buf_len,ARESSUPERNODECLIENTPORT,md_str_ip);
			if(sent==SOCKET_ERROR){
				int error=GetLastError();
				TRACE("AresDCInterdictionServer::RespondTargets() Error sending ping response to %s, error was %d.\n",md_str_ip,error);
			}
		}
	}

	delete[] buf;
}

UINT SupernodeServerSystem::GetMDServerCount(void)
{
	return m_md_server_count;
}

UINT SupernodeServerSystem::GetLoadedSupernodeCount(void)
{
	return m_loaded_supernode_count;
}

void SupernodeServerSystem::RespondToGetSupernodes(const char* src_ip)
{
	UINT src_int_ip=inet_addr(src_ip);

	//make sure this requester is in our list of clients
	int src_index=GetBinFromPeer(src_int_ip);
	bool b_found_src=false;
	for(int i=0;i<(int)mv_md_servers[src_index].Size();i++){
		MDServer *md=(MDServer*)mv_md_servers[src_index].Get(i);
		if(md->m_ip==src_int_ip){
			md->m_discovery_time.Refresh();
			b_found_src=true;
			break;
		}
		else if(md->m_discovery_time.HasTimedOut(60*60*24)){
			mv_md_servers[src_index].Remove(i);
			i--;
			m_md_server_count--;
		}
	}

	if(!b_found_src){
		m_md_server_count++;
		mv_md_servers[src_index].Add(new MDServer(src_int_ip));
	}

	vector<UINT> v_ips;
	vector<unsigned short> v_ports;
	for(int send_index=0;v_ips.size()<45 && v_ips.size()<m_loaded_supernode_count && send_index<100;send_index++){
		if(m_forward_index1>=NUMCONBINS)
			m_forward_index1=0;
		if(m_forward_index2>=mv_supernodes[m_forward_index1].Size()){
			m_forward_index2=0;
			m_forward_index1++;  //advance our bin index by 1
		}
		
		if(m_forward_index1<NUMCONBINS && m_forward_index2<mv_supernodes[m_forward_index1].Size()){
			AresSupernode* tmp_host=(AresSupernode*)mv_supernodes[m_forward_index1].Get(m_forward_index2++);

			v_ips.push_back(tmp_host->m_ip);
			v_ports.push_back(tmp_host->m_port);
		}
	}

	if(v_ips.size()==0)
		return;

	int buf_len=2+2+(int)v_ips.size()*6;
	byte *buf=new byte[buf_len];

	buf[0]=ARESSUPERNODECLIENTCODE;
	buf[1]=ARESSUPERNODEREPORTSUPERNODES;

	byte *ptr=buf+2;  //skip past first two opcodes
	*(WORD*)ptr=(WORD)v_ips.size();
	ptr+=2;

	for(int i=0;i<(int)v_ips.size();i++){
		*(UINT*)ptr=v_ips[i];
		ptr+=4;
		*(unsigned short*)ptr=v_ports[i];
		ptr+=2;
	}

	int sent=m_send_socket.SendTo(buf,buf_len,ARESSUPERNODECLIENTPORT,src_ip);
	if(sent==SOCKET_ERROR){
		int error=GetLastError();
		TRACE("SupernodeServerSystem::RespondToGetSupernodes() Error sending udp packet to %s, error was %d.\n",src_ip,error);
	}
	delete []buf;
}

UINT SupernodeServerSystem::SupernodeSaver::Run(void)
{
	CString query="insert ignore into ares_hosts values ";
	TinySQL sql;
	sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306);

	srand(GetTickCount());

	int start_index=0;
	if(mpv_supernodes->Size()>5000){ //too many supernodes to save all at once, just choose a subset
		start_index=(2*rand())%max(1,mpv_supernodes->Size());
	}

	int count=0;
	for(int i=0;i<(int)mpv_supernodes->Size() && count<10000;i++){ 
		AresSupernode *as=(AresSupernode*)mpv_supernodes->Get((i+start_index)%mpv_supernodes->Size());
		
		CString tmp;

		if(as->mb_farmed){  //only insert this one if its one marked as having been farmed
			char str_ip[16];
			sprintf( str_ip , "%u.%u.%u.%u" , (as->m_ip>>0)&0xff , (as->m_ip>>8)&0xff , (as->m_ip>>16) & 0xff ,(as->m_ip>>24) & 0xff );  //little endian

			if(count>0)
				tmp.Format(",('%s',%d,now())",str_ip,as->m_port);
			else
				tmp.Format("('%s',%d,now())",str_ip,as->m_port);

			query+=tmp;
		
			if(rand()&1){
				CString tmp_query;
				tmp_query.Format("update ares_hosts set time_created=now() where ip='%s' and port=%u",str_ip,as->m_port);
				sql.Query(tmp_query,false);
			}
			count++;
		}
	}

	if(!this->b_killThread && count>0)
		sql.Query(query,false);

	return 0;
}