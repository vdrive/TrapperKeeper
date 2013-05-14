// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
#include "VendorCount.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
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
	status = m_status_data;
	
	m_status_data.ClearCounters();
	
//	return ret;
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
void ConnectionSockets::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	int i;

	// Tell all of the sockets to perform this query, unless this is a searcher query, where we only tell 5 ultrapeer sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
/* =>_<=	if(psq.m_is_searcher_query)
	{
		// Find enough connections that have been up at least 30 seconds and have seen enough queries in a period of time
		// If we can't then reduce the number of queries required to pass
		unsigned int req_queries=100;

		int count=0;
		int quota=5;	// number of sockets were are trying to find that have enough queries

		while(1)
		{
			for(i=0;i<num_socket_events;i++)
			{
				if(m_sockets[i].IsConnected())
				{
					// Check to see that this socket has been up at least 30 seconds and has seen enough queries
					if((m_sockets[i].UpTime()>30) && (m_sockets[i].ReturnQueryCount()>req_queries))
					{
						count++;
					}
				}
			}

			if(count<quota)
			{
				// If the req_queries is already 0 and we didn't find anything, then quit trying
				if(req_queries==0)
				{
					m_status_data.v_searcher_socket_query_counts.push_back(-1);
					break;
				}

				req_queries-=1;	// subtract 1 queries
				count=0;
			}
			else
			{
				// Report back the uptime we are using to do this search
				m_status_data.v_searcher_socket_query_counts.push_back(req_queries);
				break;
			}
		}

		// Find the sockets and send the query sockets with
		count=0;
		for(i=0;i<num_socket_events;i++)
		{
			if(m_sockets[i].IsConnected())
			{
				// Check to see that this socket has been connected for at least 30 seconds and has seen enough queries
				if((m_sockets[i].UpTime()>30) && (m_sockets[i].ReturnQueryCount()>req_queries))
				{
					m_sockets[i].PerformProjectSupplyQuery(psq);
					count++;
				}
			}
	
			if(count>=quota)
			{
				break;
			}
		}
	}
	else
	{
=>_<= */
		for(i=0;i<num_socket_events;i++)
		{
			m_sockets[i].PerformProjectSupplyQuery(psq);
		}
// =>_<= 	}

	// Remove any existing queries for this project so that all returns from the new query will not count the returns for any old queries
	// Make sure that if this query is a searcher query, that we only remove searcher queries...same with supply queries
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		//if((strcmp(psq_iter->m_project.c_str(),psq.m_project.c_str())==0) /* =>_<= && (psq_iter->m_is_searcher_query==psq.m_is_searcher_query)*/)
		if(psq_iter->m_project_id == psq.m_project_id &&
			psq_iter->m_track_index == psq.m_track_index)
		{
			v_project_supply_queries.erase(psq_iter);
			break;
		}
		psq_iter++;
	}

	// Save this query to the list of project supply queries that we've done 
	v_project_supply_queries.push_back(psq);
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

	// Make sure all of the project keyword data of the project supply queries are still correct
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		bool found=false;
		for(i=0;i<v_keywords.size();i++)
		{
			if(strcmp(psq_iter->m_project.c_str(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&v_keywords[i],psq_iter->m_track_index);
				psq_iter->m_project_status_index=i;
				break;
			}
		}

		// If not found, then this project no longer exists, so remove the query from the vector
		if(!found)
		{
			v_project_supply_queries.erase(psq_iter);
			//psq_iter=v_project_supply_queries.begin();
		}
		else
		{
			psq_iter++;
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
//void ConnectionSockets::UpdateSupply(vector<SupplyProject> &supply)
//{
//	UINT i;
//
//	// Check to see if it could possibly be an update
//	if(supply.size()==1)
//	{
//		if(strstr(supply[0].m_name.c_str(),UPDATE_STRING)!=NULL)
//		{
//			char *project=new char[supply[0].m_name.size()+1];
//			strcpy(project,supply[0].m_name.c_str());
//			*strstr(project,UPDATE_STRING)='\0';	// remove the update string from the project name
//			supply[0].m_name=project;
//			delete [] project;
//
//			// Find the project in the supply, and update it
//			for(i=0;i<v_supply.size();i++)
//			{
//				if(strcmp(v_supply[i].m_name.c_str(),supply[0].m_name.c_str())==0)
//				{
//					v_supply[i]=supply[0];
//					break;
//				}
//			}
//
//			return;
//		}
//	}
//
//	// Else it's not an update, so just replace the local vector
//	v_supply=supply;
//}


//
//
//
/*
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
	// Pick a random ip that we've recently seen
	unsigned int ip=v_spoof_ips[rand() % v_spoof_ips.size()];

	// Alter the last two ip digits (ie. 192.168.x.x)
	ip&=0x0000FFFF;
	ip|=(rand()%256)<<16;
	ip|=(rand()%256)<<24;

	return ip;
}
*/
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