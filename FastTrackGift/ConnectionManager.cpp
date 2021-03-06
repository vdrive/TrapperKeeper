#include "StdAfx.h"
#include "connectionmanager.h"
#include "FastTrackGiftDll.h"
#include "imagehlp.h"
#include "VendorCount.h"
#include "WSocket.h"
#include "KeywordManager.h"

#define MAX_HOST_CACHE_SIZE			5000

ConnectionManager::ConnectionManager(void)
{
	srand((unsigned)time(NULL));
	ReadInHostCache();
	SupernodeHost host;
	host.SetIP("216.151.155.87");
	host.m_port = 3639;
	hs_host_cache.insert(host);
	m_search_id = rand()%65536; //starting with random search ID
	m_rng.Reseed(true,32);
	m_connect_to_supernode=false;
	m_vendor_counts_enabled=false;

#ifdef KAZAA_KILLER_ENABLED
		m_connect_to_supernode=true;
#endif
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
}

//
//
//
void ConnectionManager::SetKeywordManager(KeywordManager *keyword_manager)
{
	p_keyword_manager=keyword_manager;
}

//
//
//
void ConnectionManager::SetSupplyManager(SupplyManager *supply_manager)
{
	p_supply_manager=supply_manager;
}

//
//
//
void ConnectionManager::SetFileSharingManager(FileSharingManager *file_sharing_manager)
{
	p_file_sharing_manager=file_sharing_manager;
}

//
//
//
void ConnectionManager::AddModule()
{
	// Create a new module
	ConnectionModule *mod=new ConnectionModule;
	mod->InitParent(this,(UINT)v_mods.size()+1);	// init parent pointer and module number
	v_mods.push_back(mod);
}

//
//
//
void ConnectionManager::ReportStatus(ConnectionModuleStatusData &status)
{
	UINT i;

	// Add hosts to the host cache
	ReportHosts(status.v_host_cache);
	if(m_connect_to_supernode)
	{
		// Send hosts for the idle sockets
		vector<SupernodeHost> hosts;

		for(i=0;i<status.m_idle_socket_count;i++)
		{
			// If the host cache is empty, skip
			if(hs_host_cache.size()>0)
			{
				// pick random hosts to connect to
				//hosts.push_back(*(hs_host_cache.begin()));
				//hs_host_cache.erase(hs_host_cache.begin());
				int index = m_rng.GenerateWord32(0,hs_host_cache.size()-1);
				hash_set<SupernodeHost>::iterator iter = hs_host_cache.begin();
				for(int j=0;j<index;j++)
					iter++;
				hosts.push_back(*iter);
				hs_host_cache.erase(iter);
			}
			else
			{
				SupernodeHost host;
				host.SetIP("216.151.155.87");
				host.m_port = 3639;
				hosts.push_back(host);
				break;
			}
		}

		if(hosts.size()>0)
		{
			status.p_mod->ConnectToHosts(hosts);
		}

		// Limit host cache
		while(hs_host_cache.size()>MAX_HOST_CACHE_SIZE)
		{
			//hs_host_cache.erase(hs_host_cache.begin());
			int index = m_rng.GenerateWord32(0,hs_host_cache.size()-1);
			hash_set<SupernodeHost>::iterator iter = hs_host_cache.begin();
			for(int j=0;j<index;j++)
				iter++;
			hs_host_cache.erase(iter);
		}
	}

	p_parent->ReportConnectionStatus(status);

	char msg[1024];
	sprintf(msg,"%u mods : %u sockets : %u hosts cached",v_mods.size(),v_mods.size() * 60,hs_host_cache.size());
	p_parent->ShowSocketStatus(msg);
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
	for(int i=0;i<(int)v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
}

//
//
//
void ConnectionManager::LimitModuleCount(int count)
{
	while((int)v_mods.size()>count)
	{
		delete *(v_mods.end()-1);
		v_mods.erase(v_mods.end()-1);
	}
}

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
	// Open the hosts.dat file for writing...if the open fails, then who cares
	CFile file;
	MakeSureDirectoryPathExists("Host Cache\\");
	if(file.Open("Host Cache\\supernodes.dat",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		return;
	}

	// Create a buffer in memory of the host cache to write out to the hosts.dat file
	unsigned int buf_len=sizeof(unsigned int)+ (sizeof(unsigned int)+sizeof(unsigned short))*(UINT)hs_host_cache.size();	// # items, ip and port
	char *buf=new char[buf_len];
	char *ptr=buf;

	// The number of hosts
	*((unsigned int *)ptr)=(UINT)hs_host_cache.size();
	ptr+=sizeof(unsigned int);

	// Copy the hosts to the buffer
	hash_set<SupernodeHost>::const_iterator iter = hs_host_cache.begin();
	while(iter != hs_host_cache.end())
	{
		*((unsigned int *)ptr)=iter->m_ip;
		ptr+=sizeof(unsigned int);
		*((unsigned short *)ptr)=iter->m_port;
		ptr+=sizeof(unsigned short);
		iter++;
	}

	// Write the buffer to file
	file.Write(buf,buf_len);

	delete [] buf;

	file.Close();
}

//
//
//
void ConnectionManager::ReadInHostCache()
{
	UINT i;

	// If there is a hosts.dat file, read it into the hosts cache
	CFile file;
	if(file.Open("Host Cache\\supernodes.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==TRUE)
	{
		// Create a buffer in memory to read in the hosts cache file
		unsigned int buf_len=(UINT)file.GetLength();
		char *buf=new char[buf_len];
		file.Read(buf,buf_len);		// read in the file
		file.Close();
		
		char *ptr=buf;

		// Read in the number of hosts in the hosts.dat file
		unsigned int num_hosts=*((unsigned int *)ptr);
		ptr+=sizeof(unsigned int);

		// Copy the hosts from the buffer
		for(i=0;i<num_hosts;i++)
		{	
			SupernodeHost host;
			host.m_ip = (*((unsigned int *)ptr));
			ptr+=sizeof(unsigned int);
			
			host.m_port = (*((unsigned short *)ptr));
			ptr+=sizeof(unsigned short);

			hs_host_cache.insert(host);
		}

		delete [] buf;
	}
}

//
//
//
void ConnectionManager::ReportHosts(vector<SupernodeHost> &hosts)
{
	int i,j;
	vector<SupernodeHost> filtered_host_cache;
	
	// Filter the host cache with the people we are already connected to
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check with all of the mods with the hosts that they are reporting as being currently connected to
		bool found=false;
		for(j=0;j<(int)v_mods.size();j++)
		{
			if(v_mods[j]->IsConnected(hosts[i].m_ip))
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			filtered_host_cache.push_back(hosts[i]);
		}
	}

	// Add these hosts to the host cache
	for(i=0;i<(int)filtered_host_cache.size();i++)
	{
		// Check to see if they are already in the hosts vector
		pair< hash_set<SupernodeHost>::iterator, bool > pr;
		pr = hs_host_cache.insert(filtered_host_cache[i]);
	}
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
	UINT i,j;

	// Add this VendorCount object to my vector, and free the memory allocated in the thread
	for(i=0;i<vendor_counts->size();i++)
	{
		// If this vendor is in my vector, then just increment the counter. Else, add a new item to my vector
		bool found=false;
		for(j=0;j<v_vendor_counts.size();j++)
		{
			if(strcmp(v_vendor_counts[j].m_vendor.c_str(),(*vendor_counts)[i].m_vendor.c_str())==0)
			{
				found=true;
				v_vendor_counts[j].m_count+=(*vendor_counts)[i].m_count;
				break;
			}
		}

		// If i don't know about this vendor count, add it to my vector
		if(!found)
		{
			v_vendor_counts.push_back((*vendor_counts)[i]);
		}
	}

	p_parent->VendorCountsReady(v_vendor_counts);

	delete vendor_counts;	// free memory
}

//
//
//
void ConnectionManager::LogMsg(const char* msg)
{
	p_parent->Log(msg);
}

//
//
//
/*
void ConnectionManager::Search(const char* search)
{
	for(UINT i=0;i<v_mods.size();i++)
	{
		v_mods[i]->Search(search);
	}
}
*/

//
// Called by SupplyManager
//
void ConnectionManager::ProjectSupplyUpdated()
{
	// Tell all of the mods to update their supply
	/*
	for(UINT i=0;i<v_mods.size();i++)
	{
		v_mods[i]->ProjectSupplyUpdated();
	}
	*/
}

//
//
//
unsigned short ConnectionManager::PerformProjectSupplyQuery(char *project, unsigned int project_id)
{
	m_search_id++;
	UINT i=0;

	// Init the project supply query object with a GUID
	ProjectSupplyQuery psq;
	psq.m_search_id = m_search_id;
	psq.m_project_id=project_id;

	//store this guid into database
/*
	CString ts = CTime::GetCurrentTime().Format("%Y%m%d%H%M%S");
	CString cs_project = project;
	m_db_interface.InsertGUID(GNUTELLA_GUID_INDEX_TALBE, psq.m_guid, cs_project, ts);

	p_parent->m_log_window_manager.Log("Connection Manager : ",0x0000C0C0);	// dirty yellow
	p_parent->m_log_window_manager.Log("Starting Supply Query for project: ");
	p_parent->m_log_window_manager.Log(project,0x00FF0000,true);	// bold blue
	p_parent->m_log_window_manager.Log("\n");
*/	

	// Find the project in the keywords vector and give them to the query object to extract
	vector<ProjectKeywords>* keywords = p_keyword_manager->ReturnProjectKeywords();
	for(i=0;i<keywords->size();i++)
	{
		if(strcmp((*keywords)[i].m_project_name.c_str(),project)==0)
		{
			psq.ExtractProjectKeywordData(& (*keywords)[i]);
			psq.m_project_status_index=i;
			psq.m_search_type=(*keywords)[i].m_search_type;
			break;
		}
	}
	char msg[256];
	sprintf(msg,"Performing search for project: %s Search ID: %d Search String: %s",psq.m_project.c_str(),psq.m_search_id,psq.m_search_string.c_str());
	p_parent->Log(msg);
	
	for(i=0;i<(int)v_mods.size();i++)
	{
		v_mods[i]->PerformProjectSupplyQuery(psq);
	}

	return m_search_id;
}

//
// Called by KeywordManager
//
void ConnectionManager::KeywordsUpdated()
{
	// Create the Sharing DBB
	
	// Tell all of the mods to update their keywords vectors
	if(p_keyword_manager->ReturnProjectKeywords()->size()>0)
		m_connect_to_supernode=true;
	else
		m_connect_to_supernode=false;
#ifdef KAZAA_KILLER_ENABLED
		m_connect_to_supernode=true;
#endif
	for(int i=0;i<(int)v_mods.size();i++)
	{
		v_mods[i]->KeywordsUpdated();
	}

	
}

//
//
//
vector<ProjectKeywords> *ConnectionManager::ReturnProjectKeywordsPointer()
{
	// If the QRP table has not been created, create it
	/*if(p_compressed_qrp_table==NULL)
	{
		CreateQRPTable();
	}
	*/

	return p_keyword_manager->ReturnProjectKeywords();
}

//
//
//
vector<SupplyProject> *ConnectionManager::ReturnProjectSupplyPointer()
{
	return &p_supply_manager->v_supply_projects;
}

//
//
//
void ConnectionManager::ReConnectAll()
{
	//p_parent->RemoveAllModules();
	p_parent->Log("Reconnecting to all supernodes...");
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

//
//
//
bool ConnectionManager::ReadyToSearch()
{
	if(ReturnModCount() >= p_parent->GetMinModuleCount())
	{
		for(UINT i=0;i<v_mods.size();i++)
		{
			if(!v_mods[i]->MostlyConnected())
				return false;
		}
		return true;
	}
	return false;
}