// BlubsterSocket.cpp

#include "stdafx.h"
#include "BlubsterSocket.h"
#include "Checksum.h"
#include "BlubsterOpCode.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"
#include "PioletPoisonerDlg.h"
#define REQUEST_CACHE_SIZE	1000
#define MAX_TTL				10


//#define HOSTS_LIMIT			5000
//#define HOST_CACHE_LIMIT	5000
//#define QUERY_CACHE_SIZE	500
//#define QUERY_HIT_CACHE_SIZE	500
//#define IP_HDRINCL 2	// for ip spoofing

//
//
//
BlubsterSocket::BlubsterSocket()
{
//	m_host_cache_counter=0;
//	m_ping_timer=19;	// the first time the timer fires, pings will be sent

	// Create the socket
	Create(41170);

//	LoadHostCache();
//	ReadMaximumHostsDataFromRegistry();
	srand((unsigned)time(NULL));
	m_header_counter = 1;

}

//
//
//
BlubsterSocket::~BlubsterSocket()
{
}

//
//
//
/*
void BlubsterSocket::InitHwnd(HWND hwnd)
{
	m_hwnd=hwnd;
}
*/

//
//
//
void BlubsterSocket::InitParent(ConnectionModule* parent)
{
	p_parent = parent;
}

//
//
//
void BlubsterSocket::OneMinuteTimer(HWND hwnd)
{

}

//
// 1 second timer
//
void BlubsterSocket::TimerHasFired(HWND hwnd)
{

	/*
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

	// Check to see if the ping timer is at 30 seconds
	m_ping_timer++;
	if(m_ping_timer==30)
	{
		m_ping_timer=0;
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
	*/
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

//			IncrementCounters(1,0,0,0,0,0);

			// Send pong in reply
			/*
			char log[1024];
			p_parent->Log("Ping:",0x00FF0000, true);
			sprintf(log, " %u.%u.%u.%u\n", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			p_parent->Log(log);
			*/
			SendPong(ip);
			

			break;
		}
		case BlubsterOpCode::Pong:
		{
			/*
			char log[1024];
			p_parent->Log("Pong:",0x00FF0000, true);
			sprintf(log, " %u.%u.%u.%u\n", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			p_parent->Log(log);
			*/
			break;
		}
		case BlubsterOpCode::Query:
		{
//			IncrementCounters(0,0,1,0,0,0);
			/*
			char log[1024];
			p_parent->Log("Query:",0x00FF0000, true);
			sprintf(log, " %u.%u.%u.%u\n", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			p_parent->Log(log);
			*/
			break;
		}
		case BlubsterOpCode::QueryHit:
		{
			//IncrementCounters(0,0,0,1,0,0);
			/*
			char log[1024];
			p_parent->Log("QueryHit:",0x00FF0000, true);
			sprintf(log, " %u.%u.%u.%u\n", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			p_parent->Log(log);
			*/
			break;
		}
		case BlubsterOpCode::FileRequest:
		{
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
			if(v_file_request_cache.size()>0)
			{
				for(j=(int)v_file_request_cache.size()-1;j>=0;j--)	// start at end
				{
					if((v_file_request_cache[j].m_ip==src_ip) && (v_file_request_cache[j].m_count==counter))
					{
						return;
					}
				}
			}

			// We haven't seen this one before, so add it to the cache and limit the size
			v_file_request_cache.push_back(id);
			while(v_file_request_cache.size()>REQUEST_CACHE_SIZE)
			{
				v_file_request_cache.erase(v_file_request_cache.begin());
			}
            
			char *ptr=&buf[20];


			char *filename=NULL;
			int port=0;
			bool error = false;
			while(*ptr!=NULL)
			{
				if(memcmp(ptr,"FN",2)==0)	// # of shared files
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);
					// pointing at data
					filename=new char[len+1];
					memset(filename,0,len+1);
					memcpy(filename,ptr,len);
					ptr+=len;
			
				}
				else if(memcmp(ptr,"PT",2)==0)
				{
					ptr+=2;	// skip 2-char header
					unsigned char is_ascii=(unsigned char)(*(ptr++));
					unsigned char len=(unsigned char)*(ptr++);
					
					port=0;
					port+=((unsigned char)*(ptr++))<<24;
					port+=((unsigned char)*(ptr++))<<16;
					port+=((unsigned char)*(ptr++))<<8;
					port+=((unsigned char)*(ptr++))<<0;
				}
				else
				{
					error=true;
					break;
				}
			}
			char log[1024];
			p_parent->Log("FileRequest: ",0x00FF0000, true); // blue and bold
			char ip_str[32];
			int ip1,ip2,ip3,ip4;
			ip1 = ip>>0&0xFF;
			ip2 = ip>>8&0xFF;
			ip3 = ip>>16&0xFF;
			ip4 = ip>>24&0xFF;
			sprintf(ip_str, "%u.%u.%u.%u", ip1,ip2,ip3,ip4);

			if(ip1 == 64 && ip2==60 && (ip3==52 || ip3==161))
			{
				p_parent->Log(ip_str, 0x000000FF,true); //RED and bold
			}
			else
			{
				p_parent->Log(ip_str);
			}
			
			sprintf(log, ", File: %s, Port: %d\n", filename, port);
			p_parent->Log(log);
			
			//start file transfering
			PoisonEntry* pe = new PoisonEntry();
			*pe = GetPoisonEntry(filename);
			if(pe->m_filesize!=0 && pe->m_md5.length()>0)
			{
				pe->m_requester_ip=ip_str;
				pe->m_request_port=port;
				p_parent->ReceivedFileRequest(pe);
			}
			else
			{
				delete pe;
			}


			delete [] filename;	
			break;
		}
		default:
		{
//			IncrementCounters(0,0,0,0,1,0);
			/*
			char log[1024];
			p_parent->Log("Unknown:",0x000000FF, true);
			sprintf(log, " %u.%u.%u.%u\n", (ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
			p_parent->Log(log);
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
	hdr->Counter(++m_header_counter);
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

	pong_data[34]=(unsigned char)64;
/*
	if(v_hosts.size()>255)
	{
		pong_data[34]=(unsigned char)255;
	}
	else
	{
		pong_data[34]=(unsigned char)v_hosts.size();
	}
*/
	unsigned int buf_len=sizeof(BlubsterHeader)+sizeof(pong_data);
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	BlubsterHeader *hdr=(BlubsterHeader *)buf;
	hdr->Counter(++m_header_counter);
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
/*
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
	Log(msg);
}
*/
/*
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
*/

//
//
//
PoisonEntry BlubsterSocket::GetPoisonEntry(const char* filename)
{
	PoisonEntry pe;
	//get the filename
	char* filename_ptr = strrchr(filename, '\\');
	filename_ptr++;
	pe.m_filename = filename_ptr;
	CString md5_temp = filename;
	int index = md5_temp.ReverseFind('\\');
	md5_temp.Delete(index,md5_temp.GetLength()-index);

	//get the filesize
	char* temp = new char[md5_temp.GetLength()+1];
	strcpy(temp,md5_temp);
	char* temp1 = strrchr(temp,'\\');
	if(temp1!=NULL)
	{
		temp1++;
		char* stop;
		pe.m_filesize = strtoul(temp1, &stop,10);
	}
	delete [] temp;	

	index = md5_temp.ReverseFind('\\');
	md5_temp.Delete(index,md5_temp.GetLength()-index);

	//get the hash
	temp = new char[md5_temp.GetLength()+1];
	strcpy(temp,md5_temp);
	char* md5 = strrchr(temp,'\\');
	if(md5!=NULL)
	{
		md5++;
		pe.m_md5 = md5;
	}
	delete [] temp;	

	return pe;
}
