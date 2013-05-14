// BlubsterSocket.cpp

#include "stdafx.h"
#include "BlubsterSocket.h"
#include "Checksum.h"
#include "BlubsterOpCode.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"

//#define HOSTS_LIMIT			5000
//#define HOST_CACHE_LIMIT	5000
#define MAX_TTL				10
#define QUERY_CACHE_SIZE	500
#define QUERY_HIT_CACHE_SIZE	500

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
			host.m_ip=(64<<0)+(60<<8)+(52<<16)+(133<<24);	// booboo
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

	// Check to see if the ping timer is at 60 seconds
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

	// Write out the host cache to file every 30 seconds
	if((++m_host_cache_counter)>=30)
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
			IncrementCounters(1,0,0,0,0,0);

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

					// pointing at data
					num_shared_files=0;
					num_shared_files+=((unsigned char)*(ptr++))<<16;
					num_shared_files+=((unsigned char)*(ptr++))<<8;
					num_shared_files+=((unsigned char)*(ptr++))<<0;
				}
				else if(memcmp(ptr,"SK",2)==0)	// # of KB shared
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					num_kb_shared=0;
					num_kb_shared+=((unsigned char)*(ptr++))<<32;
					num_kb_shared+=((unsigned char)*(ptr++))<<24;
					num_kb_shared+=((unsigned char)*(ptr++))<<16;
					num_kb_shared+=((unsigned char)*(ptr++))<<8;
					num_kb_shared+=((unsigned char)*(ptr++))<<0;
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

					// pointing at data
					nickname=new char[len+1];
					memset(nickname,0,len+1);
					memcpy(nickname,ptr,len);
					ptr+=len;
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

					// pointing at data
					velocity=new char[len+1];
					memset(velocity,0,len+1);
					memcpy(velocity,ptr,len);
					ptr+=len;
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
				bool found=false;
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
				if(!found)
				{
					CString vendor = client_name;
					vendor += " ";
					vendor += client_version;
					AddVendor(vendor);
					AddHost(ip,41170,num_connections,vendor);
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
			IncrementCounters(0,0,1,0,0,0);

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

			// See if we have both the ip and src_ip in our hosts vector. If so, we are now "connected". If not, add it as "connected"
			//bool found_ip=false;
			//bool found_src_ip=false;
			/*
			for(i=0;i<v_hosts.size();i++)
			{
				if(v_hosts[i].m_ip==ip)
				{
					found_ip=true;
					v_hosts[i].m_connected=5;
				}
				if(v_hosts[i].m_ip==hdr->SourceIP())
				{
					found_src_ip=true;
					v_hosts[i].m_connected=5;
				}
				if(found_ip && found_src_ip)
				{
					break;
				}
			}
			*/

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

			// Log it
			/*
			char msg[1024];
			sprintf(msg,"[%u.%u.%u.%u - Query - %u.%u.%u.%u - TTL=%u] %s\r\n",
				(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,
				(hdr->SourceIP()>>0)&0xFF,(hdr->SourceIP()>>8)&0xFF,(hdr->SourceIP()>>16)&0xFF,(hdr->SourceIP()>>24)&0xFF,
				hdr->TTL(),
				filename);
			Log(msg);
			*/
			
			//see if the query matches the project keywords
			if(filename!=NULL)
				ProcessQuery(id.m_ip, filename, hdr->TTL());

/*
//			if(strstr(filename,"EBERT")!=0)
			if(strstr(filename,"EMINEM")!=0)
			{
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - Query - %u.%u.%u.%u - TTL=%u ] %s\r\n",
					(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,
					(src_ip>>0)&0xFF,(src_ip>>8)&0xFF,(src_ip>>16)&0xFF,(src_ip>>24)&0xFF,
					hdr->TTL(),
					filename);
				Log(msg);
			}
*/
/*
			if((strstr(filename,"SALT")!=NULL) && (strstr(filename,"WATER")!=NULL) && (strstr(filename,"ENEMA")!=NULL))
			{
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - Query - %u.%u.%u.%u - TTL=%u ] %s\r\n",
					(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,
					(hdr->SourceIP()>>0)&0xFF,(hdr->SourceIP()>>8)&0xFF,(hdr->SourceIP()>>16)&0xFF,(hdr->SourceIP()>>24)&0xFF,
					hdr->TTL(),
					filename);
				Log(msg);
				
				// Respond to the query
				if(hdr->SourceIP()==0)
				{
//					Spoof(ip,41170,hdr->Counter(),"C:\\Program Files\\Blubster\\My Shared Folder\\salt water enema test.mp3","F31856CDAE15185A87290DB4E66C6C52","jizzay");
					Spoof(ip,41170,hdr->Counter(),"Asses in my face.mp3","11111111111111111111111111111111","fuckassman");
					Spoof(ip,41170,hdr->Counter(),"I like all kinds of pie.mp3","11111111111111111111111111111112","assman1");
					Spoof(ip,41170,hdr->Counter(),"What the fuck is the internet.mp3","11111111111111111111111113111111","2assman");
					Spoof(ip,41170,hdr->Counter(),"Chippies.mp3","11111111111111111111111141111111","assman3");
					Spoof(ip,41170,hdr->Counter(),"Pootietangola.mp3","11111111111111111111111115111111","assman4");
					Spoof(ip,41170,hdr->Counter(),"CHEESE.mp3","11111111111111111111111111611111","assman5");
					Spoof(ip,41170,hdr->Counter(),"watchahca.mp3","11111111111111111111111111171111","assman6");
					Spoof(ip,41170,hdr->Counter(),"goobly.mp3","11111111111111111111111111118111","assman7");
					Spoof(ip,41170,hdr->Counter(),"pink stink VIP.mp3","11111111111111111111111111111911","assman8");
					Spoof(ip,41170,hdr->Counter(),"PUppETeer.mp3","11111111111111111111111111111101","assman9");
				}
				else
				{
//					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"C:\\Program Files\\Blubster\\My Shared Folder\\salt water enema test.mp3","F31856CDAE15185A87290DB4E66C6C52","jizzay");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"Asses in my face.mp3","11111111111111111111111111111111","fuckassman");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"I like all kinds of pie.mp3","11111111111111111111111111111112","assman1");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"What the fuck is the internet.mp3","11111111111111111111111113111111","2assman");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"Chippies.mp3","11111111111111111111111141111111","assman3");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"Pootietangola.mp3","11111111111111111111111115111111","assman4");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"CHEESE.mp3","11111111111111111111111111611111","assman5");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"watchahca.mp3","11111111111111111111111111171111","assman6");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"goobly.mp3","11111111111111111111111111118111","assman7");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"pink stink VIP.mp3","11111111111111111111111111111911","assman8");
					Spoof(hdr->SourceIP(),41170,hdr->Counter(),"PUppETeer.mp3","11111111111111111111111111111101","assman9");
				}
			}
*/
			delete [] filename;

			break;
		}
		case BlubsterOpCode::QueryHit:
		{
			//IncrementCounters(0,0,0,1,0,0);

			// First check to see if we have seen this query hit recently. If so, then return
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
			id.m_count=counter;
			if(v_query_hit_cache.size()>0)
			{
				for(j=(int)v_query_hit_cache.size()-1;j>=0;j--)	// start at end
				{
					if((v_query_hit_cache[j].m_ip==src_ip) && (v_query_hit_cache[j].m_count==counter))
					{
						return;
					}
				}
			}

			// We haven't seen this one before, so add it to the cache and limit the size
			v_query_hit_cache.push_back(id);
			while(v_query_hit_cache.size()>QUERY_HIT_CACHE_SIZE)
			{
				v_query_hit_cache.erase(v_query_hit_cache.begin());
			}
            
			char *ptr=&buf[20];
			
			char *filename=NULL;					// FN
			unsigned int file_length_in_bytes=0;	// FL
			unsigned int bitrate=0;					// BR
			unsigned int sampling_rate=0;			// FC
			unsigned int stereo=0;					// ST (stereo? valid values are 0-3) 3-Stereo, 2-JointStereo, 1-ForcedStereo, 0-Mono
			unsigned int song_length_in_seconds=0;	// SL
			char *md5=NULL;							// CK
			char *nickname=NULL;					// NN

			bool error=true;
			while(*ptr!=NULL)
			{
				if(memcmp(ptr,"FN",2)==0)	// filename
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					filename=new char[len+1];
					memset(filename,0,len+1);
					memcpy(filename,ptr,len);
					ptr+=len;
					error = false;
				}
				else if(memcmp(ptr,"FL",2)==0)	// file length in bytes
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					file_length_in_bytes=0;
					file_length_in_bytes+=((unsigned char)*(ptr++))<<24;
					file_length_in_bytes+=((unsigned char)*(ptr++))<<16;
					file_length_in_bytes+=((unsigned char)*(ptr++))<<8;
					file_length_in_bytes+=((unsigned char)*(ptr++))<<0;
					error = false;
				}
				else if(memcmp(ptr,"BR",2)==0)	// bitrate
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					bitrate=0;
					bitrate+=((unsigned char)*(ptr++))<<16;
					bitrate+=((unsigned char)*(ptr++))<<8;
					bitrate+=((unsigned char)*(ptr++))<<0;
					error = false;
				}
				else if(memcmp(ptr,"FC",2)==0)	// sampling rate
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					sampling_rate=0;
					sampling_rate+=((unsigned char)*(ptr++))<<24;
					sampling_rate+=((unsigned char)*(ptr++))<<16;
					sampling_rate+=((unsigned char)*(ptr++))<<8;
					sampling_rate+=((unsigned char)*(ptr++))<<0;
					error = false;
				}
				else if(memcmp(ptr,"ST",2)==0)	// unknown ST
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					stereo=0;
					stereo+=((unsigned char)*(ptr++))<<24;
					stereo+=((unsigned char)*(ptr++))<<16;
					stereo+=((unsigned char)*(ptr++))<<8;
					stereo+=((unsigned char)*(ptr++))<<0;
					error = false;
				}
				else if(memcmp(ptr,"SL",2)==0)	// song length in seconds
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);
					song_length_in_seconds=0;

					// pointing at data
					song_length_in_seconds+=((unsigned char)*(ptr++))<<24;
					song_length_in_seconds+=((unsigned char)*(ptr++))<<16;

					char byte1 = *ptr;
					song_length_in_seconds+=((unsigned char)*(ptr++))<<8;
					char byte2 = *ptr;
					song_length_in_seconds+=((unsigned char)*(ptr++))<<0;
					//kludge fix for blubster bug?
					if(byte1=='C' && byte2=='K')
						ptr-=2;
					error = false;
				}
				else if(memcmp(ptr,"CK",2)==0)	// md5 checksum
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					md5=new char[len+1];
					memset(md5,0,len+1);
					memcpy(md5,ptr,len);
					ptr+=len;
					error = false;
				}
				else if(memcmp(ptr,"NN",2)==0)	// nickname of sharer
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);

					// pointing at data
					nickname=new char[len+1];
					memset(nickname,0,len+1);
					memcpy(nickname,ptr,len);
					ptr+=len;
				}
				else
				{
					error=true;
					break;
				}
			}

			// If there was no error, then see if we have this ip in our hosts vector. If so, we are now "connected". If not, add it as "connected"
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
			/*
			bool found=false;
			for(i=0;i<v_hosts.size();i++)
			{
				if(v_hosts[i].m_ip==ip)
				{
					found=true;
					v_hosts[i].m_connected=5;
					break;
				}
			}
			if(!found)
			{
//				AddHost(ip);
				SendPing(ip,41170,2);
			}
			*/
			if(error==false)
			{
				string stereo_str;
				switch(stereo)
				{
					case 0:
					{
						stereo_str="Mono (0)";
						break;
					}
					case 1:
					{
						stereo_str="Stereo (1)";
						break;
					}
					case 2:
					{
						stereo_str="Joint Stereo (2)";
						break;
					}
					case 3:
					{
						stereo_str="? Stereo (3)";
						break;
					}
				}

				// Log it
				/*
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - Query Hit - TTL=%u ] from %s ... %s ... %u bytes ... %u kbps ... %u kHz ... %s ... %u seconds ... %s\r\n",
					(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,hdr->TTL(),
					nickname,filename,file_length_in_bytes,bitrate,sampling_rate,(char *)stereo_str.c_str(),song_length_in_seconds,md5);
				Log(msg);
				*/
				if(filename!=NULL && md5 != NULL && nickname != NULL)
					ProcessQueryHit(counter, id.m_ip, nickname,filename,file_length_in_bytes,bitrate,sampling_rate,stereo,song_length_in_seconds,md5);
			}
			else
			{
				/*
				char msg[1024];
				sprintf(msg,"[ %u.%u.%u.%u - ERROR in Query Hit ]\r\n",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
				Log(msg);
				*/
			}

			// Free memory
			if(filename!=NULL)
			{
				delete [] filename;
			}
			if(md5!=NULL)
			{
				delete [] md5;
			}
			if(nickname!=NULL)
			{
				delete [] nickname;
			}
		
			break;
		}
		default:
		{
			IncrementCounters(0,0,0,0,1,0);
			/*
			char msg[1024];
			sprintf(msg,"[ %u.%u.%u.%u - ERROR Unknown Op Code ]\r\n",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			Log(msg);
			*/
			break;
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
void BlubsterSocket::Log(char *buf)
{
//	p_dlg->Log(buf);
	char *ptr=new char[strlen(buf)+1];
	strcpy(ptr,buf);
	::PostMessage(m_hwnd,WM_STATUS_READY,(WPARAM)ptr,(LPARAM)0);
}

//
//
//
void BlubsterSocket::IncrementCounters(unsigned int ping,unsigned int pong,unsigned int query,unsigned int query_hit,unsigned int unknown,unsigned int error_pong)
{
	unsigned int *counts=new unsigned int[6];
	counts[0]=ping;
	counts[1]=pong;
	counts[2]=query;
	counts[3]=query_hit;
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
	sprintf(msg,"*** Sending Pings to %u Connected Hosts ***",connected_count);
/*
	for(i=0;i<v_hosts.size();i++)
	{
		sprintf(&msg[strlen(msg)]," %u",v_hosts[i].m_num_connections);
	}
*/
	Log(msg);
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

//
//
//
void BlubsterSocket::PerformQuery(const char *query, unsigned int counter)
{
	unsigned int i;

	unsigned int buf_len=sizeof(BlubsterHeader)+4+strlen(query);
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(counter);	// send the same counter to all of the hosts
	hdr->SourceIP(0);
	hdr->TTL(MAX_TTL);
	hdr->Op(BlubsterOpCode::Query);
	hdr->DataLength(buf_len-sizeof(BlubsterHeader));

	char query_header[]={'F','N',1,0};
	query_header[3]=(unsigned char)strlen(query);
	memcpy(&buf[sizeof(BlubsterHeader)],query_header,sizeof(query_header));

	CString upper_query=query;
	upper_query.MakeUpper();
	memcpy(&buf[sizeof(BlubsterHeader)+sizeof(query_header)],upper_query,strlen(query));

	for(i=0;i<v_hosts.size();i++)
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

//
//
//
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
void BlubsterSocket::ProcessQuery(int ip, string query, unsigned char ttl)
{
	// First make sure that the query is a valid null-terminated string
	if(IsStringValid(query.c_str())==false)
	{
		return;
	}

	UINT i,j,k;

	// Extract the keywords from the query
	vector<string> keywords;
	ExtractKeywordsFromQuery(&keywords,query.c_str());

	// Check all of the keywords and exact keywords of all of the projects

	for(i=0;i<v_keywords.size();i++)
	{
		if(v_keywords[i].m_piolet_demand_enabled)
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
				// Check to see if we've checked to see if we've seen this GUID before
				/*
				if(!checked)
				{
					checked=true;

					// Check to see if we should ignore this GUID (if we've seen this GUID too many times before)
					// THIS CALL (used to) MAKE THE BEASTIE CRASH ON CLOSE
					int ignore=GUIDCache::Ignore(m_hdr.Guid());
					if(ignore==1)	// -1 - not seen before, 0 - seen before but still respond, 1 - seen before and don't respond
					{
						return;
					}
					else if(ignore==0)
					{
						seen_this_guid_before=true;
					}

					// Check to see if it is one of our own supply-gathering queries
					for(j=0;j<p_sockets->v_project_supply_queries.size();j++)
					{
						if(p_sockets->v_project_supply_queries[j].m_guid==m_hdr.Guid())
						{
							return;
						}
					}
				}
				*/

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
				
				// If we haven't seen this GUID before, then increment the query counters
				p_connection_module->m_status_data.v_project_status[i].IncrementTrackQueryCount(track);
				
				

						ProjectStatus *status=&p_connection_module->m_status_data.v_project_status[i];

						// Save this query to be reported back to the connection manager
						Query qu;
						qu.m_query=query;
	/* =>_<= no spoofing or disting
						qu.m_spoofs_sent=Spoof(pk,p_sockets->v_supply[j],status);
						qu.m_dists_sent=Dist(pk,p_sockets->v_supply[j],status);
	=>_<= */
						//qu.m_hops=m_hdr.Hops();
						qu.m_ttl=ttl;
						qu.m_ip=ip;
						qu.m_track=track;					
						qu.m_timestamp = CTime::GetCurrentTime();
						qu.m_project = status->ProjectName();

						status->v_queries.push_back(qu);
						
						break;
			}
		}
	}
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
void BlubsterSocket::UpdateSupply(vector<SupplyProject>& supply_projects)
{
	v_supply_projects = supply_projects;
}

//
//
//
void BlubsterSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	PerformQuery(psq.m_search_string.c_str(),psq.m_header_counter);
}

//
//
//
void BlubsterSocket::ProcessQueryHit(unsigned int counter, int ip, string nickname, string filename, UINT file_length_in_bytes, int bitrate, int sampling_rate,int stereo,
						int song_length_in_seconds, char* md5)
{
	UINT i;

	// Find the project supply query that is related to this counter
	ProjectSupplyQuery *psq=NULL;
	for(i=0;i<p_connection_module->v_project_supply_queries.size();i++)
	{
		if(p_connection_module->v_project_supply_queries[i].m_header_counter==counter)
		{
			psq=&p_connection_module->v_project_supply_queries[i];
			break;
		}
	}

	// If it is still equal to NULL, then return
	if(psq==NULL)
	{
//		OutputDebugString("Couldn't find GUID for this query in GnutellaSocket::ProcessQueryHit\n");
		return;
	}

	// Find the host connection status data for this project
	ProjectStatus *status=&p_connection_module->m_status_data.v_project_status[psq->m_project_status_index];
	vector<QueryHit> *query_hits=&p_connection_module->m_status_data.v_project_status[psq->m_project_status_index].v_query_hits;


	// Confirm the lower-cased filename with any required keywords
	char *lfilename=new char[strlen(filename.c_str())+1];
	strcpy(lfilename,filename.c_str());
	strlwr(lfilename);
	int track=CheckQueryHitRequiredKeywords(psq,lfilename,file_length_in_bytes);
	if(track>=0)
	{
		// Only increment this count if this is not a searcher query
		// =>_<= if(psq->m_is_searcher_query==false)
		// =>_<= {
		status->IncrementTrackQueryHitCount(track);
		// =>_<= }

		//			(*query_hit_count)++;	// increment the query hit counter

		// Save this query hit to be reported back to the supply manager
		QueryHit qh;
		qh.m_header_counter = counter;
		qh.m_bitrate=bitrate;
		qh.m_nickname=nickname.c_str();
		qh.m_sampling_rate=sampling_rate;
		qh.m_song_length=song_length_in_seconds;
		qh.m_stereo=stereo;
		qh.m_ip=ip;
		qh.m_file_size=file_length_in_bytes;
		qh.Filename(filename.c_str());
		qh.MD5(md5);
		qh.m_track=track;
		qh.m_project = psq->m_project.c_str();
		qh.m_timestamp = CTime::GetCurrentTime();

		query_hits->push_back(qh);

	}
	delete [] lfilename;
}

//
// Return Values (Track)
// ---------------------
// -1 : query hit is not a match
//  0 : query hit is a match but there are no required keywords, so it does not correspond with any track number
// >0 : query hit is a match and is associated with the specified track number
//
int BlubsterSocket::CheckQueryHitRequiredKeywords(ProjectSupplyQuery *psq,char *lfilename,unsigned int file_size)
{
	UINT i,j;
/*
	// Find the appropriate project's supply keywords
	SupplyKeywords *supply_keywords=NULL;
	for(i=0;p_sockets->v_keywords.size();i++)
	{
		if(strcmp(p_sockets->v_keywords[i].m_project_name.c_str(),project)==0)
		{
			supply_keywords=&p_sockets->v_keywords[i].m_supply_keywords;
			break;
		}
	}

	// If we can't find the project's supply keywords
	if(supply_keywords==NULL)
	{
		return -1;
	}
*/
	// Check the file size
	if(file_size<psq->m_supply_size_threshold)
	{
		return -1;
	}

	// Check the kill words
	bool killed=false;
	for(i=0;i<psq->v_killwords.size();i++)
	{
		if(strstr(lfilename,psq->v_killwords[i])!=NULL)
		{
			return -1;
		}
	}
	
	// Check the required keywords, to determine the track.
	if(psq->v_keywords.size()==0)
	{
		return 0;
	}

	// Check all of the required keywords tracks
	for(i=0;i<psq->v_keywords.size();i++)
	{
		// Check the required keywords for this track
		bool found=true;
		for(j=0;j<psq->v_keywords[i].v_keywords.size();j++)
		{
			if(strstr(lfilename,psq->v_keywords[i].v_keywords[j])==NULL)
			{
				found=false;
				break;
			}
		}

		// If we found a matching track
		if(found)
		{
			return psq->v_keywords[i].m_track;
		}
	}
	
	// None of the required keywords matched
	return -1;
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
	m_max_host=5000;
	m_max_host_cache=5000;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletDC\\Host Limits";

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
