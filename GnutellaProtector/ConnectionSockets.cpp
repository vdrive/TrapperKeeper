// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
#include "VendorCount.h"
#include "ConnectionManager.h"
#include "SupplyManager.h"
//
//
//
ConnectionSockets::ConnectionSockets()
{
	m_rng.Reseed(true, 32);
	p_connection_manager = NULL;
	p_supply = NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}
	
	/*
	//eDonkey server IPs for spoofing
	GnutellaHost host;
	host.Host("195.245.244.243");
	host.Port(4661);
	v_edonkey_spoof_ips.push_back(host);
	host.Host("81.23.250.167");
	host.Port(4242);
	v_edonkey_spoof_ips.push_back(host);
	host.Host("62.241.53.2");
	v_edonkey_spoof_ips.push_back(host);
	host.Host("62.241.53.4");
	v_edonkey_spoof_ips.push_back(host);
	host.Host("62.241.53.16");
	v_edonkey_spoof_ips.push_back(host);
	host.Host("194.146.227.8");
	host.Port(4642);
	v_edonkey_spoof_ips.push_back(host);
	*/
}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
}

//
//
//
DWORD ConnectionSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//
void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status)
{
	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].TimerHasFired();
	}

	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_status_data.m_connected_socket_count++;
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}

			// Get the ip of the connection and the connection status of the socket
			m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	status=m_status_data;
	
	m_status_data.ClearCounters();
	
	//return ret;
}

//
//
//
void ConnectionSockets::ConnectToHosts(vector<GnutellaHost> &hosts)
{
	UINT i,j;

	// Find idle sockets to tell these hosts to
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<hosts.size();i++)
	{
		for(j=0;j<(UINT)num_socket_events;j++)
		{
			if(m_sockets[j].IsSocket()==false)
			{
				m_sockets[j].Create();
				m_sockets[j].Connect(hosts[i]);
				break;
			}
		}
	}
}

//
//
//
void ConnectionSockets::ReportHosts(vector<GnutellaHost> &hosts)
{
	int i,j;

	// Add these hosts to the hosts vector
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check to see if they are already in the hosts vector
		bool found=false;
		for(j=(int)m_status_data.v_host_cache.size()-1;j>=0;j--)
		{
			if(hosts[i]==m_status_data.v_host_cache[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			m_status_data.v_host_cache.push_back(hosts[i]);
		}
	}
}

//
//
//
void ConnectionSockets::UpdateKeywords(vector<ProjectKeywords> &keywords)
{
	UINT i,j;
	
	v_keywords=keywords;

	// Make sure that the project status data is still valid since the keywords have been updated...basically the indexes for each project in
	// the keywords vector have to be the same as the indexes for the same project in the project status vector.
	vector<ProjectStatus> old_project_status=m_status_data.v_project_status;
	m_status_data.v_project_status.clear();

	for(i=0;i<v_keywords.size();i++)
	{
		bool found=false;
		for(j=0;j<old_project_status.size();j++)
		{
			if(strcmp(old_project_status[j].ProjectName(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				m_status_data.v_project_status.push_back(old_project_status[j]);
				break;
			}
		}

		// If not found, create a new project status object (this must be a new project, then)
		if(!found)
		{
			ProjectStatus ps;
			ps.ProjectName((char *)keywords[i].m_project_name.c_str());
			m_status_data.v_project_status.push_back(ps);
		}
	}

	// Tell all of the sockets that the keywords have been updated, so that they can re-send their QRP table
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		m_sockets[i].KeywordsUpdated();
	}
}

//
//
//
void ConnectionSockets::UpdateCompressedQRPTable(CompressedQRPTable &table)
{
	m_compressed_qrp_table=table;
}


//
//
//

void ConnectionSockets::SpoofIP(unsigned int ip)
{
	v_spoof_ips.push_back(ip);

	// Limit the vector size to 100 ips
	while(v_spoof_ips.size()>100)
	{
		v_spoof_ips.erase(v_spoof_ips.begin());
	}
}

//
//
//
unsigned int ConnectionSockets::SpoofIP()
{
	if(v_spoof_ips.size())
	{
		// Pick a random ip that we've recently seen
		int rand_index = m_rng.GenerateWord32(0, v_spoof_ips.size()-1);
		if(rand_index >=0)
		{
			unsigned int ip=v_spoof_ips[rand_index];

			// Alter the last two ip digits (ie. 192.168.x.x)
			ip&=0x0000FFFF;
			ip|=(rand()%256)<<16;
			ip|=(rand()%256)<<24;
			return ip;
		}
		else
			return rand();
	}
	return rand();
}

//
//
//
void ConnectionSockets::SpoofEDonkeyIP(GnutellaHost* host)
{
	int index = rand()%v_edonkey_spoof_ips.size();
	host->Host(v_edonkey_spoof_ips[index].Host().c_str());
	host->Port(v_edonkey_spoof_ips[index].Port());
}

//
//
//
vector<VendorCount> *ConnectionSockets::ReturnVendorCounts()
{
	UINT i,j;

	// Create a vector on the heap...the memory will be freed in the main thread
	vector<VendorCount> *vendor_counts=new vector<VendorCount>;

	// Get the vendor name from all of the connected sockets. 
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		if(m_sockets[i].IsConnected())
		{
			string vendor=m_sockets[i].ReturnRemoteVendor();

			// Check the vector to see if this vendor is already in there
			bool found=false;
			for(j=0;j<vendor_counts->size();j++)
			{
				if(strcmp(vendor.c_str(),(*vendor_counts)[j].m_vendor.c_str())==0)
				{
					found=true;
					(*vendor_counts)[j].m_count++;
					break;
				}
			}

			// If not found, then create a new vendor count object
			if(!found)
			{
				VendorCount vc;
				vc.m_vendor=vendor;
				vc.m_count=1;
				vendor_counts->push_back(vc);
			}
		}
	}
	
	return vendor_counts;
}

//
//
//
int ConnectionSockets::ReturnCompressionOnCounts()
{
	UINT i;

	int counts=0;

	// Get the vendor name from all of the connected sockets. 
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		if(m_sockets[i].IsConnected())
		{
			counts+=m_sockets[i].ReturnCompressionOnCounts();
		}
	}
	
	return counts;
}

//
//
//
bool ConnectionSockets::IsProjectSupplyBeingUpdated()
{
	if(p_connection_manager != NULL)
		return p_connection_manager->p_supply_manager->IsSupplyProjectsBeingUpdated();
	else
		return true;
}
