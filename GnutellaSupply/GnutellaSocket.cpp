// GnutellaSocket.cpp

#include "stdafx.h"
#include "GnutellaSocket.h"
#include "PongData.h"
#include "PushData.h"
#include "ConnectionSockets.h"
#include "GUIDCache.h"
#include "QueryHitHeader.h"
#include "QueryHitResultHeader.h"
#include "FalseQueryHit.h"

//#include "BSA.h"

#define MAX_SEND_BUFFERS_ALLOWED	500		// was 1000
#define MAX_CACHED_QUERY_HITS		10000

//
//
//
GnutellaSocket::GnutellaSocket()
{
	ResetData();
}

//
//
//
GnutellaSocket::~GnutellaSocket()
{
	ResetData();
}

//
//
//
void GnutellaSocket::InitParent(ConnectionSockets *sockets)
{
	p_sockets=sockets;
}

//
//
//
void GnutellaSocket::ResetData()
{
	UINT i;

	m_received_gnutella_connection_string=false;
	m_received_gnutella_header=false;
	m_qrp_table_sent=false;
	
	m_connecting_to_limewire_reflector=false;

	m_on_receive_cached=false;

	m_status_data.Clear();
//	m_host_connection_status.Clear();
//	v_project_supply_queries.clear();

	for(i=0;i<v_cached_query_hits_to_send.size();i++)
	{
		delete [] v_cached_query_hits_to_send[i];
	}
	v_cached_query_hits_to_send.clear();

	m_query_count=0;
	v_query_counts.clear();

	m_state=0;
}

//
//
//
int GnutellaSocket::Connect(GnutellaHost &host)
{
//	m_host_connection_status.m_host=host;
//	m_host_connection_status.m_connect_time=CTime::GetCurrentTime();
	m_status_data.m_host=host;
	m_status_data.m_connect_time=CTime::GetCurrentTime();
	
	return TEventSocket::Connect((char *)host.Host().c_str(),host.Port());
}

//
//
//
int GnutellaSocket::Close()
{
/*
	char msg[1024];
	sprintf(msg,"0x%08x - GnutellaSocket::Close()\n",this);
	OutputDebugString(msg);
*/
/*
	if(m_state>=34)
	{
	//	char msg[1024];
		//sprintf(msg,"0x%08x - GnutellaSocket::Close() - State : %u\n",this,m_state);
		CTimeSpan ts = CTime::GetCurrentTime() - m_status_data.m_up_time;
		char vendor[128];
		strcpy(vendor, m_status_data.m_remote_vendor.c_str());
		TRACE("GnutellaSocket::Close() - State: %u Connected: %u sec Vendor: %s\n ",
			m_state,
			ts.GetTotalSeconds(), vendor);
	}
*/
	ResetData();

	return TEventSocket::Close();
}

//
//
//
void GnutellaSocket::OnConnect(int error_code)
{
/*
	char msg[1024];
	sprintf(msg,"OnConnect() - error code %u\n",error_code);
	OutputDebugString(msg);
*/
	if(error_code!=0)
	{
		Close();
//		TRACE("0x%08x - GnutellaSocket::OnConnect() - Error: %d\n",this,error_code);
		return;
	}

	m_state=10;

	string str;
/*
	// Check to see if we are connecting to a bearshare reflector ... if so, say we are a BearShare
	if(strstr((char *)m_status_data.m_host.Host().c_str(),"bearshare.net")!=0)
	{
		// Send GNUTELLA CONNECT (as BearShare 4.0.0)
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="User-Agent: BearShare 4.0.0\r\n";
		str+="Machine: 1.13.383,1,733\r\n";
		str+="Pong-Caching: 0.1\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Hops-Flow: 1.0\r\n";
		str+="Listen-IP: 0.0.0.0\r\n";
		str+="Remote-IP: 208.239.76.99\r\n";
		str+="\r\n";
	}
	else if(strstr((char *)m_status_data.m_host.Host().c_str(),"limewire.com")!=0)	// check to see if we are connecting to a limewire reflector
	{
		// If we are, then connect old-school style...and funky, so that we get pongs from this limewire reflector biatch
		m_connecting_to_limewire_reflector=true;

		str="GNUTELLA CONNECT/0.4\n\n";
	}
	else	// we are a Morpheus
	{
*/

	//if(rand()%2==0) // 50% chance connects as Limewire and 50% chance connects at Morpheus
	//{
		//Send LimeWire CONNECT
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="X-Max-TTL: 4\r\n";
		str+="X-Dynamic-Querying: 0.1\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="User-Agent: LimeWire/4.0.5\r\n";
#ifdef COMPRESSION_ENABLED
		// Accept-Encoding Header
		if(m_dnapressionOn)
			str += "Accept-Encoding: deflate\r\n";
#endif
		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
		//str+="Pong-Caching: 0.1\r\n";
		//str+="X-Guess: 0.1\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Degree: 32\r\n";
		//str+="X-Locale-Pref: en\r\n";
		char remote_ip[32];
		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
		str+=remote_ip;
		str+="\r\n";
//	}
//	else
//	{
//		str="GNUTELLA CONNECT/0.6\r\n";
//		str+="User-Agent: Morpheus 4.0.53.212 (GnucDNA 1.0.2.6)\r\n";
//		str+="X-Ultrapeer: False\r\n";
//		str+="X-Query-Routing: 0.1\r\n";	// QRP
//		str+="X-Max-TTL: 4\r\n";
//		str+="X-Dynamic-Querying: 0.1\r\n";
//		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
//		//str+="Pong-Caching: 0.1\r\n";
//		//str+="X-Guess: 0.1\r\n";
//		str+="X-Ultrapeer: False\r\n";
//		str+="X-Degree: 32\r\n";
//		//str+="X-Locale-Pref: en\r\n";
//		char remote_ip[32];
//		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
//		str+=remote_ip;
//#ifdef COMPRESSION_ENABLED
//		// Accept-Encoding Header
//		if(m_dnapressionOn)
//			str += "Accept-Encoding: deflate\r\n";
//#endif
//		str+="\r\n";
//	}

/*
		// Send GNUTELLA CONNECT (as Morpheus 4.0.53.212)
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: Morpheus 4.0.53.212 (GnucDNA 1.0.2.6)\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*

		str="GNUTELLA CONNECT/0.6\r\n";
		str+="X-Max-TTL: 4\r\n";
		str+="X-Dynamic-Querying: 0.1\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="User-Agent: LimeWire/4.0.5\r\n";
		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
		//str+="Pong-Caching: 0.1\r\n";
		//str+="X-Guess: 0.1\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Degree: 32\r\n";
		str+="X-Locale-Pref: en\r\n";
		char remote_ip[32];
		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
		str+=remote_ip;
		str+="\r\n";
*/

/*
	// Send GNUTELLA CONNECT (as Morpheus 3.0.2.9)
		str="GNUTELLA CONNECT/0.6\r\n";
		//str+="User-Agent: Gnucleus 2.0.1.0(GnucDNA 1.0.2.6)\r\n";

#ifdef ULTRAPEER
		str+="X-Ultrapeer: True\r\n";
		str+="X-Leaf-Max: 400\r\n";
#else
		str+="X-Ultrapeer: False\r\n";
		str+="X-Max-TTL: 4\r\n";
		str+="X-Dynamic-Querying: 0.1\r\n";
		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
		//str+="Pong-Caching: 0.1\r\n";
		//str+="X-Guess: 0.1\r\n";
		str+="X-Degree: 32\r\n";
		//str+="X-Locale-Pref: en\r\n";
		char remote_ip[32];
		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
		str+=remote_ip;
#endif
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*
		str="GNUTELLA CONNECT/0.6\r\n";
		//str+="User-Agent: Morpheus 3.0.2.9 (GnucDNA 0.9.1.5)\r\n";
		//str+="User-Agent: Gnucleus 1.8.4.0\r\n";
		str+="User-Agent: Morpheus 4.0.53.165 (GnucDNA 1.0.2.1)\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT (as Morpheus 1.9.1.0)
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: MorpheusOS 1.9.1.0\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT (as Morpheus 1.9.1.0) but with GGEP support and no uptime
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: MorpheusOS 1.9.1.0\r\n";
		str+="GGEP: 0.5\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT (as Gnucleus 1.8.4.0)
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: Gnucleus 1.8.4.0\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
//		char uptime[32];
//		srand((unsigned int)time(NULL));
//		sprintf(uptime, "Uptime: %2dD %2dH %2dM\r\n", rand()%7, rand()%24, rand()%60);
		str+="Uptime: 0D 00H 00M\r\n";
//		str+=uptime;
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT as Gnucleus 1.8.4.0 but with GGEP support
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: Gnucleus 1.8.4.0\r\n";
		str+="GGEP: 0.5\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT as Limewire
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: Limewire/2.8.5\r\n";
		str+="GGEP: 0.5\r\n";
		str+="X-Ultrapeer: False\r\n";
//		str+="X-Guess: 0.1\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
//		str+="Uptime: 0D 00H 00M\r\n";
		str+="\r\n";
*/
/*
		// Send GNUTELLA CONNECT (as Morpheus 2.0.2.2)
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: MorpheusOS 2.0.2.2\r\n";
		str+="X-Ultrapeer: False\r\n";
//		str+="PONG-CACHING: 0.1\r\n";
//		str+="BYE-PACKET: 0.1\r\n";
		str+="UPTIME: 0D 0H 0M\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
//		str+="LISTEN-IP: 0.0.0.0\r\n";
		str+="\r\n";
*/
  //}

	SendSocketData((char *)str.c_str(),(UINT)str.size());
}

//
//
//
void GnutellaSocket::SocketDataSent(unsigned int len)
{
	// Check to see if we've got some OnReceive(s) cached.
	if(m_on_receive_cached)
	{
		// Check to see if we have sent out enough buffers
		if(v_send_data_buffers.size()<MAX_SEND_BUFFERS_ALLOWED)
		{
			OnReceive(0);
			return;
		}
	}
}

//
//
//
void GnutellaSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}
	
	// Check to see if we need to chache this OnReceive and send some data buffers out
	if(v_send_data_buffers.size()>MAX_SEND_BUFFERS_ALLOWED)
	{
		m_on_receive_cached=true;
		return;
	}

	m_on_receive_cached=false;

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	// If we haven't received the connection string yet
	if(m_received_gnutella_connection_string==false)
	{
		ReceiveSomeSocketData(4096);
		return;
	}

	if(m_received_gnutella_header)
	{
		// The data will be read
		return;
	}

	ReceiveGnutellaHeader();

	//
	// Check to see if there is any more data waiting to be read. KLUDGE (???)
	//

	char buf[1];

	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read;
	DWORD flags=MSG_PEEK;	// this variable is both an input and an output
	
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(num_read!=0)
	{
		OnReceive(0);	// bring on the recursion
	}
}

//
//
//
void GnutellaSocket::OnClose(int error_code)
{
	TEventSocket::OnClose(error_code);

	//OutputDebugString("GnutellaSocket::OnClose()\r\n");
	//if(error_code != 0)
	//	TRACE("GnutellaSocket::OnClose() Error code: %d\r\n",error_code);
}

//
//
//
void GnutellaSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	m_state=20;

	// First check to see if we are connecting to a limewire reflector...if so we gotta do a bunch of stupid shit
	if(m_connecting_to_limewire_reflector)
	{
		if(strstr(data,"GNUTELLA OK\n\n")==NULL)
		{
			Close();
			return;
		}
		
		m_received_gnutella_connection_string=true;
		m_status_data.m_up_time=CTime::GetCurrentTime();	// we are now "up"

		// Send the unique ping with the port and ip attached to the end
		unsigned char buf[sizeof(GnutellaHeader)+30];
		memset(buf,0,sizeof(buf));
		GnutellaHeader *hdr=(GnutellaHeader *)buf;
		hdr->Init();
		hdr->Op(GnutellaOpCode::Ping);
		hdr->Length(30);
	
		// port
		unsigned short int port=6346;
		memcpy(&buf[sizeof(GnutellaHeader)],&port,sizeof(unsigned short int));

		// ip
		if(rand()%2)	// 192.168.0.1-255
		{
			buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)]=192;
			buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)+1]=168;
		}
		else			// 10.0.0.1-255
		{
			buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)]=10;
			buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)+1]=0;
		}
		buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)+2]=0;
		buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)+3]=(rand()%255)+1;

		// 8 0's

		memcpy(&buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)+sizeof(unsigned int)+8],"none:1000:false",strlen("none:1000:false"));

		SendSocketData(buf,sizeof(buf));

		return;
	}

	m_state=21;

/*
	char msg[1024];
	sprintf(msg,"0x%08x - GnutellaSocket::SomeSocketDataReceived()\n",this);
	OutputDebugString(msg);
*/
	// Check to see if we've not gotten the \r\n\r\n. If not, then return
	if(strstr((char *)data,"\r\n\r\n")==NULL)
	{
		// Check to see if we're read in the 4K...if so, then close the socket...this is too big of a buffer
		if(data_len==4096)
		{
			Close();
		}
		else	// else we have not filled the buffer yet, so receive some more data
		{
			// If there was data waiting there, try to receive some more, else wait for next OnReceive
			if(new_len>0)
			{
/*
				char msg[1024];
				sprintf(msg,"0x%08x - ReceiveSomeMoreSocketData()\n",this); 
				OutputDebugString(msg);
*/
				ReceiveSomeMoreSocketData(data,data_len,max_len);
			}
			else
			{
				m_receiving_some_socket_data=true;
			}
		}

		return;
	}

	m_state=22;

/*	
	sprintf(msg,"0x%08x - Received Gnutella Connection Header in GnutellaSocket::SomeSocketDataReceived()\n",this);
	OutputDebugString(msg);
*/

	// Check to see if he said OK
	if(strstr(data,"GNUTELLA/0.6 200")==NULL)
	{
		// Check to see if he said Full
		if(strstr(data,"GNUTELLA/0.6 503")!=NULL)
		{
		}
		else if(strstr(data,"GNUTELLA/0.6 500")!=NULL)
		{
			// error connecting...so :p on them
		}
		else
		{
//			OutputDebugString("didn't reply with GNUTELLA/0.6 200 or 503\n");
		}

		Close();
		return;
	}

	m_state=23;
	
	// Extract the User-Agent:
	m_status_data.m_remote_vendor="???";
	if(strstr(data,"User-Agent: ")!=NULL)
	{
		char *user_agent=new char[strlen(strstr(data,"User-Agent: "))+1];
		strcpy(user_agent,strstr(data,"User-Agent: "));
		
#ifdef BEARSHARE_ONLY
		if(strstr(user_agent,"BearShare")==NULL)
		{
			delete [] user_agent;
			Close();
			return;
		}
#else
		//drop all bearshare connections
		//if(strstr(user_agent,"BearShare")!=NULL)
		//{
		//	delete [] user_agent;
		//	Close();
		//	return;
		//}
		if( (strstr(user_agent,"LimeWire")==NULL))
		{
			//if( (rand()%10) != 0)
			//{
				delete [] user_agent;
				Close();
				return;
			//}
		}
#endif
		if(strstr(user_agent,"\r\n")!=NULL)
		{
			*strstr(user_agent,"\r\n")='\0';
			m_status_data.m_remote_vendor=user_agent+strlen("User-Agent: ");
		}

		delete [] user_agent;
	}
	// See if there are any ips in here (some clients put X-Try and X-Try-Ultrapeer even if the connection is OK
	ExtractHostsFromHttpHeader(data);

/*
	// TEMP KLUDGE
	char tmp_buf[1024];
	strcpy(tmp_buf,m_status_data.m_remote_vendor.c_str());
	strlwr(tmp_buf);
	if(strstr(tmp_buf,"bearshare")!=NULL)
	{
		OutputDebugString("Closing - BearShare\n");
		Close();
		return;
	}
*/

	m_state=24;

//	OutputDebugString(data);
	
	// Make sure they said they support the X-Query-Routing
	bool qrp_supported=true;
	if(strstr(data,"X-Query-Routing: 0.1\r\n")==NULL)
	{
		// KLUDGE - If it is one of the reflector hosts, don't care if they don't support QRP
		if(m_status_data.m_host.IP()!=0)
		{
//			char msg[1024];
//			sprintf(msg,"%15s %20s - does not support X-Query-Routing 0.1 - Closing...\n",m_host_connection_status.m_host.Host().c_str(),m_host_connection_status.m_remote_vendor.c_str());
//			OutputDebugString(msg);
			Close();
			return;
		}
	}

	m_state=25;

	// Make sure they don't say that they are not an ultrapeer 
	bool is_ultrapeer=true;
	if(strstr(data,"X-Ultrapeer: ")!=NULL)
	{
		char *ultrapeer=new char[strlen(strstr(data,"X-Ultrapeer: "))+1];	
		strcpy(ultrapeer,strstr(data,"X-Ultrapeer: "));

		if(strstr(ultrapeer,"\r\n")!=NULL)
		{
			*strstr(ultrapeer,"\r\n")='\0';
			strlwr(ultrapeer);
			if(strstr(ultrapeer,"false")!=NULL)
			{
				is_ultrapeer=false;
			}
		}

		delete [] ultrapeer;
	}
	
	// Check to see if they are not an ultrapeer
	if(!is_ultrapeer)	
	{
		Close();
		return;
	}

	m_state=26;
	
	// They said OK
	m_received_gnutella_connection_string=true;
	m_status_data.m_up_time=CTime::GetCurrentTime();	// we are now "up"
	
	// Say OK
	SendSocketData("GNUTELLA/0.6 200 OK\r\n\r\n",(UINT)strlen("GNUTELLA/0.6 200 OK\r\n\r\n"));

	m_state=27;

	// Send ping to get some pongs back for some hosts
	SendPing(1);

	m_state=28;

	// Send my QRP table
	SendQRPTable();

	m_state=29;
}

//
//
//
void GnutellaSocket::ExtractHostsFromHttpHeader(char *data)
{
	char *buf=new char[strlen(data)+1];
	strcpy(buf,data);
	strlwr(buf);	// make lowercase

	char *xtry=NULL;
	char *ptr=NULL;
/*
	// check for x-try
	if(strstr(buf,"x-try:")!=NULL)
	{
		// check for \r\n at the end of the x-try
		if(strstr(strstr(buf,"x-try:"),"\r\n")!=NULL)
		{
			xtry=new char[strlen(strstr(buf,"x-try:"))+1];
			strcpy(xtry,strstr(buf,"x-try:"));
			ptr=xtry;
			
			*strstr(ptr,"\r\n")='\0';	// null terminate this part of the HTTP header

			ptr+=strlen("x-try:");
			while(*ptr==' ')
			{
				if(*ptr=='\0')
				{
					break;
				}

				ptr++;
			}

			// Read in all of the hosts
			vector<GnutellaHost> hosts;
			while(*ptr!='\0')
			{
				char *host=new char[strlen(ptr)+1];
				strcpy(host,ptr);
				if(strchr(host,',')!=NULL)
				{
					*strchr(host,',')='\0';
					ptr+=strlen(host)+1;	// strlen + comma
				}
				else
				{
					ptr+=strlen(host);	// strlen but no comma
				}

				if(strchr(host,':')!=NULL)
				{
					GnutellaHost gnutella_host;
					gnutella_host.Port(strtoul(strchr(host,':')+1,NULL,10));
					*strchr(host,':')='\0';
					gnutella_host.Host(host);

					hosts.push_back(gnutella_host);

					// Store the ip for when we need to send spoofs...so that we can randomize the last two digits
					p_sockets->SpoofIP(gnutella_host.IP());
				}

				delete [] host;
			}

			// Now we have a vector of gnutella hosts...add this to the status
			p_sockets->ReportHosts(hosts);
		}
	}
*/
	if(xtry!=NULL)
	{
		delete [] xtry;
		xtry=NULL;
	}

	// check for x-try-ultrapeers
	if(strstr(buf,"x-try-ultrapeers:")!=NULL)
	{
		// check for \r\n at the end of the x-try
		if(strstr(strstr(buf,"x-try-ultrapeers:"),"\r\n")!=NULL)
		{
			xtry=new char[strlen(strstr(buf,"x-try-ultrapeers:"))+1];
			strcpy(xtry,strstr(buf,"x-try-ultrapeers:"));
			ptr=xtry;
			
			*strstr(ptr,"\r\n")='\0';	// null terminate this part of the HTTP header
			
			ptr+=strlen("x-try-ultrapeers:");
			while(*ptr==' ')
			{
				if(*ptr=='\0')
				{
					break;
				}

				ptr++;
			}

			// Read in all of the hosts
			vector<GnutellaHost> hosts;
			while(*ptr!='\0')
			{
				char *host=new char[strlen(ptr)+1];
				strcpy(host,ptr);
				if(strchr(host,',')!=NULL)
				{
					*strchr(host,',')='\0';
					ptr+=strlen(host)+1;	// strlen + comma
				}
				else
				{
					ptr+=strlen(host);	// strlen but no comma
				}

				if(strchr(host,':')!=NULL)
				{
					GnutellaHost gnutella_host;
					gnutella_host.Port(strtoul(strchr(host,':')+1,NULL,10));
					*strchr(host,':')='\0';
					gnutella_host.Host(host);

					hosts.push_back(gnutella_host);

					// Store the ip for when we need to send spoofs...so that we can randomize the last two digits
					//p_sockets->SpoofIP(gnutella_host.IP());
				}

				delete [] host;
			}

			// Now we have a vector of gnutella hosts...add this to the status
			p_sockets->ReportHosts(hosts);
		}
	}

	if(xtry!=NULL)
	{
		delete [] xtry;
		xtry=NULL;
	}

	delete [] buf;
}

//
//
//
void GnutellaSocket::ReceiveGnutellaHeader()
{
	ReceiveSocketData(sizeof(GnutellaHeader));
}

//
//
//
void GnutellaSocket::SocketDataReceived(char *data,unsigned int len)
{
	// Check to see if we are receiving a header or data
	if(m_received_gnutella_header==false)	// header
	{
		if(len==sizeof(GnutellaHeader))
		{
			m_received_gnutella_header=true;

			memcpy(&m_hdr,data,sizeof(GnutellaHeader));
			ReceivedGnutellaHeader();
		}
		else
		{
			Close();
		}
	}
	else	// data
	{
		if(len==m_hdr.Length())
		{
			m_received_gnutella_header=false;

			ReceivedGnutellaData(data);
		}
		else
		{
			Close();
		}
	}
}

//
//
//
void GnutellaSocket::ReceivedGnutellaHeader()
{
	switch(m_hdr.Op())
	{
		case GnutellaOpCode::Ping:
		{
			m_state=30;
/*
			// Check valid size
			if(m_hdr.Length()!=0)
			{
				Close();
				return;
			}
*/
			// With GGEP, there may be some GGEP extension data after the ping data

			// If the number of hops is 0, then send a 1 TTL pong
			/*
			if(m_hdr.Hops()==0)
			{
				SendPong(1);
			}
			*/
			
			if(m_hdr.Hops()<3)
			{
				SendPong(m_hdr.Hops());
			}
			else
			{
				SendPong(3);
			}
			/* Skycat */


//			OutputDebugString("Received Ping\n");

			// No data to received
			m_received_gnutella_header=false;

			p_sockets->m_status_data.m_ping_count++;

			break;
		}
		case GnutellaOpCode::Pong:
		{
			m_state=31;
/*
			// Check for valid size
			if(m_hdr.Length()!=sizeof(PongData))
			{
				Close();
				return;
			}
*/
			// With GGEP, there may be some GGEP extension data after the pong data
			//if(m_hdr.Length()!=sizeof(PongData))
			//{
			//	TRACE("Pong: header lenght is %d, != to PongData size: %d, closing socket\n",m_hdr.Length(),sizeof(PongData));
			//	Close();
			//	return;
			//}

//			OutputDebugString("Pong Header\n");

			ReceiveSocketData(m_hdr.Length());

			break;
		}
		case GnutellaOpCode::QRPTableUpdate:
		{
			m_state=32;

			// Check for data length validity
			if((m_hdr.Length()>5000)||(m_hdr.Length()<6))
			{
				TRACE("QRPTableUpdate: header lenght is %d, which > 5000 or < 6,  closing socket\n",m_hdr.Length());
				Close();
				return;
			}

			ReceiveSocketData(m_hdr.Length());

			break;
		}
		case GnutellaOpCode::Push:
		{
			m_state=33;
/*
			// Check for valid size
			if(m_hdr.Length()!=sizeof(PushData))
			{
				Close();
				return;
			}
*/

			// With GGEP, there may be some GGEP extension data after the push data
			if( m_hdr.Length() != sizeof(PushData))
			{
				TRACE("PushData: header lenght is %d, !=  sizeof(PushData) = %d, closing socket\n",m_hdr.Length(), sizeof(PushData));
				Close();
				return;
			}

//			OutputDebugString("Push Header\n");

			ReceiveSocketData(m_hdr.Length());

			break;
		}
		case GnutellaOpCode::Query:
		{
			m_state=34;

			// Check for data length validity
			if((m_hdr.Length()>1000)||(m_hdr.Length()==0))
			{
				TRACE("Query: header lenght is %d, which is > 1000 or == 0, closing socket\n",m_hdr.Length());
				Close();
				return;
			}

//			OutputDebugString("Query Header\n");

			ReceiveSocketData(m_hdr.Length());

			break;
		}
		case GnutellaOpCode::QueryHit:
		{
			m_state=35;

			// Check for data length validity
			if((m_hdr.Length()>500000)||(m_hdr.Length()==0))
			{
				TRACE("QueryHit: header lenght is %d, which is > 500000 or == 0, closing socket\n",m_hdr.Length());
				Close();
				return;
			}
		
//			OutputDebugString("QueryHit Header\n");

			ReceiveSocketData(m_hdr.Length());

			break;
		}
		default:
		{
			m_state=36;

			Close();
/*
			char msg[1024];
			sprintf(msg,"Received Unknown Op Code : %u - Len : %u - Hops : %u - TTL : %u\n",m_hdr.Op(),m_hdr.Length(),m_hdr.Hops(),m_hdr.TTL());
			OutputDebugString(msg);
			ReceiveSocketData(m_hdr.Length());
*/
			break;
		}
	}
}

//
//
//
void GnutellaSocket::ReceivedGnutellaData(char *data)
{
	switch(m_hdr.Op())
	{
		case GnutellaOpCode::Ping:
		{
			// Should never get here
			break;
		}
		case GnutellaOpCode::Pong:
		{
/*
			// Check for valid size
			if(m_hdr.Length()!=sizeof(PongData))
			{
				Close();
				return;
			}
*/
			// With GGEP support, there may be some GGEP extension data after the pong data
			if(m_hdr.Length()<sizeof(PongData))
			{
				Close();
				return;
			}

//			OutputDebugString("Pong Data\n");

			ProcessPong(data);

			p_sockets->m_status_data.m_pong_count++;

			break;
		}
		case GnutellaOpCode::QRPTableUpdate:
		{
			// Do nothing

			break;
		}
		case GnutellaOpCode::Push:
		{
			// Check for valid size
			if(m_hdr.Length()!=sizeof(PushData))
			{
				Close();
				return;
			}

//			OutputDebugString("Push Data\n");

			p_sockets->m_status_data.m_push_count++;

			break;
		}
		case GnutellaOpCode::Query:
		{
			if(m_hdr.Length() > 1024*1024 && m_hdr.Length() < 4 ) //not allowing any query greater than 1MB in length
				break;

			p_sockets->m_status_data.m_query_count++;
			//m_query_count++;

			// Extract query data
			unsigned short int *min_speed=(unsigned short int *)data;
			char *query=&data[sizeof(unsigned short int)];
			if(query==NULL)
				break;
			if(strlen(query)==0)
				break;

			char* query_xml = new char[m_hdr.Length()];
			if( m_hdr.Length() >= (strlen(query)+1))
				strcpy(query_xml,query);
			else
			{
				delete [] query_xml;
				break;
			}

			char *xml=NULL;
			if( (m_hdr.Length()-sizeof(unsigned short int)-strlen(query)-1) > 0)
			{
				xml=&data[sizeof(unsigned short int)+strlen(query)+1];
				//looking for artist tag
				char* artist=strstr(xml,"artist=\"");
				if(artist!=NULL)
				{
					if(strlen(artist) > 8)
					{
						artist+=8;
						char *end = strchr(artist,'\"');
						if(end!=NULL)
						{
							*end='\0';
							strcat(query_xml, " ");
							strcat(query_xml, artist);
						}
					}
				}
				/*
				//looking for album tag
				char* album=strstr(xml,"album=\"");
				if(album!=NULL)
				{
					if(strlen(album) > 7)
					{
						album+=8;
						char *end = strchr(album,'\"');
						if(end!=NULL)
						{
							*end='\0';
							strcat(query_xml, " ");
							strcat(query_xml, album);
						}
					}
				}
				*/

			}

			ProcessQuery(query_xml);
			delete [] query_xml;
			break;
		}
		case GnutellaOpCode::QueryHit:
		{
//			OutputDebugString("QueryHit Data\n");

			p_sockets->m_status_data.m_query_hit_count++;

			ProcessQueryHit(data);

			break;
		}
		default:
		{
			Close();
			break;
		}
	}
}

//
//
//
void GnutellaSocket::SendPing(unsigned char ttl)
{
//	OutputDebugString("GnutellaSocket::SendPing()\n");

	m_time_last_ping_sent=CTime::GetCurrentTime();

	GnutellaHeader hdr;
	hdr.Op(GnutellaOpCode::Ping);
	hdr.Length(0);

	// Reset TTL
	hdr.TTL(ttl);

	SendSocketData(&hdr,sizeof(GnutellaHeader));
}

//
//
//
void GnutellaSocket::SendPong(unsigned char ttl)
{
//	OutputDebugString("Send Pong\n");

	// If the number of hops == 0, then send back a pong so that the dude we are connected to doesn't drop us
	char buf[sizeof(GnutellaHeader)+sizeof(PongData)];
	GnutellaHeader *hdr=(GnutellaHeader *)&buf[0];
	PongData *data=(PongData *)&buf[sizeof(GnutellaHeader)];

	// Send the Pong back with the same GUID as the Ping that was received.
	hdr->Init();
	hdr->Guid(m_hdr.Guid());
	hdr->Op(GnutellaOpCode::Pong);
	hdr->Length(sizeof(PongData));

	/* Skycat */
	hdr->TTL(ttl);
	hdr->Hops(0);
	/* Skycat */

/*
	// Randomly use one of the IPs we've seen...and change the last two digits.
	int ip=p_sockets->ReturnRandomIP();
	ip=(ip&0x0000FFFF)+((rand()%256)<<24)+((rand()%256)<<16);
*/
	int ip=192;	// say we are 192.0.x.x (behind a firewall)
	ip=(ip&0x0000FFFF)+((rand()%256)<<24)+((rand()%256)<<16);

	char ip_buf[256];
	sprintf(ip_buf,"%u.%u.%u.%u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);

	// Say that we are sharing a random amount of stuff
	unsigned int shared_files=rand()%100+1;	// 1 to 100
	unsigned int shared_kb=shared_files*(3*1024+(rand()%(7*1024))); // # files * 3 to 7 MB each

	data->Init(6346,ip_buf,shared_files,shared_kb);

	SendSocketData(buf,sizeof(GnutellaHeader)+hdr->Length());
}

//
//
//
void GnutellaSocket::ProcessPong(char *data)
{
	PongData pong;
	memcpy(&pong,data,sizeof(PongData));

	GnutellaHost host;
	host.Host((char *)pong.IPString().c_str());
	host.Port(pong.Port());

	vector<GnutellaHost> hosts;
	hosts.push_back(host);
	p_sockets->ReportHosts(hosts);
}
/*
//
//
//
GnutellaHostConnectionStatus GnutellaSocket::ReturnHostConnectionStatus()
{
	GnutellaHostConnectionStatus ret=m_host_connection_status;
	m_host_connection_status.ClearCounters();
	return ret;
}
*/
//
//
//
void GnutellaSocket::TimerHasFired()
{
	// Check to see if we are even a socket
	if(IsSocket()==false)
	{
		return;
	}

	// If we haven't fully connected within 30 seconds, this connection has timed out
	if(m_received_gnutella_connection_string==false)
	{
		if((CTime::GetCurrentTime()-m_status_data.m_connect_time).GetTotalSeconds()>30)
		{
//			OutputDebugString("*** TIMEOUT ***\n");
			Close();
			return;
		}
	}

	// If we've sent the QRP table (we are totally connected) and it's been 30 seconds since we last sent a ping, do it now
	if(m_qrp_table_sent)
	{
		if((CTime::GetCurrentTime()-m_time_last_ping_sent).GetTotalSeconds()>=30)
		{
			/* Skycat */
			//SendPing(1);
			SendPing(2);
		}
	}

	// Check to see if we need to send some cached query hits
	int limit=75+rand()%(50+1);	// 75-125
	int num_sent=0;
	while((v_cached_query_hits_to_send.size()>0) && (num_sent<limit))
	{
		unsigned int buf_len=*((unsigned int *)v_cached_query_hits_to_send[0]);
		char *buf=v_cached_query_hits_to_send[0]+sizeof(unsigned int);

		SendSocketData(buf,buf_len);

		// Check to see if something went bad in the send, the socket was closed, and the vector was cleared
		if(IsSocket()==false)
		{
			return;
		}

		delete *v_cached_query_hits_to_send.begin();
		v_cached_query_hits_to_send.erase(v_cached_query_hits_to_send.begin());

		num_sent++;
	}

	// Add the query count to the query counts vector and clear the counter, limiting the vector to 5 entries
	v_query_counts.push_back(m_query_count);
	while(v_query_counts.size()>5)
	{
		v_query_counts.erase(v_query_counts.begin());
	}
	m_query_count=0;
}

//
//
//
unsigned int GnutellaSocket::ReturnHostIP()
{
	return m_status_data.m_host.IP();
}

//
//
//
void GnutellaSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
//	int i;

	// Make sure this is a fully connected socket
	if((IsSocket()==false) || (m_received_gnutella_connection_string==false))
	{
		return;
	}
/*
	// Find the search string and project keyword index for this project
	string query;
	for(i=0;i<p_sockets->v_keywords.size();i++)
	{
		if(strcmp(p_sockets->v_keywords[i].m_project_name.c_str(),psq.Project())==0)
		{
			query=p_sockets->v_keywords[i].m_supply_keywords.m_search_string;
			break;
		}
	}

	// Make sure we found the project
	if(query.size()==0)
	{
		return;
	}
*/
	// Search queries now have a "urn:'\0'" after the null-termination of the search string if it doesn't know what urn to search on
	unsigned int buf_len=sizeof(GnutellaHeader)+sizeof(unsigned short int)+psq.m_search_string.size()+1+strlen("urn:")+1;
	char *buf=new char[buf_len];
	memset(buf,0,buf_len);

	GnutellaHeader hdr;
	hdr.Guid(psq.m_guid);	// reset the header's GUID
	hdr.Op(GnutellaOpCode::Query);
	hdr.Length(buf_len-sizeof(GnutellaHeader));

	memcpy(buf,&hdr,sizeof(GnutellaHeader));

	char *ptr=&buf[sizeof(GnutellaHeader)+sizeof(unsigned short int)];
	strcpy(ptr,psq.m_search_string.c_str());
	ptr+=psq.m_search_string.size()+1;	// +1 for NULL
	strcpy(ptr,"urn:");

	// Send this out immediately. If there are any cached send buffers, put it at the beginning of the vector. Else just send normally
	if(v_send_data_buffers.size()>0)
	{
		WSocketSendData *send_data=new WSocketSendData((unsigned char *)buf,buf_len);	// Create a new send data object
		v_send_data_buffers.insert(v_send_data_buffers.begin(),send_data);	// and add it to the vector at the beginning
	}
	else
	{
		SendSocketData(buf,buf_len);
	}

	delete [] buf;
}

//
//
//
void GnutellaSocket::ProcessQueryHit(char *data)
{
	UINT i;
	//bool is_spoof_dist_poison=false;

	// Find the project supply query that is related to this GUID
	ProjectSupplyQuery *psq=NULL;
	for(i=0;i<p_sockets->v_project_supply_queries.size();i++)
	{
		if(p_sockets->v_project_supply_queries[i].m_guid==m_hdr.Guid())
		{
			psq=&p_sockets->v_project_supply_queries[i];
			break;
		}
	}

	// If it is still equal to NULL, then return
	if(psq==NULL)
	{
//		OutputDebugString("Couldn't find GUID for this query in GnutellaSocket::ProcessQueryHit\n");
		return;
	}

	if(!psq->m_audio_project)
		// Check the private data of this query hit to see if it is a spoof, dist, or poison
		if(QueryHit::IsSpoofDistPoison(data,m_hdr.Length()))
			return;

	// Find the host connection status data for this project
	ProjectStatus *status=&p_sockets->m_status_data.v_project_status[psq->m_project_status_index];
//	unsigned int *query_hit_count=&p_sockets->m_status_data.v_project_status[psq->m_project_status_index].m_query_hit_count;
	vector<QueryHit> *query_hits=&p_sockets->m_status_data.v_project_status[psq->m_project_status_index].v_query_hits;
/*
	for(i=0;i<m_host_connection_status.v_project_status.size();i++)
	{
		if(strcmp(m_host_connection_status.v_project_status[i].m_project_name.c_str(),psq->m_project.c_str())==0)
		{
			query_hit_count=&m_host_connection_status.v_project_status[i].m_query_hit_count;
		}
	}

	// If I couldn't find the project status for this project, return
	if(query_hit_count==NULL)
	{
//		OutputDebugString("Couldn't find project status for this project in GnutellaSocket::ProcessQueryHit\n");
		return;
	}
*/
	// Process the query
	char *ptr=&data[0];
	unsigned char *num_hits=(unsigned char *)ptr;
	ptr+=sizeof(unsigned char);
	unsigned short int *port=(unsigned short int *)ptr;
	ptr+=sizeof(unsigned short int);
	unsigned int *ip=(unsigned int *)ptr;
	ptr+=sizeof(unsigned int);
	unsigned int *speed=(unsigned int *)ptr;
	ptr+=sizeof(unsigned int);
/*
	char msg[1024];
	sprintf(msg,"Num Hits : %u\n",*num_hits);
	OutputDebugString(msg);
*/
	unsigned int data_len=m_hdr.Length();
	//debug
/*
	if(data_len >= 5000)
		TRACE("QueryHit data length = %d, number of hits = %d\n", data_len, *num_hits);
*/
	// Result set
	for(i=0;i<*num_hits;i++)
	{
		unsigned int *file_index=(unsigned int *)ptr;
		ptr+=sizeof(unsigned int);
		unsigned int *file_size=(unsigned int *)ptr;
		ptr+=sizeof(unsigned int);

		// Check that we haven't gone beyond the bounds of the data
		if((UINT)(ptr-data)>data_len){return;}		
		char *filename=ptr;
		// Check to see that the string is valid
		if(IsStringValid(filename)==false){return;}		
		ptr+=strlen(filename)+1;

		// Check that we haven't gone beyond the bounds of the data
		if((UINT)(ptr-data)>data_len){return;}		
		char *info=ptr;
		// Check to see that the string is valid
		if(IsStringValid(info)==false){return;}
		ptr+=strlen(info)+1;

		// *&* HERE If the info's last character is a space, then this reply is from us...so return if this is not a searcher query 
/*		if(psq->m_is_searcher_query==false)
		{
			if(is_spoof_dist_poison)
			{
				return;
			}

/*
			if(strlen(info)>0)
			{
				if(info[strlen(info)-1]==' ')
				{
					return;
				}
			}

		}
*/
		// Confirm the lower-cased filename with any required keywords
		char *lfilename=new char[strlen(filename)+1];
		strcpy(lfilename,filename);
		strlwr(lfilename);
		int track=CheckQueryHitRequiredKeywords(psq,lfilename,*file_size);
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
			qh.m_guid=psq->m_guid;
			qh.m_port=*port;
			qh.m_ip=*ip;
			qh.m_speed=*speed;
			qh.m_file_index=*file_index;
			qh.m_file_size=*file_size;
			qh.Filename(filename);
			//qh.Info(info);
			//qh.m_is_searcher_query_hit=psq->m_is_searcher_query;
			//if(is_spoof_dist_poison)
			//	qh.m_is_spoof_dist_poison=is_spoof_dist_poison;
			//else
			//	qh.m_is_spoof_dist_poison=qh.IsSwarm();
			qh.m_track=track;
			qh.m_project = psq->m_project.c_str();
			qh.m_project_id = psq->m_project_id;
			qh.m_timestamp = CTime::GetCurrentTime();
	
			//extract sha1 from this file
			char* p_sha1 = strstr(info, "urn:sha1:");
			if(p_sha1 != NULL)
			{
				p_sha1+=strlen("urn:sha1:");
				strncpy(qh.m_sha1, p_sha1, 32);
				qh.m_sha1[32] = '\0';
			}
			query_hits->push_back(qh);

		}
		delete [] lfilename;
	}

	// Trailer
	//char vendor_code[4];
	//memcpy(vendor_code,ptr,sizeof(vendor_code));
	//ptr+=sizeof(vendor_code);
	//unsigned char *open_data_size=(unsigned char *)ptr;
	//ptr+=sizeof(unsigned char);
	//unsigned short int *open_data=(unsigned short int *)ptr;
	//ptr+=sizeof(unsigned short int);
}

//
//
//
bool GnutellaSocket::IsConnected()
{
	if(IsSocket())
	{
		if(m_received_gnutella_connection_string)
		{
			return true;
		}
	}

	return false;
}

//
//
//
void GnutellaSocket::SendQRPTable()
{
	// Send the QRP RESET
	char reset_buf[sizeof(GnutellaHeader)+sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned char)];	// hdr+variant+table_len+infinity
	char *ptr=reset_buf;
	GnutellaHeader *hdr=(GnutellaHeader *)ptr;
	hdr->Init();
	hdr->Op(GnutellaOpCode::QRPTableUpdate);
	hdr->TTL(1);
	hdr->Length(sizeof(reset_buf)-sizeof(GnutellaHeader));
	ptr+=sizeof(GnutellaHeader);
	*ptr=0;	// reset
	ptr++;
	unsigned int *table_len=(unsigned int *)ptr;
	*table_len=65536;	// 64k
	ptr+=sizeof(unsigned int);
	*ptr=2;	// infinity
	SendSocketData(reset_buf,sizeof(reset_buf));
/*	
	// Create QRP table and send the PATCH
	char qrp[65536/2];	// divided by 2 because 4 bits per entry
	memset(qrp,0,sizeof(qrp));

	// Add all of the project's keywords to the patch
	for(i=0;i<p_sockets->v_keywords.size();i++)
	{
		// Query Keywords
		for(j=0;j<p_sockets->v_keywords[i].m_query_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_sockets->v_keywords[i].m_query_keywords.v_keywords[j].keyword.c_str(),qrp);
		}

		// Exact Query Keywords
		for(j=0;j<p_sockets->v_keywords[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_sockets->v_keywords[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),qrp);
		}

		// Supply Keywords
		for(j=0;j<p_sockets->v_keywords[i].m_supply_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_sockets->v_keywords[i].m_supply_keywords.v_keywords[j].keyword.c_str(),qrp);
		}

		for(j=0;j<p_sockets->v_keywords[i].m_qrp_keywords.v_keywords.size();j++)
		{
			QRP::AddUpTo3KeywordsToTable((char *)p_sockets->v_keywords[i].m_qrp_keywords.v_keywords[j].c_str(),qrp);
		}
	}

	// Compress QRP table
	CompressionModule mod;
	mod.CompressBuffer((unsigned char *)qrp,sizeof(qrp));
	unsigned int cbuf_len=mod.ReturnCompressedBufferLength();
	char *cbuf=new char[cbuf_len];
	memcpy(cbuf,mod.ReturnCompressedBuffer(),cbuf_len);
*/
	// Send the QRP Table patch
	unsigned int buf_len=sizeof(GnutellaHeader)+sizeof(unsigned char)*5+p_sockets->m_compressed_qrp_table.GetLen();
	char *buf=new char[buf_len];
	ptr=buf;
	hdr=(GnutellaHeader *)ptr;
	hdr->Init();
	hdr->Op(GnutellaOpCode::QRPTableUpdate);
	hdr->TTL(1);
	hdr->Length(buf_len-sizeof(GnutellaHeader));
	ptr+=sizeof(GnutellaHeader);
	*(ptr++)=1;	// patch
	*(ptr++)=1;	// seq number
	*(ptr++)=1;	// seq size
	*(ptr++)=1;	// compressor - zlib
	*(ptr++)=8;	// bits per entry (either 4 or 8)

	memcpy(ptr,p_sockets->m_compressed_qrp_table.GetData(),p_sockets->m_compressed_qrp_table.GetLen());
	
	SendSocketData(buf,buf_len);

	delete [] buf;

	// Now that we've sent the QRP table, we can initialize the project status counters, since we'll be getting queries.
	m_qrp_table_sent=true;
//	m_host_connection_status.UpdateProjects(p_sockets->v_keywords);
}

//
//
//
void GnutellaSocket::ProcessQuery(char *query)
{
	UINT i,j,k;
	bool seen_this_guid_before=false;

// Check to see if we've checked to see if we've seen this GUID before
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
			/* =>_<=
			// Only return if this is not a searcher query...we wanna be hit by ourselves on searcher queries
			if(p_sockets->v_project_supply_queries[j].m_is_searcher_query==false)
			{
				return;
			}
			else
			{
				seen_this_guid_before=true;	// if it is a searcher query, don't count it towards demand
			}
			*/
		}
	}

	// First make sure that the query is a valid null-terminated string
	if(IsStringValid(query)==false)
	{
		return;
	}


	//bool checked=false;				// have we have checked to see if we should respond to this GUID

	// Extract the keywords from the query
	vector<string> keywords;
	ExtractKeywordsFromQuery(&keywords,query);

	CString search_string;
	// Check all of the keywords and exact keywords of all of the projects
	for(i=0;i<p_sockets->v_keywords.size();i++)
	{
		ProjectKeywords *pk=&p_sockets->v_keywords[i];
		if(search_string.GetLength()>0)
		{
			if(search_string.CompareNoCase(ExtractFirstWord(p_sockets->v_keywords[i].m_supply_keywords.m_search_string.c_str()))!=0)
				break;
		}

		if(pk->m_gnutella_demand_enabled)
		{

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
				// Find the appropriate track (if any) this query is looking for
				char *lquery=new char[strlen(query)+1];
				strcpy(lquery,query);
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

				/*
				//try to search for the next project if track = 0 until search string is different
				if( (track == 0) && (i+1 < p_sockets->v_keywords.size()) )
				{
					bool more_project = true;
					bool found_next_match_project = false;
					int index = 1;
					int this_track=0;
					while(more_project)
					{
						if( (i+index) < p_sockets->v_keywords.size() )
						{
							if(strcmp(p_sockets->v_keywords[i+index].m_supply_keywords.m_search_string.c_str(), 
								p_sockets->v_keywords[i].m_supply_keywords.m_search_string.c_str())!=NULL)
								break;
							else //same search string project, needs to find out the track number
							{
								char *this_lquery=new char[strlen(query)+1];
								strcpy(this_lquery,query);
								strlwr(this_lquery);
								this_track=0;
								for(j=0;j<p_sockets->v_keywords[i+index].m_supply_keywords.v_keywords.size();j++)
								{
									bool found_it=true;
									for(k=0;k<p_sockets->v_keywords[i+index].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
									{
										if(strstr(this_lquery,p_sockets->v_keywords[i+index].m_supply_keywords.v_keywords[j].v_keywords[k])==NULL)
										{
											found_it=false;
											break;
										}
									}
									
									if(found_it)
									{
					//					track=j+1;	// +1 b/c the track number is 1+keyword index, and track=0 is it didn't match any track's keywords WRONG!!!
										this_track=p_sockets->v_keywords[i+index].m_supply_keywords.v_keywords[j].m_track;
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
							break;
					}
					if(found_next_match_project)
					{
						pk = &p_sockets->v_keywords[i+index];
						track = this_track;
						i=i+index;
					}
				}
				*/

				// If we haven't seen this GUID before, then increment the query counters
				if(seen_this_guid_before==false)
				{
	//				p_sockets->m_status_data.v_project_status[i].m_query_count++;
					p_sockets->m_status_data.v_project_status[i].IncrementTrackQueryCount(track);
				}

				// Find the SupplyProject for this project
//				for(j=0;j<p_sockets->v_supply.size();j++)
//				{
//					if(strcmp(p_sockets->v_supply[j].m_name.c_str(),pk->m_project_name.c_str())==0)
//					{
						ProjectStatus *status=&p_sockets->m_status_data.v_project_status[i];

						// Save this query to be reported back to the connection manager
						Query qu;
						qu.m_guid=m_hdr.Guid();
						qu.m_query=query;
	/* =>_<= no spoofing or disting
						qu.m_spoofs_sent=Spoof(pk,p_sockets->v_supply[j],status);
						qu.m_dists_sent=Dist(pk,p_sockets->v_supply[j],status);
	=>_<= */
						qu.m_hops=m_hdr.Hops();
						qu.m_ttl=m_hdr.TTL();
						qu.m_ip=m_status_data.m_host.IP();
						qu.m_track=track;
						
						qu.m_timestamp = CTime::GetCurrentTime();
						qu.m_project = status->ProjectName();
						qu.m_project_id = pk->m_id;

						status->v_queries.push_back(qu);
						
						if(track > 0)
							break;
						else
							search_string=ExtractFirstWord(p_sockets->v_keywords[i].m_supply_keywords.m_search_string.c_str());
//					}
//				}
			}
		}
	}
}

//
//
//
void GnutellaSocket::ExtractKeywordsFromQuery(vector<string> *keywords,char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	//strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	cstring.MakeLower();
	//string cstring_string=cstring;
	strcpy(lbuf,cstring);

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
// Replaces all characters that are not letters or numbers with spaces in place
//
void GnutellaSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
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
void GnutellaSocket::KeywordsUpdated()
{
//	int i;

	// If I have already sent the QRP, then need to resend it.  If I haven't sent it yet, then I will send it in a bit.
	if(m_qrp_table_sent)
	{
		SendQRPTable();
	}
/*
	// Make sure all of the project keyword data of the project supply queries are still correct
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		bool found=false;
		for(i=0;i<p_sockets->v_keywords.size();i++)
		{
			if(strcmp(psq_iter->m_project.c_str(),p_sockets->v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&p_sockets->v_keywords[i]);
				break;
			}
		}

		// If not found, then this project no longer exists, so remove the query from the vector
		if(!found)
		{
			v_project_supply_queries.erase(psq_iter);
			psq_iter=v_project_supply_queries.begin();
		}
		else
		{
			psq_iter++;
		}
	}
*/
}

//
// Return Values (Track)
// ---------------------
// -1 : query hit is not a match
//  0 : query hit is a match but there are no required keywords, so it does not correspond with any track number
// >0 : query hit is a match and is associated with the specified track number
//
int GnutellaSocket::CheckQueryHitRequiredKeywords(ProjectSupplyQuery *psq,char *lfilename,unsigned int file_size)
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
unsigned int GnutellaSocket::UpTime()
{
	// If we haven't fully connected yet, then we aren't even "up"
	if(m_received_gnutella_connection_string==false)
	{
		return 0;
	}

	return (UINT)(CTime::GetCurrentTime()-m_status_data.m_up_time).GetTotalSeconds();
}

//
// Scrambles the bits of a 32 bit value, except for the MSB so that the case is unchanged
//
unsigned int GnutellaSocket::BitScramble(unsigned int val)
{
	unsigned int ret=0;

	ret|=((val>> 15 )&0x0001)<<0;
	ret|=((val>> 29 )&0x0001)<<1;
	ret|=((val>> 6  )&0x0001)<<2;
	ret|=((val>> 11 )&0x0001)<<3;
	ret|=((val>> 7  )&0x0001)<<4;
	ret|=((val>> 21 )&0x0001)<<5;
	ret|=((val>> 23 )&0x0001)<<6;
	ret|=((val>> 2  )&0x0001)<<7;
	ret|=((val>> 30 )&0x0001)<<8;
	ret|=((val>> 14 )&0x0001)<<9;
	ret|=((val>> 26 )&0x0001)<<10;
	ret|=((val>> 18 )&0x0001)<<11;
	ret|=((val>> 0  )&0x0001)<<12;
	ret|=((val>> 27 )&0x0001)<<13;
	ret|=((val>> 12 )&0x0001)<<14;
	ret|=((val>> 10 )&0x0001)<<15;
	ret|=((val>> 22 )&0x0001)<<16;
	ret|=((val>> 28 )&0x0001)<<17;
	ret|=((val>> 1  )&0x0001)<<18;
	ret|=((val>> 25 )&0x0001)<<19;
	ret|=((val>> 5  )&0x0001)<<20;
	ret|=((val>> 9  )&0x0001)<<21;
	ret|=((val>> 20 )&0x0001)<<22;
	ret|=((val>> 13 )&0x0001)<<23;
	ret|=((val>> 8  )&0x0001)<<24;
	ret|=((val>> 19 )&0x0001)<<25;
	ret|=((val>> 17 )&0x0001)<<26;
	ret|=((val>> 4  )&0x0001)<<27;
	ret|=((val>> 24 )&0x0001)<<28;
	ret|=((val>> 16 )&0x0001)<<29;
	ret|=((val>> 3  )&0x0001)<<30;

	return ret;
}

//
//
//
unsigned int GnutellaSocket::ReturnCachedQueryHitCount()
{
	return (UINT)v_cached_query_hits_to_send.size();
}

//
//
//
unsigned int GnutellaSocket::ReturnQueryCount()
{
	UINT i;

	// returns the sum of the query counts to determine whether or not i should get searcher queries
	unsigned int ret=0;
	for(i=0;i<v_query_counts.size();i++)
	{
		ret+=v_query_counts[i];
	}

	return ret;
}

//
//
//
bool GnutellaSocket::IsStringValid(char *ptr)
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
//
//
string GnutellaSocket::ReturnRemoteVendor()
{
	return m_status_data.m_remote_vendor;
}

//
//
//
CString GnutellaSocket::ExtractFirstWord(const char* query)
{
	char* buf=new char[strlen(query)+1];
	sscanf(query,"%s",buf);
	CString word=buf;
	delete [] buf;
	return word;
}