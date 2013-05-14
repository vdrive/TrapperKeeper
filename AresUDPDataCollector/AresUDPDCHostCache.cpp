#include "StdAfx.h"
#include "aresudpdchostcache.h"
#include "..\tkcom\timer.h"
#include "..\tkcom\TinySQL.h"
#include "..\AresProtector\AresHost.h"
#include "AresUDPDataCollectorSystem.h"


AresUDPDCHostCache::AresUDPDCHostCache(void)
{
	m_index1=0;
	m_index2=0;

	m_user_index=0;
	m_num_hosts=0;
}

AresUDPDCHostCache::~AresUDPDCHostCache(void)
{
}


UINT AresUDPDCHostCache::Run(void)
{
	srand(GetTickCount());
	int delay=10+(rand()%LOADDELAY);  //TYDEBUG

	Timer last_sn_request;
	Timer last_host_save;

	UINT save_attempt=0;

	while(!this->b_killThread){
		Sleep(100);

	
		if(last_sn_request.HasTimedOut(5)){
			last_sn_request.Refresh();
			m_interface.RequestSupernodes();
		}

		if(last_host_save.HasTimedOut(15)){
			last_host_save.Refresh();
			int added=0;
			while(m_interface.mv_saved_hosts.Size()>0){
				SaveHostObject *sh=(SaveHostObject*)m_interface.mv_saved_hosts.Get(0);

				int index=GetBinFromPeer(sh->m_ip.c_str());
				bool b_found=false;
				
				for(int i=0;i<(int)mv_hosts_array[index].Size();i++){
					AresHost* ah=(AresHost*)mv_hosts_array[index].Get(i);
					if(stricmp(ah->GetIP(),sh->m_ip.c_str())==0){
						b_found=true;
						break;
					}
				}
				if(!b_found){
					if(sh->m_port!=4685 && sh->m_port!=BASEPORT){
						mv_hosts_array[index].Add(new AresHost(sh->m_ip.c_str(),sh->m_port));
						m_num_hosts++;
						added++;
					}
				}

				m_interface.mv_saved_hosts.Remove(0);
			}
			if(added>0){
				SystemRef ref;
				CString log_msg;
				log_msg.Format("Received %u new hosts bring total to %u hosts\n",added,m_num_hosts);
				ref.System()->Log(log_msg);
			}
		}
	}
	return 0;
}

bool AresUDPDCHostCache::GetNextHost(Vector &v,bool tcp_ip)
{

	if(tcp_ip && mv_reconnect_hosts.Size()>0){
		//we have a host we'd like to try to reconnect to.  This was probably a host we've been connected to but lost the connection for whatever reason.
		AresHost *ah=(AresHost*)mv_reconnect_hosts.Get(0);
		v.Add(ah);
		mv_reconnect_hosts.Remove(0);
		return true;
	}


	int attempt=0;
	while(v.Size()==0 && attempt++<(NUMCONBINS/4)){
		if(m_index1>=NUMCONBINS)
			m_index1=0;

		if(m_index2>=mv_hosts_array[m_index1].Size()){
			m_index2=0;
			m_index1++;  //advance our bin index by 1
		}

		if(m_index2>=mv_hosts_array[m_index1].Size()){
			continue;
		}
		else{
			v.Add(mv_hosts_array[m_index1].Get(m_index2++));
		}
	}

	return true;
}

UINT AresUDPDCHostCache::GetKnownHosts(void)
{
	return m_num_hosts;
}

bool AresUDPDCHostCache::IsReady(void)
{
	return (m_num_hosts>0)?true:false;
}

void AresUDPDCHostCache::SaveHost(const char* host_ip, unsigned short port)
{
	if(mv_hosts_to_save.Size()<500){
		
		//see if we already know about this host
		int index=GetBinFromPeer(host_ip);

		bool b_found=false;
		
		for(int i=0;i<(int)mv_hosts_array[index].Size();i++){
			AresHost* ah=(AresHost*)mv_hosts_array[index].Get(i);
			if(stricmp(ah->GetIP(),host_ip)==0){
				b_found=true;
				break;
			}
		}

		if(!b_found){
			//to cut down on traffic only save this if this client doesn't know about it.
			//if this client does know about it, then that means the supernode server knows about it because thats where we got it.
			mv_hosts_to_save.Add(new SaveHostObject(host_ip,port));
		}
	}
}

bool AresUDPDCHostCache::GetRandomMDHost(Vector &v_tmp)
{
	CSingleLock lock(&m_md_host_lock,TRUE);
	if(mv_md_hosts.Size()==0)
		return false;

	int rand_index=rand()%mv_md_hosts.Size();
	AresHost* h=(AresHost*)mv_md_hosts.Get(rand_index);
	v_tmp.Add(h);
	return true;
}

void AresUDPDCHostCache::StartSystem(void)
{
	m_interface.StartSystem();
	this->StartThreadLowPriority();
}

void AresUDPDCHostCache::StopSystem(void)
{
	this->StopThread();
	m_interface.StopSystem();
}

void AresUDPDCHostCache::SNInterface::ReceivedSupernodes(vector<string> &v_ips, vector<unsigned short> &v_ports){
	for(int i=0;i<(int)v_ips.size();i++){
		mv_saved_hosts.Add(new SaveHostObject(v_ips[i].c_str(),v_ports[i]));
	}
}

//call this to prioritize reconnecting to a host.  This should probably be done if we lost a connection to a good host.
void AresUDPDCHostCache::AttemptReconnection(const char* ip, unsigned short port)
{
	mv_reconnect_hosts.Add(new AresHost(ip,port));
}