// BlubsterSocket.cpp

#include "stdafx.h"
#include "BlubsterSocket.h"
#include "Checksum.h"
#include "BlubsterOpCode.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"
#include "PoisonEntry.h"
#include "PoisonerStatus.h"

//#define HOSTS_LIMIT			5000
//#define HOST_CACHE_LIMIT	5000
#define MAX_TTL				10
#define QUERY_CACHE_SIZE	1000
#define QUERY_HIT_CACHE_SIZE	1000

#define IP_HDRINCL 2	// for ip spoofing

//
//
//
BlubsterSocket::BlubsterSocket()
{
	m_host_cache_counter=0;
	m_ping_timer=29;	// the first time the timer fires, pings will be sent

	// Create the socket
	//	Create(41170,(64<<0)+(60<<8)+(52<<16)+(134<<24));
	Create(41170);

	LoadHostCache();
	ReadMaximumHostsDataFromRegistry();
	srand((unsigned)time(NULL));
	m_last_use_poisoner_index=0;
}

//
//
//
BlubsterSocket::~BlubsterSocket()
{
}
/*
//
//
//
void BlubsterSocket::InitParent(CBlubsterDlg *dlg)
{
p_dlg=dlg;
}
*/
//
//
//
void BlubsterSocket::InitHwnd(HWND hwnd)
{
	m_hwnd=hwnd;
}

//
//
//
void BlubsterSocket::InitParent(ConnectionModule* parent)
{
	p_connection_module = parent;
}

//
// 1 second timer
//
void BlubsterSocket::TimerHasFired()
{
	unsigned int i;

	//debug
	//ProcessQuery(0,"afi but home is nowhere",2,123);

	// Check to see if there are no hosts. If not, get some ips from the host cache
	if(v_hosts.size()==0)
	{
		if(v_host_cache.size()>0)
		{
			for(i=0;i<m_max_host;i++)
			{
				v_hosts.push_back(v_host_cache[rand()%v_host_cache.size()]);
			}
			sort(v_hosts.begin(),v_hosts.end());
		}
		else	// add booboo *&* kludge *&*
		{		
			BlubsterHost host;
			host.m_ip=(216<<0)+(151<<8)+(155<<16)+(87<<24);
			SendPing(host.m_ip,host.m_port,2);
			host.m_ip=(216<<0)+(151<<8)+(155<<16)+(94<<24);
			SendPing(host.m_ip,host.m_port,2);
			return;
		}
	}

	// If we are not connected to at least 10 hosts, get more hosts from our connected hosts
	/*
	unsigned int connected_count=0;
	for(i=0;i<v_hosts.size();i++)
	{
	if(v_hosts[i].m_connected>0)
	{
	connected_count++;
	}
	}
	*/

	// Check to see if the ping timer is at 30 seconds
	m_ping_timer++;
	if(m_ping_timer==30)
	{
		m_ping_timer=0;
		/*
		// Send Pings to connected hosts (keep-alives)
		SendPings(0);

		// Send a query to say hi to everyone
		PerformQuery("salt water enema");
		*/
		// See if we need more hosts
		/*
		if(connected_count<m_max_host)
		{
		SendPings(MAX_TTL);	// was 2
		}
		else
		{
		SendPings(MAX_TTL);	// was 0
		}
		*/
		SendPings(MAX_TTL);
	}

	// Write out the host cache to file every 60 seconds
	if((++m_host_cache_counter)>=60)
	{
		m_host_cache_counter=0;

		CFile file;
		if(file.Open("host_cache.dat",CFile::modeCreate|CFile::typeBinary|CFile::modeWrite))
		{
			for(i=0;i<v_host_cache.size();i++)
			{
				unsigned int ip_port[2];
				ip_port[0]=v_host_cache[i].m_ip;
				ip_port[1]=v_host_cache[i].m_port;

				file.Write(ip_port,sizeof(ip_port));
			}
			file.Close();
		}
	}
}

//
//
//
void BlubsterSocket::OnReceive(int error_code)
{
	//	unsigned int i;

	char buf[4096];
	memset(buf,0,sizeof(buf));

	unsigned int ip=0;
	unsigned short int port=0;
	unsigned int num_read=0;
	int ret=ReceiveFrom(buf,sizeof(buf),&ip,&port,&num_read);

	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
		return; 
	}

	BlubsterHeader *hdr=(BlubsterHeader *)buf;

	//nah...don't check the checksum for saving some juice
	// Check the checksum 
	/*
	if(Checksum::Calc((unsigned char *)&buf[2],num_read-2) != hdr->Checksum())
	{	
	char msg[1024];
	sprintf(msg,"[%u.%u.%u.%u - *** Bad Message ***]\r\n",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
	Log(msg);

	return;
	}
	*/

	switch(hdr->Op())
	{
	case BlubsterOpCode::Ping:
		{
			/*
			char msg[1024];
			sprintf(msg,"[ %u.%u.%u.%u - Ping - TTL=%u ]\r\n",(src_ip>>0)&0xFF,(src_ip>>8)&0xFF,(src_ip>>16)&0xFF,(src_ip>>24)&0xFF,hdr->TTL());
			Log(msg);
			*/

			// Send pong in reply
			SendPong(ip);

			break;
		}
	case BlubsterOpCode::Pong:
		{
			//using too much CPU, will optimize later
			//IncrementCounters(0,1,0,0,0,0);

			char *ptr=&buf[20];

			unsigned int num_shared_files=0;
			unsigned int num_kb_shared=0;
			char *nickname=NULL;
			unsigned int num_connections=0;
			char *client_version=NULL;
			char *velocity=NULL;
			char *client_name=NULL;

			bool error=false;
			while(*ptr!=NULL)
			{
				if(memcmp(ptr,"SF",2)==0)	// # of shared files
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					ptr+=len;
					/*
					// pointing at data
					num_shared_files=0;
					num_shared_files+=((unsigned char)*(ptr++))<<16;
					num_shared_files+=((unsigned char)*(ptr++))<<8;
					num_shared_files+=((unsigned char)*(ptr++))<<0;
					*/
				}
				else if(memcmp(ptr,"SK",2)==0)	// # of KB shared
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					ptr+=len;
					/*
					// pointing at data
					num_kb_shared=0;
					num_kb_shared+=((unsigned char)*(ptr++))<<32;
					num_kb_shared+=((unsigned char)*(ptr++))<<24;
					num_kb_shared+=((unsigned char)*(ptr++))<<16;
					num_kb_shared+=((unsigned char)*(ptr++))<<8;
					num_kb_shared+=((unsigned char)*(ptr++))<<0;
					*/
				}
				else if(memcmp(ptr,"NI",2)==0)	// nickname
				{
					if(nickname!=NULL)
					{
						error=true;
						break;
					}

					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					ptr+=len;
					/*
					// pointing at data
					nickname=new char[len+1];
					memset(nickname,0,len+1);
					memcpy(nickname,ptr,len);
					ptr+=len;
					*/
				}
				else if(memcmp(ptr,"NC",2)==0)	// num connections
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					num_connections=0;
					num_connections+=((unsigned char)*(ptr++))<<8;
					num_connections+=((unsigned char)*(ptr++))<<0;
				}
				else if(memcmp(ptr,"CN",2)==0) //client name ?
				{
					if(client_name!=NULL)
					{
						error=true;
						break;
					}

					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					client_name=new char[len+1];
					memset(client_name,0,len+1);
					memcpy(client_name,ptr,len);
					ptr+=len;
				}
				else if(memcmp(ptr,"CV",2)==0)	// client version
				{
					if(client_version!=NULL)
					{
						error=true;
						break;
					}

					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					client_version=new char[len+1];
					memset(client_version,0,len+1);
					memcpy(client_version,ptr,len);
					ptr+=len;
				}
				else if(memcmp(ptr,"VL",2)==0)
				{
					if(velocity!=NULL)
					{
						error=true;
						break;
					}

					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					ptr+=len;
					/*
					// pointing at data
					velocity=new char[len+1];
					memset(velocity,0,len+1);
					memcpy(velocity,ptr,len);
					ptr+=len;
					*/
				}
				else
				{
					error=true;
					break;
				}
			}

			if(error==false)
			{
				/*
				// If the nickname is not specified, it is Anonymous
				if(nickname==NULL)
				{
				nickname=new char[strlen("Anonymous")+1];
				strcpy(nickname,"Anonymous");
				}

				// Log it
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - Pong - TTL=%u ] from %s ... %u Files ... %u KB ... %u Connections ... %s ... %s\r\n",
				(src_ip>>0)&0xFF,(src_ip>>8)&0xFF,(src_ip>>16)&0xFF,(src_ip>>24)&0xFF,hdr->TTL(),
				nickname,num_shared_files,num_kb_shared,num_connections,client_version,velocity);
				Log(msg);
				*/
			}
			else
			{
				/*
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - ERROR in Pong ] - Unknown Message \"%s\"\r\n",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,
				(ip>>24)&0xFF, ptr);
				Log(msg);
				*/
				IncrementCounters(0,0,0,0,0,1);
			}

			// If there was no error, then see if we have this ip in our hosts vector. If so, we are now "connected". If not, add it as "connected"
			if(error==false)
			{
				BlubsterHost host;
				host.m_ip = ip;
				bool found = binary_search(v_hosts.begin(), v_hosts.end(), host);


				if(!found)
				{
					CString vendor = client_name;
					vendor += " ";
					vendor += client_version;
					AddVendor(vendor);
					AddHost(ip,41170,num_connections,vendor);
				}
				else
				{
					vector<BlubsterHost>::iterator result = NULL;
					result = find(v_hosts.begin(),v_hosts.end(),host);
					if(result != v_hosts.end() && result != NULL)
					{
						if(result->m_connected_before == false)
						{
							CString vendor = client_name;
							vendor += " ";
							vendor += client_version;
							AddVendor(vendor);
							result->SetVendor(vendor);
							result->m_connected_before = true;
						}
						result->m_connected=5;
						result->m_num_connections=num_connections;
					}
				}
				/*
				for(i=0;i<v_hosts.size();i++)
				{
				if(v_hosts[i].m_ip==ip)
				{
				found=true;
				if(v_hosts[i].m_connected_before == false) //haven't seen this host b4
				{
				CString vendor = client_name;
				vendor += " ";
				vendor += client_version;
				AddVendor(vendor);
				v_hosts[i].SetVendor(vendor);
				v_hosts[i].m_connected_before = true;
				}
				v_hosts[i].m_connected=5;
				v_hosts[i].m_num_connections=num_connections;
				break;
				}
				}
				*/
			}
			else //remove this host from both v_host and v_host_cache
			{
				/*
				vector<BlubsterHost>::iterator iter = v_hosts.begin();
				while(iter != v_hosts.end())
				{
				if(iter->m_ip == ip)
				{
				if( strlen(iter->m_vendor) > 0) //remvoe the old vendor count
				{
				for(UINT j=0; j<v_vendor_counts.size(); j++)
				{
				if(strcmp(iter->m_vendor, v_vendor_counts[j].m_vendor.c_str())==0)
				{
				v_vendor_counts[j].m_count--;
				break;
				}
				}
				}
				v_hosts.erase(iter);
				break;
				}
				iter++;
				}
				iter = v_host_cache.begin();
				while(iter != v_host_cache.end())
				{
				if(iter->m_ip == ip)
				{
				v_host_cache.erase(iter);
				break;
				}
				iter++;
				}
				*/
				BlubsterHost host;
				host.m_ip = ip;
				bool found = binary_search(v_hosts.begin(), v_hosts.end(), host);

				if(found)
				{
					vector<BlubsterHost>::iterator result = NULL;
					result = find(v_hosts.begin(),v_hosts.end(),host);
					if(result != v_hosts.end() && result != NULL)
					{
						if( strlen(result->m_vendor) > 0) //remvoe the old vendor count
						{
							for(UINT j=0; j<v_vendor_counts.size(); j++)
							{
								if(strcmp(result->m_vendor, v_vendor_counts[j].m_vendor.c_str())==0)
								{
									v_vendor_counts[j].m_count--;
									break;
								}
							}
						}
						v_hosts.erase(result);		
					}
				}
			}

			// Free memory
			if(nickname!=NULL)
			{
				delete [] nickname;
			}
			if(client_version!=NULL)
			{
				delete [] client_version;
			}
			if(client_name!=NULL)
			{
				delete [] client_name;
			}
			if(velocity!=NULL)
			{
				delete [] velocity;
			}

			break;
		}
	case BlubsterOpCode::Query:
		{
			IncrementCounters(0,1,0,0,0,0);

			// First check to see if we have seen this query recently. If so, then return
			int j;
			unsigned int counter=hdr->Counter();
			unsigned int src_ip=hdr->SourceIP();

			QueryID id;
			if(src_ip==0)	// if the source IP in the blubster header is 0, get the ip from the packet
			{
				id.m_ip=ip;
			}
			else
			{
				id.m_ip=src_ip;
			}
			// check to see if the query is from one of our DC
			if(IsQueryFromPioletDC(id.m_ip))
				return;

			id.m_count=counter;
			if(v_query_cache.size()>0)
			{
				for(j=(int)v_query_cache.size()-1;j>=0;j--)	// start at end
				{
					if((v_query_cache[j].m_ip==src_ip) && (v_query_cache[j].m_count==counter))
					{
						return;
					}
				}
			}

			// We haven't seen this one before, so add it to the cache and limit the size
			v_query_cache.push_back(id);
			while(v_query_cache.size()>QUERY_CACHE_SIZE)
			{
				v_query_cache.erase(v_query_cache.begin());
			}

			char *ptr=&buf[20];

			ptr+=2;	// skip 2-char header
			unsigned char is_ascii=(unsigned char)(*(ptr++));
			unsigned char len=(unsigned char)*(ptr++);

			char *filename=NULL;

			// pointing at data
			filename=new char[len+1];
			memset(filename,0,len+1);
			memcpy(filename,ptr,len);
			ptr+=len;

			// forward the query to the other dudes I am connected to if the TTL-1 is > 0
			if((hdr->TTL()-1)==1)
			{
				IncrementCounters(1,0,0,0,0,0);

				// If this query was from one of my connections, forward the query with the host's ip
				if(hdr->SourceIP()==0)
				{
					ForwardQuery(ip,hdr->Counter(),hdr->TTL()-1,filename);
				}
				else
				{
					ForwardQuery(hdr->SourceIP(),hdr->Counter(),hdr->TTL()-1,filename);
				}
			}


			BlubsterHost host;
			host.m_ip = ip;
			bool found_ip = binary_search(v_hosts.begin(), v_hosts.end(), host);
			host.m_ip = hdr->SourceIP();
			bool found_src_ip =  binary_search(v_hosts.begin(), v_hosts.end(), host);


			if(found_ip || found_src_ip)
			{
				vector<BlubsterHost>::iterator result = NULL;
				result = find(v_hosts.begin(),v_hosts.end(),host);
				if(result != v_hosts.end() && result != NULL)
				{
					result->m_connected=5;
				}
			}

			if(!found_ip)
			{
				//				AddHost(ip);
				SendPing(ip,41170,2);
			}
			if(!found_src_ip)
			{
				//				AddHost(src_ip);
				SendPing(hdr->SourceIP(),41170,2);
			}


			//see if the query matches the project keywords
			if(filename!=NULL)
				ProcessQuery(id.m_ip, filename, hdr->TTL(), counter);

			delete [] filename;

			break;
		}
	case BlubsterOpCode::QueryHit:
		{
			//spoofer doesn't care QH
			break;
		}
	default:
		{
			IncrementCounters(0,0,0,0,1,0);
			/*
			char msg[1024];
			sprintf(msg,"[ %u.%u.%u.%u - ERROR Unknown Op Code : %d]\r\n",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF, hdr->Op());
			Log(msg);
			break;
			*/
		}
	}
}

//
//
//
void BlubsterSocket::SendPing(unsigned int ip,unsigned short int port,unsigned char ttl)
{
	char buf_len=sizeof(BlubsterHeader);
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(p_connection_module->GetHeaderCounter());
	hdr->DestIP(ip);
	hdr->TTL(ttl);
	hdr->Op(BlubsterOpCode::Ping);
	hdr->DataLength(buf_len-sizeof(BlubsterHeader));

	// Calc checksum
	hdr->Checksum(Checksum::Calc((unsigned char *)&buf[2],buf_len-2));

	unsigned int num_sent=0;
	int ret=SendTo(buf,buf_len,ip,port,&num_sent);

	delete [] buf;	// free memory

	// Check for error
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}

//
//
//
void BlubsterSocket::SendPong(unsigned int ip,unsigned short int port,unsigned char ttl)
{
	//char pong_data[]={'S','F',0,3,0,0,0,'S','K',0,5,0,0,0,0,0,'N','C',0,2,0,10,'C','V',1,5,'2','.','0','.','0','V','L',1,2,'T','1'};
	char pong_data[]={'S','F',0,3,0,0x0a,0x1a,'S','K',0,5,0,0,0x10,0x2b,0x02,'N','I',1,9,'A','n','o','n','y','m','o','u','s','N','C',0,2,0,64,'C','N',1,6,'P','i','o','l','e','t','C','V',1,5,'1','.','0','.','5','V','L',1,2,'T','3'};

	if(v_hosts.size()>255)
	{
		pong_data[34]=(unsigned char)255;
	}
	else
	{
		pong_data[34]=(unsigned char)v_hosts.size();
	}

	unsigned int buf_len=sizeof(BlubsterHeader)+sizeof(pong_data);
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(p_connection_module->GetHeaderCounter());
	hdr->DestIP(ip);
	hdr->TTL(ttl);
	hdr->Op(BlubsterOpCode::Pong);
	hdr->DataLength(buf_len-sizeof(BlubsterHeader));

	memcpy(&buf[sizeof(BlubsterHeader)],pong_data,sizeof(pong_data));

	// Calc checksum
	hdr->Checksum(Checksum::Calc((unsigned char *)&buf[2],buf_len-2));

	unsigned int num_sent=0;
	int ret=SendTo(buf,buf_len,ip,port,&num_sent);

	delete [] buf;	// free memory

	// Check for error
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}

//
//
//
void BlubsterSocket::Log(char *buf, COLORREF color, bool bold, bool italic)
{
	/*
	char *ptr=new char[strlen(buf)+1];
	strcpy(ptr,buf);
	::PostMessage(m_hwnd,WM_STATUS_READY,(WPARAM)ptr,(LPARAM)0);
	*/
	p_connection_module->Log(buf, color, bold, italic);
}

//
//
//
void BlubsterSocket::IncrementCounters(unsigned int fordwarded_query,unsigned int query,unsigned int query_matched,unsigned int spoof_sent,unsigned int unknown,unsigned int error_pong)
{
	unsigned int *counts=new unsigned int[6];
	counts[0]=fordwarded_query;
	counts[1]=query;
	counts[2]=query_matched;
	counts[3]=spoof_sent;
	counts[4]=unknown;
	counts[5]=error_pong;
	::PostMessage(m_hwnd,WM_STATUS_READY,(WPARAM)0,(LPARAM)counts);
}

//
//
//
void BlubsterSocket::SendPings(unsigned char ttl)
{
	unsigned int i;

	// Delete all unknown hosts, since they are now not connected
	vector<BlubsterHost> hosts;
	for(i=0;i<v_hosts.size();i++)
	{
		if(v_hosts[i].m_connected>0)
		{
			hosts.push_back(v_hosts[i]);
		}
		else
		{
			if( strlen(v_hosts[i].m_vendor) > 0) //remvoe the old vendor count
			{
				for(UINT j=0; j<v_vendor_counts.size(); j++)
				{
					if(strcmp(v_hosts[i].m_vendor, v_vendor_counts[j].m_vendor.c_str())==0)
					{
						v_vendor_counts[j].m_count--;
						break;
					}
				}
			}
		}
	}
	v_hosts=hosts;
	sort(v_hosts.begin(), v_hosts.end());

	unsigned int connected_count=0;
	unsigned int not_connected_count=0;

	// Send pings to the connected hosts
	//	for(i=0;i<v_hosts.size();i++)
	for(i=0;i<v_hosts.size();i++)
	{
		if(v_hosts[i].m_connected>0)
		{
			connected_count++;
			v_hosts[i].m_connected--;
			SendPing(v_hosts[i].m_ip,v_hosts[i].m_port,ttl);
		}
	}

	char msg[1024];
	sprintf(msg,"*** Sending Pings to %u Connected Hosts ***\r\n",connected_count);
	/*
	for(i=0;i<v_hosts.size();i++)
	{
	sprintf(&msg[strlen(msg)]," %u",v_hosts[i].m_num_connections);
	}
	*/
	Log(msg,0x00008000,true);
}

//
//
//
void BlubsterSocket::ForwardQuery(unsigned int src_ip,unsigned int counter,unsigned char ttl,char *query)
{
	int i;

	unsigned int buf_len=sizeof(BlubsterHeader)+4+strlen(query);
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(counter);	// copy the counter from the original query, don't increment it
	hdr->SourceIP(src_ip);
	hdr->TTL(ttl);
	hdr->Op(BlubsterOpCode::Query);
	hdr->DataLength(buf_len-sizeof(BlubsterHeader));

	char query_header[]={'F','N',1,0};
	query_header[3]=(unsigned char)strlen(query);
	memcpy(&buf[sizeof(BlubsterHeader)],query_header,sizeof(query_header));
	memcpy(&buf[sizeof(BlubsterHeader)+sizeof(query_header)],query,strlen(query));
	/*
	// Send it to one of the connected hosts...picked randomly
	if(v_hosts.size()>0)
	{
	while(1)
	{
	unsigned int rand_index=rand()%((unsigned int)v_hosts.size());
	if(v_hosts[rand_index].m_ip!=src_ip)
	{
	// Set dest ip
	hdr->DestIP(v_hosts[rand_index].m_ip);

	// Calc checksum
	hdr->Checksum(Checksum::Calc((unsigned char *)&buf[2],buf_len-2));

	unsigned int num_sent=0;
	int ret=SendTo(buf,buf_len,v_hosts[rand_index].m_ip,v_hosts[rand_index].m_port,&num_sent);

	// Check for error
	int error=0;
	if(ret==-1)
	{
	error=GetLastError();
	}

	break;
	}
	}
	}
	*/
	for(i=0;i<(int)v_hosts.size();i++)
	{
		if(v_hosts[i].m_ip!=src_ip)
		{
			// Set dest ip
			hdr->DestIP(v_hosts[i].m_ip);

			// Calc checksum
			hdr->Checksum(Checksum::Calc((unsigned char *)&buf[2],buf_len-2));

			unsigned int num_sent=0;
			int ret=SendTo(buf,buf_len,v_hosts[i].m_ip,v_hosts[i].m_port,&num_sent);

			// Check for error
			int error=0;
			if(ret==-1)
			{
				error=GetLastError();
			}
		}
	}

	delete [] buf;	// free memory
}

bool SortByNumConnections(BlubsterHost &host1, BlubsterHost& host2)
{
	if(host1.m_num_connections<host2.m_num_connections)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
void BlubsterSocket::AddHost(unsigned int ip,unsigned short int port,unsigned int num_connections, const char* vendor)
{
	BlubsterHost host;
	host.m_ip=ip;
	host.m_port=port;
	host.m_connected=5;
	host.m_num_connections=num_connections;
	host.SetVendor(vendor);
	host.m_connected_before = true;

	//Insertion sort
	vector<BlubsterHost>::iterator location;
	location = lower_bound(v_hosts.begin(),v_hosts.end(), host);
	if(location != v_hosts.end())
		v_hosts.insert(location,host);
	else
		v_hosts.push_back(host);

	// Limit to hosts to HOSTS_LIMIT if it is twice the size of HOSTS_LIMIT
	if(v_hosts.size()>(2*m_max_host))
	{
		sort(v_hosts.begin(),v_hosts.end(),SortByNumConnections);	// smaller num connections at the beginning

		while(v_hosts.size()>m_max_host)
		{
			if( strlen(v_hosts.begin()->m_vendor) > 0) //remvoe the old vendor count
			{
				for(UINT j=0; j<v_vendor_counts.size(); j++)
				{
					if(strcmp(v_hosts.begin()->m_vendor, v_vendor_counts[j].m_vendor.c_str())==0)
					{
						v_vendor_counts[j].m_count--;
						break;
					}
				}
			}
			v_hosts.erase(v_hosts.begin());
		}
		sort(v_hosts.begin(), v_hosts.end());
	}

	host.m_connected=1;
	host.ClearVendor();
	host.m_connected_before = false;
	v_host_cache.push_back(host);

	// Limit host cache to HOST_CACHE_LIMIT
	while(v_host_cache.size()>m_max_host_cache)
	{
		v_host_cache.erase(v_host_cache.begin());
	}
}

//
//
//
void BlubsterSocket::LoadHostCache()
{
	unsigned int ip_port[2];
	BlubsterHost host;

	CFile file;
	if(file.Open("host_cache.dat",CFile::typeBinary|CFile::modeRead))
	{
		while(file.Read(ip_port,sizeof(ip_port)))
		{
			host.m_ip=ip_port[0];
			//			host.m_port=ip_port[1];
			host.m_port=41170;
			host.m_connected=1;
			host.m_num_connections=0;

			v_host_cache.push_back(host);
		}
		file.Close();
	}
}

//
//
//
void BlubsterSocket::ProcessQuery(int ip, string query, unsigned char ttl, UINT counter)
{
	//ignore the query if we are assigning the project supply vector to the newest one (causing the crash otherwise)
	if(p_connection_module->IsProjectSupplyBeingUpdated())
		return;

	// First make sure that the query is a valid null-terminated string
	/*
	if(IsStringValid(query.c_str())==false)
	{
	return;
	}
	*/

	UINT i,j,k;

	// Extract the keywords from the query
	vector<string> keywords;
	ExtractKeywordsFromQuery(&keywords,query.c_str());

	// Check all of the keywords and exact keywords of all of the projects

	for(i=0;i<v_keywords.size();i++)
	{
		ProjectKeywords *pk=&v_keywords[i];

		unsigned int weight=0;

		// Check all of the keywords for this project
		for(j=0;j<pk->m_query_keywords.v_keywords.size();j++)
		{
			// Against all of the keywords for this query
			for(k=0;k<keywords.size();k++)
			{
				if(strstr(keywords[k].c_str(),pk->m_query_keywords.v_keywords[j].keyword.c_str())!=0)
				{
					weight+=pk->m_query_keywords.v_keywords[j].weight;
				}
			}
		}

		// Check all of the exact keywords for this project
		for(j=0;j<pk->m_query_keywords.v_exact_keywords.size();j++)
		{
			// Against all of the keywords for this query
			for(k=0;k<keywords.size();k++)
			{
				if(strcmp(keywords[k].c_str(),pk->m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
				{
					weight+=pk->m_query_keywords.v_exact_keywords[j].weight;
				}
			}
		}

		// If the weight is >= 100, then this is a query match for the current project.  Check with the kill words, just in case
		if(weight>=100)
		{
			for(j=0;j<keywords.size();j++)
			{
				// See if any of the project killwords are within the query's keywords
				bool found=false;
				for(k=0;k<pk->m_query_keywords.v_killwords.size();k++)
				{
					if(strstr(keywords[j].c_str(),pk->m_query_keywords.v_killwords[k].keyword.c_str())!=0)
					{
						found=true;
						weight=0;
						break;
					}
				}

				// If it didn't match a project killword, see if any of the project exact killwords are equal to the query's keywords
				if(!found)
				{
					for(k=0;k<pk->m_query_keywords.v_exact_killwords.size();k++)
					{
						if(strcmp(keywords[j].c_str(),pk->m_query_keywords.v_exact_killwords[k].keyword.c_str())==0)
						{
							weight=0;
							break;
						}
					}
				}

				// If the query contained a killword, the weight has been reset to 0...so move on to the next project
				if(weight==0)
				{
					break;
				}
			}
		}

		// If the weight is still >= 100, then this is definitely a match
		if(weight>=100)
		{
			IncrementCounters(0,0,1,0,0,0); //query matched
			// Find the appropriate track (if any) this query is looking for
			char *lquery=new char[strlen(query.c_str())+1];
			strcpy(lquery,query.c_str());
			strlwr(lquery);
			int track=0;
			for(j=0;j<pk->m_supply_keywords.v_keywords.size();j++)
			{
				bool found=true;
				for(k=0;k<pk->m_supply_keywords.v_keywords[j].v_keywords.size();k++)
				{
					if(strstr(lquery,pk->m_supply_keywords.v_keywords[j].v_keywords[k])==NULL)
					{
						found=false;
						break;
					}
				}

				if(found)
				{
					//					track=j+1;	// +1 b/c the track number is 1+keyword index, and track=0 is it didn't match any track's keywords WRONG!!!
					track=pk->m_supply_keywords.v_keywords[j].m_track;
					break;
				}
			}
			delete [] lquery;

			//try to search for the next project if track = 0 until search string is different
			if( (track == 0) && (i+1 < v_keywords.size()) )
			{
				bool more_project = true;
				bool found_next_match_project = false;
				int index = 1;
				int this_track=0;
				while(more_project)
				{
					if( (i+index) < v_keywords.size() )
					{
						if(strcmp(v_keywords[i+index].m_supply_keywords.m_search_string.c_str(), 
							v_keywords[i].m_supply_keywords.m_search_string.c_str())!=NULL)
							break;
						else //same search string project, needs to find out the track number
						{
							char *this_lquery=new char[strlen(query.c_str())+1];
							strcpy(this_lquery,query.c_str());
							strlwr(this_lquery);
							this_track=0;
							for(j=0;j<v_keywords[i+index].m_supply_keywords.v_keywords.size();j++)
							{
								bool found_it=true;
								for(k=0;k<v_keywords[i+index].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
								{
									if(strstr(this_lquery,v_keywords[i+index].m_supply_keywords.v_keywords[j].v_keywords[k])==NULL)
									{
										found_it=false;
										break;
									}
								}

								if(found_it)
								{
									//					track=j+1;	// +1 b/c the track number is 1+keyword index, and track=0 is it didn't match any track's keywords WRONG!!!
									this_track=v_keywords[i+index].m_supply_keywords.v_keywords[j].m_track;
									break;
								}
							}
							delete [] this_lquery;
							if(this_track > 0)
							{
								found_next_match_project = true;
								break;
							}
						}
						index++;
					}
					else
					{
						break;
					}
				}
				if(found_next_match_project)
				{
					pk = &v_keywords[i+index];
					track = this_track;
					i=i+index;
				}
			}
#ifdef NO_PROCESSING
			if(track==0) //not sending spoofs to artist name only
				return;
#endif


			// If we haven't seen this GUID before, then increment the query counters
			p_connection_module->m_status_data.v_project_status[i].IncrementTrackQueryCount(track);

			ProjectStatus *status=&p_connection_module->m_status_data.v_project_status[i];

			// Save this query to be reported back to the connection manager
			Query qu;
			qu.m_query=query;
			int spoof_sent = 0;

			if((p_connection_module->p_supply) != NULL) //the spoof entries are ready
			{
				if(!p_connection_module->IsProjectSupplyBeingUpdated())
				{
					spoof_sent = Spoof(ip, 41170, counter, status->ProjectName(), track);
				}
				else
					return;
			}

			if(spoof_sent <= 0)
				return;

			qu.m_spoofs_sent=spoof_sent;

			char msg[1024];
			int ip1,ip2,ip3,ip4;
			ip1=(ip>>0)&0xFF;
			ip2=(ip>>8)&0xFF;
			ip3=(ip>>16)&0xFF;
			ip4=(ip>>24)&0xFF;
			/*
			sprintf(msg,"%d SPOOFS sent to ", spoof_sent);
			Log(msg);
			sprintf(msg,"%u.%u.%u.%u", ip1,ip2,ip3,ip4);
			if(ip1==38 && ip2==119 && (ip3==66 || ip3==64))
			Log(msg, 0x000000FF, true);
			else
			Log(msg);
			sprintf(msg," for project %s : track : %d Query: %s\r\n",
			status->ProjectName(),
			track,
			query.c_str());
			Log(msg);
			*/			
			if(spoof_sent >= 0)
			{
				IncrementCounters(0,0,0,spoof_sent,0,0);
				//qu.m_ttl=ttl;
				qu.m_ip=ip;
				qu.m_track=track;					
				qu.m_timestamp = CTime::GetCurrentTime();
				qu.m_project = status->ProjectName();
				status->v_queries.push_back(qu);
			}
			break;
		}
	}
}

//
//
//
int BlubsterSocket::Spoof(unsigned int dest_ip,unsigned short int dest_port,unsigned int counter, string project, int track)
{
	/*
	unsigned int home=(66<<0)+(27<<8)+(208<<16)+(184<<24);
	unsigned int near_home=(66<<0)+(27<<8)+(208<<16)+(185<<24);
	unsigned int booboo=(64<<0)+(60<<8)+(52<<16)+(133<<24);
	unsigned int jay=(64<<0)+(60<<8)+(52<<16)+(135<<24);
	*/

	if(p_connection_module->IsProjectSupplyBeingUpdated())
		return 0;

	vector<SupplyProject>::iterator iter = p_connection_module->p_supply->begin();
	while(iter != p_connection_module->p_supply->end())
	{
		if(strcmp(iter->m_name.c_str(), project.c_str())==0)
			break;
		iter++;
	}
	if(iter == p_connection_module->p_supply->end())
	{
		return -1; //no supply entry found
	}
	SupplyProject supply_project = *iter;

	//testing phase, sending 150 spoofs
	int num_spoofs=0;

	//if(track > 0)
	//	num_spoofs = 150; //supply_project->m_num_spoofs;
	//else
		num_spoofs = 200;


	int first_spoof_entry_index = 0;
	int last_spoof_entry_index = 0;

#ifdef PIOLET_POISON
	int first_poison_entry_index = 0;
	int last_poison_entry_index = 0;
#endif

	if(track > 0)
	{
		FindFirstAndLastSpoofEntryIndex(supply_project.v_spoof_entries, track, first_spoof_entry_index, last_spoof_entry_index);
#ifdef PIOLET_POISON
		FindFirstAndLastSpoofEntryIndex(supply_project.v_poison_entries, track, first_poison_entry_index, last_poison_entry_index);
#endif
	}
	else
	{
		last_spoof_entry_index = (int)supply_project.v_spoof_entries.size() - 1;
#ifdef PIOLET_POISON
		last_poison_entry_index = (int)supply_project.v_poison_entries.size() - 1;
#endif
	}

	int ip1 = dest_ip>>0&0xff;
	int ip2 = dest_ip>>8&0xff;
	int ip3 = dest_ip>>16&0xff;
	int ip4 = dest_ip>>24&0xff;
	bool from_us = false;
	if(ip1==38 && ip2==119 && (ip3==66 || ip3==64))
	{
		from_us = true;
	}

	//Spoofing
	for(int i=0; i<num_spoofs; i++)
	{
		char my_check[32+1];
		memset(my_check,0,sizeof(my_check));
		for(int j=0;j<16;j++)
		{
			sprintf(&my_check[j*2],"%02X",rand()&0xFF);
		}

		if(last_spoof_entry_index - first_spoof_entry_index > 0)
		{
			int random_spoof_entry_index = (rand()%(last_spoof_entry_index - first_spoof_entry_index +1)) + first_spoof_entry_index;
			//	int random_supply_project_index = rand() % (int)p_connection_module->p_supply->size();
			int random_nickname_index  =0;
			if(supply_project.v_spoof_entries.size() > 0)
				random_nickname_index = rand()% (int)supply_project.v_spoof_entries.size();
			string nickname = supply_project.v_spoof_entries[random_nickname_index].Nickname();
			char* username=NULL;
			/*
			if(from_us)
			{
				username=new char[strlen("spoof2")+1];
				strcpy(username,"spoof2");
			}
			else
			{
			*/
				username=new char[nickname.size()+10];
				strcpy(username,nickname.c_str());
				if(stricmp(nickname.c_str(),"anonymous")!= 0)
				{
					ChangeUsername(username);
				}
			//}

			UINT file_length = supply_project.v_spoof_entries[random_spoof_entry_index].m_file_length;
			int delta = (rand() % 100*1024);
			BOOL plus = rand() % 2;
			if(!plus)
				delta *= -1;
			file_length += delta;
			file_length = (file_length/137)*137;

			IPSpoof(v_hosts[rand()%v_hosts.size()].m_ip,
				41170,
				dest_ip,
				dest_port,
				counter,
				supply_project.v_spoof_entries[random_spoof_entry_index],
				my_check,
				username,
				file_length,
				supply_project.v_spoof_entries[random_spoof_entry_index].Filename() );
			//char poisoner_ip_str[16];
			//unsigned int poisoner_ip = GetPoisonerIP(poisoner_ip_str);
			//IPSpoof(poisoner_ip,41170,dest_ip,dest_port,counter,
			//	supply_project.v_spoof_entries[random_spoof_entry_index],my_check,username,file_length,supply_project.v_spoof_entries[random_spoof_entry_index].Filename());
			if(username!=NULL)
				delete [] username;
		}
		else //umm...no spoof entry to use yet...might make up some spoofing later
		{
			num_spoofs = -2;
		}

	}
	char msg[1024];

#ifdef PIOLET_POISON
	//	PoisonEntries poison_entries;
	char poisoner_ip_str[16];
	UINT poisons_count=0;
	if(supply_project.v_poison_entries.size() > 0)
	{
		/*
		UINT total_popularity = 0;
		for(i=first_poison_entry_index; i<=last_poison_entry_index; i++)
		{
		total_popularity += supply_project.v_poison_entries[i].m_popularity;
		}
		*/

		//poisoning
		//PoisonEntry pe;
		UINT num_poisons = last_poison_entry_index-first_poison_entry_index+1;
		/*
		if(track > 0)
		num_poisons = 150;//(last_poison_entry_index - first_poison_entry_index + 1)*3;
		else
		num_poisons = 200;
		*/
		int same_filename_used= p_connection_module->m_rng.GenerateWord32(1,100);
		for(UINT i=0; i<num_poisons; i++)
		{
			/*
			UINT index=first_poison_entry_index;
			UINT popularity_index = 0;
			UINT rand_index = p_connection_module->m_rng.GenerateWord32(0, total_popularity);
			for(index=first_poison_entry_index; index <= last_poison_entry_index; index++)
			{
			popularity_index += supply_project.v_poison_entries[index].m_popularity;
			if(rand_index <= popularity_index)
			break;
			}
			if(index > last_poison_entry_index)
			index = last_poison_entry_index;
			*/

			UINT index = first_poison_entry_index+i;

			int num_poisoners = GetNumPoisoners();

			CString filename;
			GetSpoofFilename(supply_project.v_poison_entries[index], filename);

			bool filename_used=false;
			for(i=first_poison_entry_index;i<index;i++)
			{
				CString filename2;
				GetSpoofFilename(supply_project.v_poison_entries[i], filename2);
				if(stricmp(filename,filename2)==0)
				{
					filename_used=true;
					break;
				}
			}
			if(filename_used)
			{
				//if(rand()%2==0)
				//{
					CString renamed = filename;
					CString appended_name;
					appended_name.Format("(%d).mp3",same_filename_used);
					if(! renamed.Replace(".mp3",appended_name) )
						if(! renamed.Replace(".MP3",appended_name))
							if(! renamed.Replace(".wma", appended_name))
								renamed.Replace(".WMA", appended_name);
					filename = renamed;
				//}
				//else
				//{
				//	CString appended_name;
				//	appended_name.Format("(%d)%s",same_filename_used,supply_project.v_spoof_entries[index].Filename());
				//	filename=appended_name;
				//}
				same_filename_used++;
			}

			for(int j=0;j<num_poisoners;j++)
			{
				unsigned int poisoner_ip = GetPoisonerIP(poisoner_ip_str);

				int random_nickname_index  = rand()% (int)supply_project.v_spoof_entries.size();
				string nickname = supply_project.v_spoof_entries[random_nickname_index].Nickname();
				char* username=NULL;
				/*
				if(from_us)
				{
					username=new char[strlen("poison2")+1];
					strcpy(username,"poison2");
				}
				else
				{
				*/
					username=new char[nickname.size()+10];			
					strcpy(username,nickname.c_str());
					if(stricmp(nickname.c_str(),"anonymous")!= 0)
					{
						ChangeUsername(username);
					}
				//}

				IPSpoof(poisoner_ip,
					41170,
					dest_ip,
					dest_port,
					counter,
					supply_project.v_poison_entries[index],
					supply_project.v_poison_entries[index].MD5(),
					username,
					supply_project.v_poison_entries[index].m_file_length,
					filename );

				if(username!=NULL)
					delete [] username;
				poisons_count++;
			}
		}
	}

	//Send this poison entry to the poisoner
	//p_connection_module->SendPoisonEntry(poisoner_ip_str, poison_entries);

	sprintf(msg, "%d POISONS sent to", poisons_count);
	Log(msg);
	sprintf(msg," %u.%u.%u.%u %d entries ", ip1,ip2,ip3,ip4,last_poison_entry_index-first_poison_entry_index+1);
	if(!from_us)
		Log(msg);
	else
		Log(msg, 0x000000FF, true);
	sprintf(msg,"for project %s : track : %d]\r\n",
		project.c_str(),track);
	Log(msg);
#endif
	sprintf(msg, "%d SPOOFS sent to", num_spoofs);
	Log(msg);
	sprintf(msg," %u.%u.%u.%u %d entries ", ip1,ip2,ip3,ip4,last_spoof_entry_index-first_spoof_entry_index+1);
	if(!from_us)
		Log(msg);
	else
		Log(msg, 0x000000FF, true);
	sprintf(msg,"for project %s : track : %d]\r\n",
		project.c_str(),track);
	Log(msg);

	return num_spoofs;
}

//
//
//
void BlubsterSocket::IPSpoof(unsigned int src_ip,unsigned short int src_port,unsigned int dest_ip,unsigned short int dest_port,
							 unsigned int counter,SupplyEntry& entry,const char* checksum, const char* nickname, UINT file_length,const char* filename)
{
	//
	// Form the Blubster query hit
	//
	unsigned int buf_len=sizeof(BlubsterHeader);
	buf_len+=4+(unsigned int)strlen(filename);	// FN (file name)
	buf_len+=4+4;								// FL (file length)
	buf_len+=4+3;								// BR (bitrate)
	buf_len+=4+4;								// FC (sampling rate)
	buf_len+=4+4;								// ST (stereo)
	buf_len+=4+4;								// SL (song length in seconds)
	buf_len+=4+32;								// CK (md5 checksum)
	buf_len+=4+(unsigned int)strlen(nickname);	// NN (nickname)
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	// Init the header
	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(counter);
	hdr->DestIP(dest_ip);
	hdr->TTL(0);
	hdr->Op(BlubsterOpCode::QueryHit);
	hdr->DataLength(buf_len-sizeof(BlubsterHeader));

	// Copy the data
	char *ptr=&buf[sizeof(BlubsterHeader)];

	// Filename

	char fn[]={'F','N',1,0};
	fn[3]=(unsigned char)strlen(filename);
	memcpy(ptr,fn,sizeof(fn));
	ptr+=sizeof(fn);
	memcpy(ptr,filename,strlen(filename));
	ptr+=strlen(filename);
	/*
	char fn[]={'F','N',1,0};
	fn[3]=(unsigned char)strlen(entry.Filename());
	memcpy(ptr,fn,sizeof(fn));
	ptr+=sizeof(fn);
	memcpy(ptr,entry.Filename(),strlen(entry.Filename()));
	ptr+=strlen(entry.Filename());
	*/

	// File length
	char fl[]={'F','L',0,4};
	memcpy(ptr,fl,sizeof(fl));
	ptr+=sizeof(fl);
	//	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(1024*1024));	// 1 MB
	/*
	int random_file_length = (rand() % 100*1024);
	bool plus = rand() % 2;
	if(!plus)
	random_file_length *= -1;
	random_file_length += entry.m_file_length;
	*/
	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(file_length));
	ptr+=fl[3];

	// Bitrate
	char br[]={'B','R',0,3};
	memcpy(ptr,br,sizeof(br));
	ptr+=sizeof(br);
	// even though there's 3 bytes available, only the bottom 2 will most likely be used
	*((unsigned short int *)&ptr[1])=BlubsterHeader::EndianSwitch((unsigned short int)(entry.m_bit_rate));	// 128 kbps
	ptr+=br[3];

	// Sampling Rate (FC)
	char fc[]={'F','C',0,4};
	memcpy(ptr,fc,sizeof(fc));
	ptr+=sizeof(fc);
	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(entry.m_sampling_rate));	// 44.1 kHz
	ptr+=fc[3];

	// Stereo
	char st[]={'S','T',0,4};
	memcpy(ptr,st,sizeof(st));
	ptr+=sizeof(st);
	//	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(1));	// stereo
	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(entry.m_stereo));	// joint stereo
	ptr+=st[3];

	// Length of song in seconds
	char sl[]={'S','L',0,4};
	memcpy(ptr,sl,sizeof(sl));
	ptr+=sizeof(sl);
	//	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(3*60));	// 3:00
	*((unsigned int *)ptr)=BlubsterHeader::EndianSwitch((unsigned int)(entry.m_song_length));
	ptr+=sl[3];

	// Checksum
	char ck[]={'C','K',1,32};
	memcpy(ptr,ck,sizeof(ck));
	ptr+=sizeof(ck);
	memcpy(ptr,checksum,32);
	ptr+=ck[3];

	// Nickname
	char nn[]={'N','N',1,0};
	nn[3]=(unsigned char)strlen(nickname);
	memcpy(ptr,nn,sizeof(nn));
	ptr+=sizeof(nn);
	memcpy(ptr,nickname,strlen(nickname));
	ptr+=strlen(nickname);

	// Calculate checksum
	hdr->Checksum(Checksum::Calc((unsigned char *)&buf[2],buf_len-2));

	//
	// Form the IP and UDP headers
	//

	// Create a new RAW socket that we will ip spoof with
	SOCKET hSocket=WSASocket(AF_INET,SOCK_RAW,0,NULL,0,0);	

	// Change the socket option to include the IP header in the send data buffer
	BOOL opt=TRUE;
	setsockopt(hSocket,IPPROTO_IP,IP_HDRINCL,(char *)&opt,sizeof(opt));	

	// IP Header
	unsigned char ip[20];
	memset(ip,0,sizeof(ip));
	// ip version
	ip[0]|=0x40;
	// length of header (in 32-bit words)
	ip[0]|=0x05;
	// total length in bytes of data and header (will be set automatically)
	ip[2]=0x00;
	ip[3]=0x00;
	// ID (continually incrementing counter to ID this packet in relation to other packets before and after it)
	ip[4]=rand()%256;
	ip[5]=rand()%256;
	// Fragment offset
	ip[6]=0x00;
	ip[7]=0x00;
	// TTL (hops/seconds ?)
	ip[8]=0x80;
	// Protocol
	ip[9]=0x11;	// UDP
	// Header checksum (will be set automatically)
	ip[10]=0x00;
	ip[11]=0x00;
	// Source address ip1.ip2.ip3.ip4
	ip[12]=(src_ip>>0)&0xFF;	// ip1
	ip[13]=(src_ip>>8)&0xFF;	// ip2
	ip[14]=(src_ip>>16)&0xFF;	// ip3
	ip[15]=(src_ip>>24)&0xFF;	// ip4
	// Dest address	ip1.ip2.ip3.ip4
	ip[16]=(dest_ip>>0)&0xFF;	// ip1
	ip[17]=(dest_ip>>8)&0xFF;	// ip1
	ip[18]=(dest_ip>>16)&0xFF;	// ip1
	ip[19]=(dest_ip>>24)&0xFF;	// ip1

	// UDP Header
	unsigned char udp[8];
	memset(udp,0,sizeof(udp));
	// Source port : 41170
	udp[0]=(src_port>>8)&0xFF;
	udp[1]=(src_port>>0)&0xFF;
	// Dest port : 41170
	udp[2]=(dest_port>>8)&0xFF;
	udp[3]=(dest_port>>0)&0xFF;
	// Length of UDP header and data (8+buf_len)
	udp[4]=((8+buf_len)>>8)&0xFF;
	udp[5]=((8+buf_len)>>0)&0xFF;
	// Checksum (will be calculated below)
	udp[6]=0x00;
	udp[7]=0x00;

	// Calculate the checksum

	// IP pseudo-header
	char pseudo[12];
	memset(pseudo,0,sizeof(pseudo));
	// - Source IP
	memcpy(&pseudo[0],&ip[12],4);
	// - Dest IP
	memcpy(&pseudo[4],&ip[16],4);
	// - Protocol
	pseudo[9]=ip[9];
	// - UDP Length
	memcpy(&pseudo[10],&udp[4],2);

	// Perform the checksum calculations
	unsigned int i;
	unsigned int sum=0;
	for(i=0;i<sizeof(pseudo);i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&pseudo[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}
	for(i=0;i<sizeof(udp);i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&udp[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}

	// Zero-pad the UDP data to make the size even (if necessary)
	unsigned int data_len=buf_len;
	if(data_len%2==1)
	{
		data_len++;
	}
	unsigned char *data=new unsigned char[data_len];
	memset(data,0,data_len);
	memcpy(data,buf,buf_len);
	for(i=0;i<data_len;i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&data[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}

	// Finalize the checksum
	unsigned short int checksum=(unsigned short int)sum;
	checksum^=0xFFFF;

	// Copy the checksum to the UDP header
	udp[6]=(checksum>>8)&0xFF;
	udp[7]=checksum&0xFF;

	//
	// Create the datagram packet to send
	//
	unsigned int pkt_len=sizeof(ip)+sizeof(udp)+data_len;
	char *pkt=new char[pkt_len];
	memcpy(pkt,ip,sizeof(ip));
	memcpy(&pkt[sizeof(ip)],udp,sizeof(udp));
	memcpy(&pkt[sizeof(ip)+sizeof(udp)],data,data_len);

	WSABUF wsabuf;
	wsabuf.buf=pkt;
	wsabuf.len=pkt_len;

	DWORD sent=0;

	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=dest_ip;
	addr.sin_port=htons(dest_port);

	int ret=WSASendTo(hSocket,&wsabuf,1,&sent,0,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL);
	if(ret!=0)
	{
		int error=WSAGetLastError();
	}

	closesocket(hSocket);

	//free memory
	delete [] pkt;
	delete [] data;
	delete [] buf;

	/*
	// Send query hit
	unsigned int num_sent=0;
	ret=SendTo(buf,buf_len,dest_ip,dest_port,&num_sent);

	delete [] pkt;	// free memory

	// Check for error
	error=0;
	if(ret==-1)
	{
	error=GetLastError();
	}
	*/
}

//
//
//
void BlubsterSocket::AddVendor(CString vendor_name)
{
	VendorCount vendor;
	vendor.m_vendor = vendor_name;
	vendor.m_count = 1;

	bool found = false;
	UINT i=0;
	for(i=0;i<v_vendor_counts.size();i++)
	{
		if(v_vendor_counts[i] == vendor)
		{
			found=true;
			v_vendor_counts[i].m_count++;
			break;
		}
	}
	if(!found)
		v_vendor_counts.push_back(vendor);
	sort(v_vendor_counts.begin(),v_vendor_counts.end());
}

//
//
//
bool BlubsterSocket::IsStringValid(const char *ptr)
{
	int i;

	// Make sure that there is a null within 1024 characters of ptr...terminating the string
	for(i=0;i<1024;i++)
	{
		if(*(ptr+i)=='\0')
		{
			return true;
		}
	}

	return false;
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void BlubsterSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')))
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}

//
//
//
void BlubsterSocket::ExtractKeywordsFromQuery(vector<string> *keywords, const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	string cstring_string=cstring;
	strcpy(lbuf,cstring_string.c_str());

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<keywords->size();i++)
		{
			if(strcmp((*keywords)[i].c_str(),ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}

	delete [] lbuf;
}

//
//
//
void BlubsterSocket::UpdateProjectKeywords(vector<ProjectKeywords>& keywords)
{
	v_keywords = keywords;
}

//
//
//
/*
void BlubsterSocket::UpdateSupply(vector<SupplyProject>& supply_projects)
{
v_supply_projects = supply_projects;
}
*/
//
//
//
void BlubsterSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	//PerformQuery(psq.m_search_string.c_str(),psq.m_header_counter);
}

//
//
//
void BlubsterSocket::UpdateHostLimit(UINT max_host)
{
	m_max_host = max_host;
}

//
//
//
void BlubsterSocket::UpdateHostCacheLimit(UINT max_host_cache)
{
	m_max_host_cache = max_host_cache;
}

//
//
//
void BlubsterSocket::ReadMaximumHostsDataFromRegistry()
{
	// If this is the first time running, or the key doesn't exist, return default values
	m_max_host=300;
	m_max_host_cache=5000;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletSpoofer\\Host Limits";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		UINT val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Max Hosts")==0)
			{
				m_max_host=val;
			}
			else if(strcmp(szName,"Max Hosts Cache")==0)
			{
				m_max_host_cache=val;
			}

			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}
	RegCloseKey(hKey);
}

//
//
//
vector<VendorCount>* BlubsterSocket::ReturnVendorCountsPointer()
{
	return &v_vendor_counts;
}

//
//
//
void BlubsterSocket::FindFirstAndLastSpoofEntryIndex(vector<SupplyEntry>& spoof_entries, int track , int& first_spoof_entry_index,int&  last_spoof_entry_index)
{
	bool found_first=false;
	for(int i=0;i<(int)spoof_entries.size();i++)
	{
		if(!found_first && spoof_entries[i].m_track == track)
		{
			first_spoof_entry_index = i;
			found_first = true;
		}
		if(found_first && spoof_entries[i].m_track != track)
		{
			last_spoof_entry_index = i-1;
			return;
		}
	}
	if(found_first && last_spoof_entry_index == 0)
		last_spoof_entry_index = (int)spoof_entries.size()-1;
}

//
//
//
bool BlubsterSocket::IsQueryFromPioletDC(int ip)
{
	char ip_str[16];
	sprintf(ip_str, "%u.%u.%u.%u", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
	for(UINT i=0; i<p_connection_module->v_piolet_dcs.size(); i++)
	{
		if(p_connection_module->v_piolet_dcs[i].CompareNoCase(ip_str)==0)
			return true;
	}
	return false;
}

//
//
//
UINT BlubsterSocket::GetPoisonerIP(char* ip_str)
{
	UINT poisoner_ip=0;
	vector<PoisonerStatus>* poisoners = p_connection_module->GetPoisoners();
	if(m_last_use_poisoner_index >= (int)poisoners->size())
		m_last_use_poisoner_index = 0;
	if(poisoners->size() > 0)
	{
		poisoner_ip = (*poisoners)[m_last_use_poisoner_index].m_ip;
		strcpy(ip_str, (*poisoners)[m_last_use_poisoner_index].m_ip_str);
	}
	m_last_use_poisoner_index++;
	return poisoner_ip;
}

//
//
//
void BlubsterSocket::GetSpoofFilename(SupplyEntry& se, CString& filename)
{
	filename = se.Filename();
	int index =filename.ReverseFind('\\');
	index++;
	filename.Delete(index,filename.GetLength()-index);
	filename += se.MD5();
	filename += "\\";

	char filesize[16];
	sprintf(filesize,"%u",se.m_file_length);

	filename += filesize;
	filename += "\\";
	char* real_filename = strrchr(se.Filename(),'\\');
	real_filename++;

	filename += real_filename;
}

//
//
//
//
//
//
void BlubsterSocket::ChangeUsername(char * username)
{
	CString string = username;
	char* tempuser = new char[strlen(username)+10];
	int len;
	int num;
	int randnum;
	char numchar[1+1];

	//Remove all the numbers
	string.Remove('0');
	string.Remove('1');
	string.Remove('2');
	string.Remove('3');
	string.Remove('4');
	string.Remove('5');
	string.Remove('6');
	string.Remove('7');
	string.Remove('8');
	string.Remove('9');

	len = string.GetLength();
	num = (len%10);
	randnum = rand()%4;

	strcpy(tempuser,string);
	_itoa(num,numchar,10);
	strcat(tempuser,numchar);
	for (int i = 0;i<randnum;i++)
	{
		num = rand()%10;
		_itoa(num,numchar,10);
		strcat(tempuser,numchar);
	}

	strcpy(username,tempuser);
	delete [] tempuser;
}

//
//
//
int BlubsterSocket::GetNumPoisoners()
{
	return p_connection_module->GetNumPoisoners();
}