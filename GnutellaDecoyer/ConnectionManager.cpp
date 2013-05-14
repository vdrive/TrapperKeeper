#include "StdAfx.h"
#include "ConnectionManager.h"
#include "ConnectionModule.h"
#include "GnutellaDecoyerDll.h"
#include "QRP.h"
#include "CompressionModule.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "VendorCount.h"

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
	m_vendor_counts_enabled=false;
#ifdef BEARSHARE_ONLY
	ReadInBearShareIPs();
#endif
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
	//m_db_interface.CloseConnection();
}

//
//
//
void ConnectionManager::AddModule()
{
	
	// Create a new module
	ConnectionModule *mod=new ConnectionModule;
	mod->InitParent(this,(UINT)v_mods.size()+1);	// init parent pointer and module number
	//mod->NewShareFileList(v_share_files);
	v_mods.push_back(mod);
	
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
			if(v_high_priority_host_cache.size() > 0)
			{
				hosts.push_back(v_high_priority_host_cache[0]);
				v_high_priority_host_cache.erase(v_high_priority_host_cache.begin());
			}
			else
			{
				hosts.push_back(v_host_cache[0]);
				v_host_cache.erase(v_host_cache.begin());
			}
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

	char msg[128];
	sprintf(msg,"%u mods : %u sockets : %u hosts cached : %u web cache urls",v_mods.size(),v_mods.size() * 60,v_host_cache.size(),m_web_cache.NumURLs());
	p_parent->ShowSocketStatus(msg);

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
//
//
void ConnectionManager::CreateQRPTable()
{
	// Doing it like Morpheus 1.9.1.0 does it
	UINT i,j,k;

	byte qrp[65536];
	memset(qrp,0,sizeof(qrp));



	for(i=0;i<v_share_files.size();i++)
	{
		for(j=0;j<v_share_files[i].v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable(v_share_files[i].v_keywords[j].c_str(),qrp);
		}
	}
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
	mod.CompressBuffer(qrp,sizeof(qrp));
	m_compressed_qrp_table_len=mod.ReturnCompressedBufferLength();
	
	p_compressed_qrp_table=new char[m_compressed_qrp_table_len];
	memcpy(p_compressed_qrp_table,mod.ReturnCompressedBuffer(),m_compressed_qrp_table_len);
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
	//check number of connected sockets
	UINT connected=0;
	for(int i=0;i<(int)v_mods.size();i++)
	{
		connected+=v_mods[i]->GetConnectedNum();
	}
	if(!connected)
		ReadInHostCache();
}

//
//
//
void ConnectionManager::WriteOutHostCache()
{
	UINT i;

	// Open the hosts.dat file for writing...if the open fails, then who cares
	CFile file;
	MakeSureDirectoryPathExists("Host Cache\\");
	if(file.Open("Host Cache\\hosts.dat",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
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
	if(file.Open("Host Cache\\hosts.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==TRUE)
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
	vector<GnutellaHost> filtered_host_cache;
	if(file.Open("C:\\syncher\\rcv\\GnutellaHostCache\\hosts.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==TRUE)
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
				filtered_host_cache.push_back(host);
		}

		delete [] buf;
	}

	// Add these hosts to the host cache
	for(i=0;i<(UINT)filtered_host_cache.size();i++)
	{
		// Check to see if they are already in the hosts vector
		bool found=false;
		for(UINT j=0;j<(UINT)v_host_cache.size();j++)
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
void ConnectionManager::ConnectToHost(CString host)
{
	vector<GnutellaHost> hosts;
	GnutellaHost gHost;
	gHost.Host(host);
	gHost.Port(6346);
	for(UINT i=0;i<10; i++)
	{
		v_high_priority_host_cache.push_back(gHost);
	}
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

//
//
//
void ConnectionManager::ReadInBearShareIPs()
{
	CStdioFile file;
	if(file.Open("C:\\syncher\\rcv\\Executables\\connect.txt",CFile::modeRead|CFile::typeText|CFile::shareDenyNone)!=0)
	{
		CString read_in;
		bool bear_share_nodes=false;
		while(file.ReadString(read_in))
		{
			if(read_in.Find("[")>=0)
			{
				if(read_in.Find("BearShare")>=0)
					bear_share_nodes=true;
				else
					bear_share_nodes=false;
				continue;
			}
			if(bear_share_nodes)
			{
				int ip1,ip2,ip3,ip4,port;
				GnutellaHost host;
				sscanf(read_in,"%d.%d.%d.%d:%d",&ip1,&ip2,&ip3,&ip4,&port);
				CString ip;
				ip.Format("%d.%d.%d.%d",ip1,ip2,ip3,ip4);
				host.Host(ip);
				host.Port(port);
				v_host_cache.push_back(host);
			}			
		}
		file.Close();
	}
}

//
//
//
void ConnectionManager::NewShareFileList(vector<QueryHitResult>& share_files)
{
	CSingleLock singleLock(&m_share_list_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		v_share_files.clear();
		v_share_files=share_files;
		singleLock.Unlock();
		CreateQRPTable();
		for(UINT i=0;i<v_mods.size();i++)
		{
			v_mods[i]->NewShareFileList(share_files);
		}
	}
	// Create the QRP table with the updated keywords
}

//
//
//
vector<QueryHitResult> ConnectionManager::GetShareFiles()
{
	vector<QueryHitResult> shares;
	CSingleLock singleLock(&m_share_list_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		shares = v_share_files;
		singleLock.Unlock();
	}
	return shares;
}
