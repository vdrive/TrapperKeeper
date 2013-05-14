#include "StdAfx.h"
#include "connectionmanager.h"
//#include "FastTrackGiftDll.h"
#include "imagehlp.h"
//#include "VendorCount.h"
#include "WSocket.h"
#include "BTClientDll.h"
#include "ListeningSocket.h"
#include "ClientData.h"
#include "Config.h"

#define MAX_HOST_CACHE_SIZE			5000

ConnectionManager::ConnectionManager(void)
{
	//srand((unsigned)time(NULL));
	//ReadInHostCache();
	//SupernodeHost host;
	//host.SetIP("38.118.163.133");
	//host.m_port = 3639;
	//hs_host_cache.insert(host);
	//host.SetIP("38.119.64.12");
	//hs_host_cache.insert(host);
	//m_rng.Reseed(true,32);
	//m_vendor_counts_enabled=false;
	//m_connect_to_supernode=true;

	m_refresh_modules = false;
	m_num_mods = 100000;
	m_next_port = 6881;

}

//
//
//
ConnectionManager::~ConnectionManager(void)
{
	// Free memory (if it wasn't already done by KillModules()
	for(int i=0;i<(int)v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();

	for(int j=0;j<(int)v_listening_sockets.size();j++)
	{
		delete v_listening_sockets[j];
	}
	v_listening_sockets.clear();
}
/*
//
//
//
void ConnectionManager::SetFileSharingManager(FileSharingManager *file_sharing_manager)
{
	p_file_sharing_manager=file_sharing_manager;
}
*/
//
//
//

void ConnectionManager::InitParent(BTClientDll * parent)
{
	p_parent = parent;
}


void ConnectionManager::LogMess(char * msg)
{
	
	CFile logfile;
	
	if (logfile.Open("BTConnectionManagerLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
	{
		if(logfile.GetLength()> 100000000)
		{
			logfile.Close();
			remove("BTConnectionManagerLog.txt");
			logfile.Open("BTConnectionManagerLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite);
		}
		logfile.SeekToEnd();
		logfile.Write(msg, (unsigned int)strlen(msg));
		logfile.Write("\r\n",2);
		logfile.Close();
	}
	

}
void ConnectionManager::StartNewTorrent(TorrentFile * tf)
{

	LogMess("Start New Torrent");
	// If it's time to reconnect we'll flush the old modules and start new ones
	if (m_refresh_modules == true)
	{
		KillModules();
	}


	TorrentFile temptorrent;
	int found = 0;
	int listeningport = 6881;
	
	// Check and see if this is a new torrent and needs a new listening mod
	if ((m_config.m_listening == true)&&((int)v_listening_mods.size() < m_config.m_max_listeners ))
	{
		for (int i=0; i < (int)v_listening_mods.size(); i++)
		{
			if (strcmp(v_torrents[i].GetName().c_str(),tf->GetName().c_str()) == 0)
			{

				if(v_listening_mods[i] != NULL)
				{
					found = 1;
					listeningport = v_listening_mods[i]->GetListeningPort();
					break;
				}			

			}
		}

		if (found == 0)
		{
		//Create a new listening socket and Mod here
		ListeningSocket *newsocket = new ListeningSocket();
		listeningport = m_next_port;
		
		newsocket->InitParent(this,(int)v_listening_sockets.size());
		newsocket->Create(listeningport);
		newsocket->Listen();
		v_listening_sockets.push_back(newsocket);
		AddListeningModule(tf,listeningport);
		v_torrents.push_back(*tf);
		}
	}

	/*
	m_refresh_modules = false;
	AddModule(tf,listeningport);	
	LogMess("Start New Torrent Ended");
*/
	found = 0;
	
	for (int j=0; j <	(int)v_mods.size(); j++)
	{
	
		TorrentFile temptorrent = v_mods[j]->GetTorrentFile();
		if ((strcmp(tf->GetName().c_str(),temptorrent.GetName().c_str()) == 0) && (tf->GetLength() == temptorrent.GetLength()))
		{
			v_mods[j]->ConnectToNewClients(tf);
			found = 1;
		}
		

	}
	
	if (found == 0)
	{
		m_refresh_modules = false;
		AddModule(tf,listeningport);
		LogMess("Start New Torrent Ended");
	}


	LimitModuleCount(m_config.m_max_num_mods);

	//PostMessage(hwnd,WM_INIT_THREAD_DATA,(WPARAM)wparam, (LPARAM)&tf)

}
void ConnectionManager::AddModule(TorrentFile * tf,int listeningport)
{
	LogMess("Add Module Started");
	// Create a new module
	ConnectionModule *mod=new ConnectionModule(m_num_mods);
	m_num_mods++;
	mod->InitParent(this,(UINT)v_mods.size()+1);	// init parent pointer and module number
	mod->InitTorrent(tf,listeningport);
	v_mods.push_back(mod);
	LogMess("Add Module Ended");

}

void ConnectionManager::AddListeningModule(TorrentFile * tf, int listeningport)
{
	// Create a new module
	ConnectionModule *mod=new ConnectionModule(m_next_port);
	mod->InitParent(this,(UINT)v_listening_mods.size()+1);	// init parent pointer and module number
	mod->InitTorrent(tf,listeningport);
	mod->SetAsListener();
	v_listening_mods.push_back(mod);
	m_next_port++;

}

void ConnectionManager::LogMsg(char* msg)
{
	p_parent->DisplayInfo(msg);
}


//
//
//
void ConnectionManager::ReportStatus(ConnectionModuleStatusData &status)
{
	//UINT i;

	//// Add hosts to the host cache
	//ReportHosts(status.v_host_cache);
	//if(m_connect_to_supernode)
	//{
	//	// Send hosts for the idle sockets
	//	vector<SupernodeHost> hosts;

	//	for(i=0;i<status.m_idle_socket_count;i++)
	//	{
	//		// If the host cache is empty, skip
	//		if(hs_host_cache.size()>0)
	//		{
	//			// pick random hosts to connect to
	//			//hosts.push_back(*(hs_host_cache.begin()));
	//			//hs_host_cache.erase(hs_host_cache.begin());
	//			int index = m_rng.GenerateWord32(0,hs_host_cache.size()-1);
	//			hash_set<SupernodeHost>::iterator iter = hs_host_cache.begin();
	//			for(int j=0;j<index;j++)
	//				iter++;
	//			hosts.push_back(*iter);
	//			hs_host_cache.erase(iter);
	//		}
	//		else
	//		{
	//			SupernodeHost host;
	//			host.SetIP("38.118.163.133");
	//			host.m_port = 3639;
	//			hosts.push_back(host);
	//			host.SetIP("38.119.64.12");
	//			hosts.push_back(host);
	//		}
	//	}

	//	if(hosts.size()>0)
	//	{
	//		status.p_mod->ConnectToHosts(hosts);
	//	}

	//	// Limit host cache
	//	while(hs_host_cache.size()>MAX_HOST_CACHE_SIZE)
	//	{
	//		//hs_host_cache.erase(hs_host_cache.begin());
	//		int index = m_rng.GenerateWord32(0,hs_host_cache.size()-1);
	//		hash_set<SupernodeHost>::iterator iter = hs_host_cache.begin();
	//		for(int j=0;j<index;j++)
	//			iter++;
	//		hs_host_cache.erase(iter);
	//	}
	//}

	//p_parent->ReportConnectionStatus(status);

	//char msg[1024];
	//sprintf(msg,"%u mods : %u sockets : %u hosts cached",v_mods.size(),v_mods.size() * 60,hs_host_cache.size());
	//p_parent->ShowSocketStatus(msg);
}

//
//
//
unsigned int ConnectionManager::ReturnModCount()
{
	return (UINT)v_mods.size();
}

//
// Called when the dll is going to be removed...so that the threads (hopefully) aren't still looking for the GUID cache when everything's freed
//
void ConnectionManager::KillModules()
{
	// Free memory
	LogMess("Killing Modules");
	for(int i=0;i<(int)v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
	LogMess("Modules Killed");
}


void ConnectionManager::KillModule(int modnum)
{
	// Free memory
	LogMess("Killing Module");
	for(int i=0;i<(int)v_mods.size();i++)
	{
		int killmodnum = (*(v_mods.begin()+i))->GetModnum();
		if (killmodnum == modnum)
		{
			delete *(v_mods.begin()+i);
			LogMess("Module deleted");
			v_mods.erase(v_mods.begin()+i);
			LogMess("Module erased");
			p_parent->RemoveModule(modnum);
			return;
		}

	}

	LogMess("Did not find the module, something went wrong");
}
//
//
//
void ConnectionManager::LimitModuleCount(int count)
{
	while((int)v_mods.size()>count)
	{
		LogMess("Limiting modules");
		delete *(v_mods.begin());
		v_mods.erase(v_mods.begin());
	}
}
/*
//
//
//
void ConnectionManager::OnHeartbeat()
{
	WriteOutHostCache();
}

//
//
//
void ConnectionManager::WriteOutHostCache()
{
	//// Open the hosts.dat file for writing...if the open fails, then who cares
	//CFile file;
	//MakeSureDirectoryPathExists("Host Cache\\");
	//if(file.Open("Host Cache\\supernodes.dat",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	//{
	//	return;
	//}

	//// Create a buffer in memory of the host cache to write out to the hosts.dat file
	//unsigned int buf_len=sizeof(unsigned int)+ (sizeof(unsigned int)+sizeof(unsigned short))*(UINT)hs_host_cache.size();	// # items, ip and port
	//char *buf=new char[buf_len];
	//char *ptr=buf;

	//// The number of hosts
	//*((unsigned int *)ptr)=(UINT)hs_host_cache.size();
	//ptr+=sizeof(unsigned int);

	//// Copy the hosts to the buffer
	//hash_set<SupernodeHost>::const_iterator iter = hs_host_cache.begin();
	//while(iter != hs_host_cache.end())
	//{
	//	*((unsigned int *)ptr)=iter->m_ip;
	//	ptr+=sizeof(unsigned int);
	//	*((unsigned short *)ptr)=iter->m_port;
	//	ptr+=sizeof(unsigned short);
	//	iter++;
	//}

	//// Write the buffer to file
	//file.Write(buf,buf_len);

	//delete [] buf;

	//file.Close();
}

//
//
//
void ConnectionManager::ReadInHostCache()
{
	//UINT i;

	//// If there is a hosts.dat file, read it into the hosts cache
	//CFile file;
	//if(file.Open("Host Cache\\supernodes.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==TRUE)
	//{
	//	// Create a buffer in memory to read in the hosts cache file
	//	unsigned int buf_len=(UINT)file.GetLength();
	//	char *buf=new char[buf_len];
	//	file.Read(buf,buf_len);		// read in the file
	//	file.Close();
	//	
	//	char *ptr=buf;

	//	// Read in the number of hosts in the hosts.dat file
	//	unsigned int num_hosts=*((unsigned int *)ptr);
	//	ptr+=sizeof(unsigned int);

	//	// Copy the hosts from the buffer
	//	for(i=0;i<num_hosts;i++)
	//	{	
	//		SupernodeHost host;
	//		host.m_ip = (*((unsigned int *)ptr));
	//		ptr+=sizeof(unsigned int);
	//		
	//		host.m_port = (*((unsigned short *)ptr));
	//		ptr+=sizeof(unsigned short);

	//		hs_host_cache.insert(host);
	//	}

	//	delete [] buf;
	//}
}

//
//
//
void ConnectionManager::ReportHosts(vector<SupernodeHost> &hosts)
{
	//int i,j;
	//vector<SupernodeHost> filtered_host_cache;
	//
	//// Filter the host cache with the people we are already connected to
	//for(i=0;i<(int)hosts.size();i++)
	//{
	//	// Check with all of the mods with the hosts that they are reporting as being currently connected to
	//	bool found=false;
	//	for(j=0;j<(int)v_mods.size();j++)
	//	{
	//		if(v_mods[j]->IsConnected(hosts[i].m_ip))
	//		{
	//			found=true;
	//			break;
	//		}
	//	}

	//	if(!found)
	//	{
	//		filtered_host_cache.push_back(hosts[i]);
	//	}
	//}

	//// Add these hosts to the host cache
	//for(i=0;i<(int)filtered_host_cache.size();i++)
	//{
	//	// Check to see if they are already in the hosts vector
	//	pair< hash_set<SupernodeHost>::iterator, bool > pr;
	//	pr = hs_host_cache.insert(filtered_host_cache[i]);
	//}
}

//
// 1 sec
//
void ConnectionManager::TimerHasFired()
{
	UINT i;

	if(m_vendor_counts_timer_counter < 10)
		m_vendor_counts_timer_counter++;
	else
		m_vendor_counts_timer_counter=10;
	// If it's been 10 seconds, get the vendor counts
	if(m_vendor_counts_timer_counter==10 && m_vendor_counts_enabled)
	{
		// Reset counters
		m_vendor_counts_timer_counter=0;
		v_vendor_counts.clear();

		// Tell all of the modules to report their vendor counts
		for(i=0;i<v_mods.size();i++)
		{
			v_mods[i]->ReportVendorCounts();
		}
	}

}

//
//
//
void ConnectionManager::VendorCountsReady(vector<VendorCount> *vendor_counts)
{
	//UINT i,j;

	//// Add this VendorCount object to my vector, and free the memory allocated in the thread
	//for(i=0;i<vendor_counts->size();i++)
	//{
	//	// If this vendor is in my vector, then just increment the counter. Else, add a new item to my vector
	//	bool found=false;
	//	for(j=0;j<v_vendor_counts.size();j++)
	//	{
	//		if(strcmp(v_vendor_counts[j].m_vendor.c_str(),(*vendor_counts)[i].m_vendor.c_str())==0)
	//		{
	//			found=true;
	//			v_vendor_counts[j].m_count+=(*vendor_counts)[i].m_count;
	//			break;
	//		}
	//	}

	//	// If i don't know about this vendor count, add it to my vector
	//	if(!found)
	//	{
	//		v_vendor_counts.push_back((*vendor_counts)[i]);
	//	}
	//}

	//p_parent->VendorCountsReady(v_vendor_counts);

	//delete vendor_counts;	// free memory
}

//
//
//

//
//
//
void ConnectionManager::ReConnectAll()
{
	//p_parent->RemoveAllModules();
	//p_parent->Log("Reconnecting to all supernodes...");
	// Tell all of the mods to update their keywords vectors
	//delete all modules
	for(int i=0;i<(int)v_mods.size();i++)
	{
		//delete v_mods[i];
		v_mods[i]->ReConnectAll();
	}
	//v_mods.clear();
	//AddModule();
}
*/


void ConnectionManager::AddNewClient(SOCKET hSocket)
{

	for( int i = 0, len = (int)v_mods.size(); i < len; i++ )
	{
		if( v_mods[i]->HasIdleSocket())
		{
			v_mods[i]->AddNewClient(hSocket);
			return;
		}
	}
	
	ConnectionModule *pConMod = new ConnectionModule(m_num_mods);
	m_num_mods++;
	pConMod->AddNewClient( hSocket );
	v_mods.push_back( pConMod );
}

void ConnectionManager::SetRefreshFlag()
{
	m_refresh_modules = true;
}

void ConnectionManager::IncomingConnection(vector<SOCKET>sockets, int socknum)
{

	if (v_listening_mods[socknum] != NULL)
	{
		for (int i = 0; i < (int)sockets.size(); i++)
		{
			v_listening_mods[socknum]->AddNewClient(sockets[i]);
		}
	}
}

void ConnectionManager::ClientDataReported(ClientData * cdata)
{

	p_parent->ClientDataReported(cdata);

}

void ConnectionManager::ModuleDataReported(ModuleData * cdata)
{

	p_parent->ModuleDataReported(cdata);

}

void ConnectionManager::ReadConfig()
{
	m_config.ReadConfig();
}

Config ConnectionManager::GetConfig()
{

	return (m_config);

}