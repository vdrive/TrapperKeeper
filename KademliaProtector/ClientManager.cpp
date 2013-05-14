#include "StdAfx.h"
#include "clientmanager.h"
#include "KademliaProtectorSystem.h"
#include "FTCreditTracker.h"

ClientManager::ClientManager(void)
{
}

ClientManager::~ClientManager(void)
{
}

void ClientManager::AddClient(KadFTConnection* con)
{
	mv_clients.Add(con);
	
}

UINT ClientManager::Run(void)
{

	TRACE("ClientManager::Run(void) THREAD STARTED\n");
	KademliaProtectorSystemReference ref;

	//Timer last_credit_update;
	//Timer last_credit_drain;

	while(!this->b_killThread){
		Sleep(300);

		bool refresh_last_credit_drain=false;

		//give some credits back every 10 minutes or so
		/*
		if(last_credit_update.HasTimedOut(60*10)){
			last_credit_update.Refresh();
			for(int bin_index=0;bin_index<NUMCONBINS;bin_index++){
				for(int j=0;j<(int)mv_client_credits[bin_index].Size();j++){
					FTCreditTracker* ftct=(FTCreditTracker*)mv_client_credits[bin_index].Get(j);
					ftct->GainCredit();
				}
			}
		}*/

		ref.System()->LogToFile("ClientManager::Run(void) BEGIN");
		for(int i=0;i<(int)mv_clients.Size();i++){
			KadFTConnection *kc=(KadFTConnection*)mv_clients.Get(i);
			
			/*
			int bin_index=GetBinFromPeer(kc->GetIP());
			bool b_continue=false;
			for(int j=0;j<(int)mv_client_credits[bin_index].Size();j++){
				FTCreditTracker* ftct=(FTCreditTracker*)mv_client_credits[bin_index].Get(j);
				if(stricmp(ftct->m_ip.c_str(),kc->GetIP())==0){
					if(last_credit_drain.HasTimedOut(60*5)){
						//every X minutes credit each of these so they don't stay connected forever
						refresh_last_credit_drain=true;
						ftct->m_credits++;
					}
					if(ftct->m_credits>20){
						//if this client has been active too long, disconnect him
						mv_terminal_clients.Add(kc);
						mv_clients.Remove(i);
						i--;
						b_continue=true;
					}
					break;
				}
			}
			if(b_continue)
				continue;
				*/

			if(kc->IsOld() || (i==0 && mv_clients.Size()>1000)){
				mv_terminal_clients.Add(kc);
				mv_clients.Remove(i);
				i--;
				continue;
			}
			kc->Update();
		}

//		if(refresh_last_credit_drain)
//			last_credit_drain.Refresh();
		ref.System()->LogToFile("ClientManager::Run(void) END");
	}

	TRACE("ClientManager::Run(void) THREAD TERMINATED\n");
	return 0;
}

void ClientManager::Shutdown(void)
{
	TRACE("ClientManager::Shutdown(void) STARTED\n");
	this->StopThread();
	int count=mv_clients.Size();
	for(int i=0;i<(int)mv_clients.Size();i++){
		KadFTConnection *kc=(KadFTConnection*)mv_clients.Get(i);
		kc->Shutdown();
		mv_clients.Remove(i);
		i--;
	}

	
	mv_clients.Clear();
	

	TRACE("ClientManager::Shutdown(void) FINISHED, %d CLIENTS TERMINATED\n",count);
}

UINT ClientManager::GetClientCount(void)
{
	return mv_clients.Size();
}
void ClientManager::GuiClearTerminalClients(void)
{
	for(int i=0;i<(int)mv_terminal_clients.Size();i++){
		KadFTConnection *kc=(KadFTConnection*)mv_terminal_clients.Get(i);
		kc->Shutdown();
		mv_terminal_clients.Remove(i);
		i--;
	}

}

void ClientManager::UseCredit(const char* peer)
{
	/*
	int index=GetBinFromPeer(peer);
	for(int j=0;j<(int)mv_client_credits[index].Size();j++){
		FTCreditTracker* ftct=(FTCreditTracker*)mv_client_credits[index].Get(j);
		if(stricmp(peer,ftct->m_ip.c_str())==0){
			ftct->UseCredit();
			if(ftct->m_credits==15)
				ftct->m_credits=18;  //force at least small ban of this client
			return;
		}
	}

	mv_client_credits[index].Add(new FTCreditTracker(peer));
	*/
}

bool ClientManager::HasEnoughCreditToConnect(const char* peer)
{
	int index=GetBinFromPeer(peer);
	for(int j=0;j<(int)mv_client_credits[index].Size();j++){
		FTCreditTracker* ftct=(FTCreditTracker*)mv_client_credits[index].Get(j);
		if(stricmp(peer,ftct->m_ip.c_str())==0){
			if(ftct->m_credits>15)
				return false;
			else return true;
		}
	}
	return true;
}
