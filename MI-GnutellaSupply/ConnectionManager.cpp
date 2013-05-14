#include "StdAfx.h"
#include "ConnectionManager.h"
#include "ConnectionModule.h"
#include "GnutellaSupplyDll.h"
#include "KeywordManager.h"
#include "QRP.h"
#include "CompressionModule.h"
#include "SupplyManager.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "VendorCount.h"
#include "DBManager.h"

//
//
//
ConnectionManager::ConnectionManager()
{
	srand((unsigned)time(NULL));

	p_compressed_qrp_table=NULL;
	m_compressed_qrp_table_len=0;

	ReadInHostCache();

	m_web_cache.InitParent(this);

	m_vendor_counts_timer_counter=0;
	//bool connected = false;
	//while(!connected)
	//{
	//	connected = m_db_interface.OpenSupplyConnection("127.0.0.1","onsystems","ebertsux37","gnutella_raw_supply");
	//	Sleep(500);
	//}
	m_vendor_counts_enabled=false;
}

//
//
//
ConnectionManager::~ConnectionManager()
{
	if(p_compressed_qrp_table!=NULL)
	{
		delete [] p_compressed_qrp_table;
		p_compressed_qrp_table=NULL;
		m_compressed_qrp_table_len=0;
	}

	// Free memory (if it wasn't already done by KillModules()
	for(int i=0;i<(int)v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
	//m_db_interface.CloseSupplyConnection();
}

//
//
//
void ConnectionManager::InitKeywordManager(KeywordManager *keyword_manager)
{
	p_keyword_manager=keyword_manager;
}

//
//
//
void ConnectionManager::InitSupplyManager(SupplyManager *supply_manager)
{
	p_supply_manager=supply_manager;
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
void ConnectionManager::PerformProjectSupplyQuery(ProjectSupplyQuery& psq)
{
	UINT i;

	//// Init the project supply query object with a GUID
	//ProjectSupplyQuery psq;
	//
	//if(guid==NULL)
	//{
	//	// Create the GUID and make it look like a new gnutella client
	//	CoCreateGuid(&psq.m_guid);
	//	unsigned char *ptr=(unsigned char *)&psq.m_guid;
	//	ptr[8]=0xFF;
	//	ptr[15]=0x00;;
	//}
	//else
	//{
	//	psq.m_guid=*guid;
	//}

	//p_parent->m_log_window_manager.Log("Connection Manager : ",0x0000C0C0);	// dirty yellow
	//p_parent->m_log_window_manager.Log("Starting Supply Query for project: ");
	//p_parent->m_log_window_manager.Log(psq.m_project.c_str(),0x00FF0000,true);	// bold blue
	//p_parent->m_log_window_manager.Log(" Search string: ",0x008080FF); //fade red
	//p_parent->m_log_window_manager.Log(psq.m_search_string.c_str(),0x00888888,true);	// bold grey
	//p_parent->m_log_window_manager.Log("\n");
	//
	//// Find the project in the keywords vector and give them to the query object to extract
	//for(i=0;i<p_keyword_manager->v_project_keywords.size();i++)
	//{
	//	if(strcmp(p_keyword_manager->v_project_keywords[i].m_project_name.c_str(),project)==0)
	//	{
	//		psq.ExtractProjectKeywordData(&p_keyword_manager->v_project_keywords[i]);
	//		psq.m_project_status_index=i;
	//		psq.m_project_id = p_keyword_manager->v_project_keywords[i].m_id;
	//		if(p_keyword_manager->v_project_keywords[i].m_search_type != ProjectKeywords::search_type::audio)
	//			psq.m_audio_project=false;
	//		break;
	//	}
	//}
	
	for(i=0;i<(int)v_mods.size();i++)
	{
		v_mods[i]->PerformProjectSupplyQuery(psq);
	}

	//return psq.m_guid;
}

//
//
//
void ConnectionManager::ReportStatus(ConnectionModuleStatusData &status)
{
	UINT i;

	// Check to see if a connection in the gwebcache has timed-out
	m_web_cache.CheckForTimeout();

	// Add hosts to the host cache
	ReportHosts(status.v_host_cache);

	// Send hosts for the idle sockets
	vector<GnutellaHost> hosts;
	for(i=0;i<status.m_idle_socket_count;i++)
	{
		// If the host cache is empty, we need to connect to a GWebCache to get some
		if(v_host_cache.size()==0)
		{
			// If the web cache is busy, add the reflectors to the host cache
			if(m_web_cache.ConnectToWebCache()==false)
			{
/*
				GnutellaHost host;
				host.Port(6346);
				string str;

				vector<string> reflectors;

				// Try to open the reflectors file...if it doesn't exist, then use the hard-coded ones
				CStdioFile reflectors_file;
				if(reflectors_file.Open("reflectors.ini",CFile::typeText|CFile::modeRead|CFile::shareDenyWrite))
				{
					CString reflector_cstring;
					while(reflectors_file.ReadString(reflector_cstring))
					{
						reflector_cstring.TrimLeft();
						reflector_cstring.TrimRight();
						string reflector_str=reflector_cstring;

						if(reflector_str.size()>0)
						{
							reflectors.push_back(reflector_str);
						}
					}

					reflectors_file.Close();
				}
				else
				{
					// Add the reflector hosts to a vector
					reflectors.push_back("connect1.gnutellanet.com");
					reflectors.push_back("connect2.gnutellanet.com");
					reflectors.push_back("connect3.gnutellanet.com");
					reflectors.push_back("connect4.gnutellanet.com");
					reflectors.push_back("connect5.gnutellanet.com");
					reflectors.push_back("connect6.gnutellanet.com");
					reflectors.push_back("router2.limewire.com");
					reflectors.push_back("router3.limewire.com");
					reflectors.push_back("router4.limewire.com");
					reflectors.push_back("router6.limewire.com");
					reflectors.push_back("connect1.bearshare.net");
					reflectors.push_back("connect2.bearshare.net");
					reflectors.push_back("connect3.bearshare.net");
					reflectors.push_back("public.bearshare.net");
				}

				// Pick a random reflector
				str=reflectors[rand()%reflectors.size()];

				host.Host((char *)str.c_str());
				hosts.push_back(host);
*/
			}
		}
		else	// we have some hosts
		{		
			// Move the top of the host cache to the hosts vector
			hosts.push_back(v_host_cache[0]);
			v_host_cache.erase(v_host_cache.begin());
		}
	}

	if(hosts.size()>0)
	{
		status.p_mod->ConnectToHosts(hosts);
	}

	// Limit host cache to 5000 hosts
	while(v_host_cache.size()>10000)	// was 10000
	{
		v_host_cache.erase(v_host_cache.begin());
	}

	p_parent->ReportConnectionStatus(status);

	char msg[1024];
	sprintf(msg,"%u mods : %u sockets : %u hosts cached : %u web cache urls",v_mods.size(),v_mods.size() * 60,v_host_cache.size(),m_web_cache.NumURLs());
	p_parent->Log(msg);

	// Send any supply to the supply manager
	/* =>_<=
	for(i=0;i<status.v_project_status.size();i++)
	{
		if(status.v_project_status[i].v_query_hits.size()>0)
		{
			p_supply_manager->QueryHitsReceived((char *)status.v_project_status[i].ProjectName(),status.v_project_status[i].v_query_hits);
		}
	}
	*/
}

//
//
//
unsigned int ConnectionManager::ReturnModCount()
{
	return (UINT)v_mods.size();
}

//
// Called by KeywordManager
//
void ConnectionManager::KeywordsUpdated()
{
	// Create the QRP table with the updated keywords
	CreateQRPTable();

	// Tell all of the mods to update their keywords vectors
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
	if(p_compressed_qrp_table==NULL)
	{
		CreateQRPTable();
	}

	return &p_keyword_manager->v_project_keywords;
}

//
// Called by SupplyManager
//
//void ConnectionManager::ProjectSupplyUpdated(char *project_name)
//{
//	// Tell all of the mods to update their supply
//	for(UINT i=0;i<v_mods.size();i++)
//	{
//		v_mods[i]->ProjectSupplyUpdated(project_name);
//	}
//}

//
//
//
//vector<SupplyProject> *ConnectionManager::ReturnProjectSupplyPointer()
//{
//	return &p_supply_manager->v_supply_projects;
//}
//
//
//
void ConnectionManager::CreateQRPTable()
{
	// Doing it like Morpheus 1.9.1.0 does it

	char qrp[65536];
	memset(qrp,0,sizeof(qrp));

#ifndef NO_DEMAND
	UINT i,j,k;
	// Add all of the project's keywords to the patch
	for(i=0;i<p_keyword_manager->v_project_keywords.size();i++)
	{
		// Query Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_query_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_query_keywords.v_keywords[j].keyword.c_str(),qrp);
		}

		// Exact Query Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),qrp);
		}

		
		// Supply Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_supply_keywords.v_keywords.size();j++)
		{
			for(k=0;k<p_keyword_manager->v_project_keywords[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
			{
				QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_supply_keywords.v_keywords[j].v_keywords[k],qrp);
			}
		}

		// QRP Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_qrp_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_qrp_keywords.v_keywords[j].c_str(),qrp);
		}
	}
	
	QRP::AddUpTo3KeywordsToTable("&",qrp);
	// Add mp3 (and other generic keywords) to the table
	/*
	QRP::AddUpTo3KeywordsToTable("mp3",qrp);
	QRP::AddUpTo3KeywordsToTable("avi",qrp);
	QRP::AddUpTo3KeywordsToTable("mpg",qrp);
	QRP::AddUpTo3KeywordsToTable("mpeg",qrp);
	QRP::AddUpTo3KeywordsToTable("divx",qrp);
	QRP::AddUpTo3KeywordsToTable("new",qrp);
	QRP::AddUpTo3KeywordsToTable("album",qrp);
	QRP::AddUpTo3KeywordsToTable("movie",qrp);
	*/
#endif
	QRP::AddUpTo3KeywordsToTable("http://www.limewire.com/schemas/audio.xsd",qrp);
	QRP::AddUpTo3KeywordsToTable("http://www.limewire.com/schemas/video.xsd",qrp);

	// Compress QRP table
	if(p_compressed_qrp_table!=NULL)
	{
		delete [] p_compressed_qrp_table;
		p_compressed_qrp_table=NULL;
		m_compressed_qrp_table_len=0;
	}

	CompressionModule mod;
	mod.CompressBuffer((unsigned char *)qrp,sizeof(qrp));
	m_compressed_qrp_table_len=mod.ReturnCompressedBufferLength();
	
	p_compressed_qrp_table=new char[m_compressed_qrp_table_len];
	memcpy(p_compressed_qrp_table,mod.ReturnCompressedBuffer(),m_compressed_qrp_table_len);

/*
	int i,j;

	char qrp[65536/2];		// divided by 2 because 4 bits per entry
	memset(qrp,0,sizeof(qrp));

	// Add all of the project's keywords to the patch
	for(i=0;i<p_keyword_manager->v_project_keywords.size();i++)
	{
		// Query Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_query_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_query_keywords.v_keywords[j].keyword.c_str(),qrp);
		}

		// Exact Query Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),qrp);
		}

		// Supply Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_supply_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_supply_keywords.v_keywords[j].keyword.c_str(),qrp);
		}

		// QRP Keywords
		for(j=0;j<p_keyword_manager->v_project_keywords[i].m_qrp_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_keyword_manager->v_project_keywords[i].m_qrp_keywords.v_keywords[j].c_str(),qrp);
		}
	}

	// Add mp3 to the table
	QRP::AddUpTo3KeywordsToTable("mp3",qrp);

	// Compress QRP table
	if(p_compressed_qrp_table!=NULL)
	{
		delete [] p_compressed_qrp_table;
		p_compressed_qrp_table=NULL;
		m_compressed_qrp_table_len=0;
	}

	CompressionModule mod;
	mod.CompressBuffer((unsigned char *)qrp,sizeof(qrp));
	m_compressed_qrp_table_len=mod.ReturnCompressedBufferLength();
	
	p_compressed_qrp_table=new char[m_compressed_qrp_table_len];
	memcpy(p_compressed_qrp_table,mod.ReturnCompressedBuffer(),m_compressed_qrp_table_len);
*/
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
	UINT i;

	// Open the hosts.dat file for writing...if the open fails, then who cares
	CFile file;
	MakeSureDirectoryPathExists("C:\\syncher\\src\\GnutellaHostCache\\");
	if(file.Open("C:\\syncher\\src\\GnutellaHostCache\\hosts.dat",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		return;
	}

	// Create a buffer in memory of the host cache to write out to the hosts.dat file
	unsigned int buf_len=sizeof(unsigned int)+2*sizeof(unsigned int)*(UINT)v_host_cache.size();	// # items, ip and port
	char *buf=new char[buf_len];
	char *ptr=buf;

	// The number of hosts
	*((unsigned int *)ptr)=(UINT)v_host_cache.size();
	ptr+=sizeof(unsigned int);

	// Copy the hosts to the buffer
	for(i=0;i<v_host_cache.size();i++)
	{
		*((unsigned int *)ptr)=v_host_cache[i].IP();
		ptr+=sizeof(unsigned int);
		*((unsigned int *)ptr)=v_host_cache[i].Port();
		ptr+=sizeof(unsigned int);	
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
	if(file.Open("C:\\syncher\\src\\GnutellaHostCache\\hosts.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==TRUE)
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
			GnutellaHost host;
			host.IP(*((unsigned int *)ptr));
			ptr+=sizeof(unsigned int);
			
			host.Port(*((unsigned int *)ptr));
			ptr+=sizeof(unsigned int);
			
			if(FilterHost(host.Host().c_str())==false)
				v_host_cache.push_back(host);
		}

		delete [] buf;
	}
}

//
//
//
void ConnectionManager::ReportHosts(vector<GnutellaHost> &hosts)
{
	int i,j;
	vector<GnutellaHost> filtered_host_cache;
	
	// Filter the host cache with the people we are already connected to
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check with all of the mods with the hosts that they are reporting as being currently connected to
		bool found=false;
		found=FilterHost(hosts[i].Host().c_str());
		if(!found)
		{
			for(j=0;j<(int)v_mods.size();j++)
			{
				if(v_mods[j]->IsConnected(hosts[i].IP()))
				{
					found=true;
					break;
				}
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
		bool found=false;
		for(j=(int)v_host_cache.size()-1;j>=0;j--)
		{
			if(filtered_host_cache[i]==v_host_cache[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			v_host_cache.push_back(filtered_host_cache[i]);
		}
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
		m_vendor_counts_timer_counter = 10;

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
void ConnectionManager::InitDBManager(DBManager *db_manager)
{
	p_db_manager = db_manager;
}

//
//
//
bool ConnectionManager::FilterHost(const char* host)
{
	int ip1,ip2,ip3,ip4;
	sscanf(host, "%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
	if( (ip1==64 && ip2==14) ||
		(ip1==64 && ip2==15) ||
		(ip1==64 && ip2==27) ||
		(ip1==64 && ip2==28) ||
		(ip1==64 && ip2==37) ||
		(ip1==64 && ip2==58) ||
		(ip1==64 && ip2==70) ||
		(ip1==64 && ip2==78) ||
		(ip1==64 && ip2==79) ||
		(ip1==64 && ip2==89) ||
		(ip1==64 && ip2==210) ||
		(ip1==66 && ip2==14) ||
		(ip1==66 && ip2==37) ||
		(ip1==66 && ip2==119) ||
		(ip1==66 && ip2==128) ||
		(ip1==66 && ip2==186) ||
		(ip1==165 && ip2==193) ||
		(ip1==167 && ip2==216) ||
		(ip1==206 && ip2==132) ||
		(ip1==206 && ip2==251) ||
		(ip1==209 && ip2==67) ||
		(ip1==209 && ip2==143) ||
		(ip1==209 && ip2==202) ||
		(ip1==209 && ip2==225) ||
		(ip1==216 && ip2==19) ||
		(ip1==216 && ip2==32) ||
		(ip1==216 && ip2==33) ||
		(ip1==216 && ip2==34) ||
		(ip1==216 && ip2==35) ||
		(ip1==216 && ip2==64) ||
		(ip1==216 && ip2==69) ||
		(ip1==216 && ip2==74) ||
		(ip1==216 && ip2==104) ||
		(ip1==216 && ip2==109) ||
		(ip1==216 && ip2==144) ||
		(ip1==216 && ip2==177) ||
		(ip1==216 && ip2==182) ||
		(ip1==216 && ip2==219) ||
		(ip1==216 && ip2==227) )
	{
		return true;
	}
	else
		return false;
}










