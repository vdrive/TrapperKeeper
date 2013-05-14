// GnutellaSocket.cpp

#include "stdafx.h"
#include "GnutellaSocket.h"
#include "PongData.h"
#include "PushData.h"
#include "ConnectionSockets.h"
#include "GUIDCache.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "sha.h"
#include "ConnectionManager.h"
//#include "BSA.h"
#include "iphlpapi.h"

#define MAX_SEND_BUFFERS_ALLOWED	500		// was 1000
#define MAX_CACHED_QUERY_HITS		10000

//
//
//
GnutellaSocket::GnutellaSocket()
{
	srand((unsigned)time(NULL));
	ResetData();

	char iai_buf[2048];
	DWORD iai_buf_len=2048;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;
	DWORD ret = GetAdaptersInfo(iai,&iai_buf_len);
	m_my_ip = (const char *)iai->IpAddressList.IpAddress.String;
	m_my_ip_int = inet_addr(m_my_ip);
}

//
//
//
GnutellaSocket::~GnutellaSocket()
{
	ResetData();
	// Clean up compression
	//inflateEnd(&InflateStream);
	//deflateEnd(&DeflateStream);
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
	
	//m_connecting_to_limewire_reflector=false;

	m_on_receive_cached=false;

	m_status_data.Clear();
//	m_host_connection_status.Clear();

	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		for(i=0;i<v_cached_query_hits_to_send.size();i++)
		{
			delete [] v_cached_query_hits_to_send[i];
		}
		v_cached_query_hits_to_send.clear();

		singleLock.Unlock();
	}
	//m_query_count=0;
//	v_query_counts.clear();
	m_state=0;
	m_handshake.Empty();

	// Compression
//#ifdef COMPRESSION_ENABLED
//	m_dnapressionOn = true;
//#else
//	m_dnapressionOn = false;
//#endif
//	m_InflateRecv   = false;
//	m_DeflateSend   = false;
//
//	InflateStream.zalloc = Z_NULL;
//	InflateStream.zfree  = Z_NULL;
//	InflateStream.opaque = Z_NULL;
//	InflateStream.state  = Z_NULL;
//
//	DeflateStream.zalloc   = Z_NULL;
//	DeflateStream.zfree    = Z_NULL;
//	DeflateStream.opaque   = Z_NULL;
//	DeflateStream.state    = Z_NULL;
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

	if(error_code!=0)
	{
		Close();
//		TRACE("0x%08x - GnutellaSocket::OnConnect() - Error: %d\n",this,error_code);
		return;
	}

	m_state=10;

	string str;
#ifdef LEAF

	if(rand()%2==0) // 50% chance connects as Limewire and 50% chance connects at Morpheus
	{
		//Send LimeWire CONNECT
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="X-Max-TTL: 4\r\n";
		str+="X-Dynamic-Querying: 0.1\r\n";
		//str+="X-Version: 4.0.4\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="User-Agent: LimeWire/4.0.5\r\n";
		//str+="Vendor-Message: 0.1\r\n";
		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
		//str+="GGEP: 0.5\r\n";
		str+="Listen-IP: ";
		str+=m_my_ip;
		str+=":6346\r\n";
		//str+="Pong-Caching: 0.1\r\n";
		//str+="X-Guess: 0.1\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="X-Degree: 32\r\n";
		str+="X-Locale-Pref: en\r\n";
		char remote_ip[32];
		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
		str+=remote_ip;
		str+="X-Ultrapeer-Needed: True\r\n";
		str+="\r\n";
	}
	else
	{
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="X-Ultrapeer: False\r\n";
		str+="User-Agent: Morpheus 4.0.53.212 (GnucDNA 1.0.2.6)\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="X-Max-TTL: 4\r\n";
		str+="X-Ultrapeer-Query-Routing: 0.1\r\n";
		str+="X-Dynamic-Querying: 0.1\r\n";
		//str+="Pong-Caching: 0.1\r\n";
		//str+="X-Guess: 0.1\r\n";
		str+="X-Degree: 32\r\n";
		//str+="X-Locale-Pref: en\r\n";
		str+="X-Ultrapeer-Needed: True\r\n";
		//str+="GGEP: 0.5\r\n";
		
		char remote_ip[32];
		sprintf(remote_ip,"Remote-IP: %s\r\n",m_status_data.m_host.Host().c_str());
		str+=remote_ip;
		
		str+="\r\n";
	}
#else

#ifdef ULTRAPEER
		str="GNUTELLA CONNECT/0.6\r\n";
		str+="User-Agent: Gnucleus 2.0.1.0(GnucDNA 1.0.2.6)\r\n";
		str+="X-Ultrapeer: True\r\n";
		str+="X-Leaf-Max: 400\r\n";
		str+="X-Query-Routing: 0.1\r\n";	// QRP
		str+="Uptime: 0D 00H 00M\r\n";

		str+="\r\n";
#endif
#endif

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
	
	// Check to see if we need to cache this OnReceive and send some data buffers out
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

	m_state=21;


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

	m_handshake = data;

	m_state=23;
	
	// Extract the User-Agent:
	m_status_data.m_remote_vendor="???";
	if(strstr(data,"User-Agent: ")!=NULL)
	{
		char *user_agent=new char[strlen(strstr(data,"User-Agent: "))+1];
		strcpy(user_agent,strstr(data,"User-Agent: "));

#ifdef BEARSHARE_ONLY
		//connects to BearShare Client only
		if( (strstr(user_agent,"BearShare")==NULL))
		//drop all bearshare connections
		//if( (strstr(user_agent,"BearShare")!=NULL)/*|| (strstr(user_agent,"LimeWire")==NULL)*/)
		{
			delete [] user_agent;
			Close();
			return;
		}
#else
		if(strstr(user_agent,"BearShare")!=NULL)
		{
			//if( (rand()%10) != 0)
			//{
				delete [] user_agent;
				Close();
				return;
			//}
		}
#endif
#ifdef NO_IMESH
		if(strstr(user_agent,"iMesh")!=NULL)
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
	string ok="GNUTELLA/0.6 200 OK\r\n";

	ok+="\r\n";
    
	SendSocketData((char*)ok.c_str(),ok.length());
	//SendSocketData("GNUTELLA/0.6 200 OK\r\nAccept-Encoding: deflate\r\n\r\n",(UINT)strlen("GNUTELLA/0.6 200 OK\r\nAccept-Encoding: deflate\r\n\r\n"));

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
			if(m_hdr.Length()!=sizeof(PongData))
			{
				TRACE("Pong: header lenght is %d, != to PongData size: %d, closing socket\n",m_hdr.Length(),sizeof(PongData));
				Close();
				return;
			}

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
int GnutellaSocket::ReceivedDeflatedGnutellaHeader()
{
	switch(m_hdr.Op())
	{
		case GnutellaOpCode::Ping:
		{
			m_state=30;

			if(m_hdr.Hops()<3)
			{
				SendPong(m_hdr.Hops());
			}
			else
			{
				SendPong(3);
			}

			// No data to received
			m_received_gnutella_header=false;
			p_sockets->m_status_data.m_ping_count++;
			return m_hdr.Length();
		}
		case GnutellaOpCode::Pong:
		{
			m_state=31;

			// With GGEP, there may be some GGEP extension data after the pong data
			if(m_hdr.Length()!=sizeof(PongData))
			{
				TRACE("Pong: header lenght is %d, != to PongData size: %d, closing socket\n",m_hdr.Length(),sizeof(PongData));
				Close();
				return -1;
			}
			return m_hdr.Length();
		}
		case GnutellaOpCode::QRPTableUpdate:
		{
			m_state=32;

			// Check for data length validity
			if((m_hdr.Length()>PACKET_BUFF-sizeof(GnutellaHeader))||(m_hdr.Length()<6))
			{
				TRACE("QRPTableUpdate: header lenght is %d, which > 32745 or < 6,  closing socket\n",m_hdr.Length());
				Close();
				return -1;
			}

			return m_hdr.Length();

		}
		case GnutellaOpCode::Push:
		{
			m_state=33;

			// With GGEP, there may be some GGEP extension data after the push data
			if( m_hdr.Length() != sizeof(PushData))
			{
				TRACE("PushData: header lenght is %d, !=  sizeof(PushData) = %d, closing socket\n",m_hdr.Length(), sizeof(PushData));
				Close();
				return -1;
			}

			return m_hdr.Length();
		}
		case GnutellaOpCode::Query:
		{
			m_state=34;

			// Check for data length validity
			if((m_hdr.Length()>1000)||(m_hdr.Length()==0))
			{
				TRACE("Query: header lenght is %d, which is > 1000 or == 0, closing socket\n",m_hdr.Length());
				Close();
				return -1;
			}

			return m_hdr.Length();
		}
		case GnutellaOpCode::QueryHit:
		{
			m_state=35;

			// Check for data length validity
			if((m_hdr.Length()>500000)||(m_hdr.Length()==0))
			{
				TRACE("QueryHit: header lenght is %d, which is > 500000 or == 0, closing socket\n",m_hdr.Length());
				Close();
				return -1;
			}
		
			return m_hdr.Length();
		}
		default:
		{
			m_state=36;
			//Close();
			TRACE("Received Unknown Op Code : %u - Len : %u - Hops : %u - TTL : %u\n",m_hdr.Op(),m_hdr.Length(),m_hdr.Hops(),m_hdr.TTL());
			return m_hdr.Length();
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
//			OutputDebugString("Query Data\n");
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
				//xml=new char[m_hdr.Length()];
				//ZeroMemory(xml,m_hdr.Length());
				//memcpy(xml,&data[sizeof(unsigned short int)+strlen(query)+1],m_hdr.Length()-sizeof(unsigned short int)-strlen(query)-1);
				//char* end_of_xml=strstr(xml,"urn:");
				//if(end_of_xml!=NULL)
				//{
				//	end_of_xml+=4;
				//	*end_of_xml='\0';
				//}
				//else
				//{
				//	xml[m_hdr.Length()-1]='\0';
				//}
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
				//looking for album tag
				char* album=strstr(xml,"album=\"");
				if(album!=NULL)
				{
					if(strlen(album) > 7)
					{
						album+=7;
						char *end = strchr(album,'\"');
						if(end!=NULL)
						{
							*end='\0';
							strcat(query_xml, " ");
							strcat(query_xml, album);
						}
					}
				}
				//looking for title tag
				//char* title=strstr(xml,"title=\"");
				//if(title!=NULL)
				//{
				//	if(strlen(title) > 7)
				//	{
				//		title+=7;
				//		char *end = strchr(title,'\"');
				//		if(end!=NULL)
				//		{
				//			*end='\0';
				//			if(strstr(title,"&apos")!=NULL)
				//			{
				//				strcat(query_xml, " ");
				//				strcat(query_xml, title);
				//			}
				//		}
				//	}
				//}
				//delete [] xml;
			}
#ifdef _DEBUG
			if(strstr(query_xml,"\'")!=NULL)
				TRACE("An apostrophe found - %s\nXML - %s\n",query_xml,xml);
#endif
			ProcessQuery(query_xml);
			delete [] query_xml;
			break;
		}
		case GnutellaOpCode::QueryHit:
		{
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

	/* Skycat 
	hdr->TTL(ttl);
	hdr->Hops(0);
	Skycat */

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

	// If we haven't fully connected within 60 seconds, this connection has timed out
	if(m_received_gnutella_connection_string==false)
	{
		if((CTime::GetCurrentTime()-m_status_data.m_connect_time).GetTotalSeconds()>60)
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
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		while((v_cached_query_hits_to_send.size()>0) && (num_sent<limit))
		{
			unsigned int buf_len=*((unsigned int *)v_cached_query_hits_to_send[0]);
			char *buf=v_cached_query_hits_to_send[0]+sizeof(unsigned int);

			SendSocketData(buf,buf_len);
			delete [] *v_cached_query_hits_to_send.begin();
			v_cached_query_hits_to_send.erase(v_cached_query_hits_to_send.begin());

			// Check to see if something went bad in the send, the socket was closed, and the vector was cleared
			if(IsSocket()==false)
			{
				singleLock.Unlock();
				return;
			}
			num_sent++;
		}
		singleLock.Unlock();
	}

	// Add the query count to the query counts vector and clear the counter, limiting the vector to 5 entries
/*	v_query_counts.push_back(m_query_count);
	while(v_query_counts.size()>5)
	{
		v_query_counts.erase(v_query_counts.begin());
	}
	m_query_count=0;
	*/
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
void GnutellaSocket::SendSpoofs(FalseQueryHit &hit)
{
	int buf_len=sizeof(GnutellaHeader)+hit.GetBufferLength();
	char *buf=new char[buf_len];

	GnutellaHeader *hdr=(GnutellaHeader *)buf;
	hdr->Init();
	hdr->Guid(m_hdr.Guid());
	hdr->Op(GnutellaOpCode::QueryHit);
	hdr->Length(hit.GetBufferLength());
	int ttl= m_hdr.Hops() + 2;
	hdr->TTL(ttl);

	hit.WriteToBuffer(&buf[sizeof(GnutellaHeader)]);
	SendSocketData(buf,buf_len);
	delete [] buf;
}

//
//
//
void GnutellaSocket::ProcessQuery(char *query)
{
	//ignore the query if we are assigning the project supply vector to the newest one (causing the crash otherwise)
	//if(p_sockets->IsProjectSupplyBeingUpdated())
	//	return;

	bool seen_this_guid_before=false;
	TRACE("Query: %s\n",query);

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

	// First make sure that the query is a valid null-terminated string
	if(IsStringValid(query)==false)
	{
		return;
	}

	UINT h,i,j,k;

	
	// Extract the keywords from the query
	vector<string> keywords;
	ExtractKeywordsFromQuery(&keywords,query);
	for(h=0;h<keywords.size();h++)
	{
		for(i=0;i<p_sockets->v_share_files.size();i++)
		{
			for(j=0;j<p_sockets->v_share_files[i].v_keywords.size();j++)
			{
				if(keywords[h]==p_sockets->v_share_files[i].v_keywords[j])
				{
					FalseQueryHit hit;
					QueryHitResult result = p_sockets->v_share_files[i];
					CString sha1="urn:sha1:";
					sha1+=result.m_info.c_str();
					result.m_info=sha1;
					hit.v_results.push_back(result);
					hit.m_hdr.NumberOfHits(1);
					hit.m_hdr.Port(6346);
					hit.m_hdr.IP(m_my_ip_int);
					hit.m_hdr.Speed(20000+rand()%20000);	// 20,000 - 40,000
					if(hit.v_results[0].m_type!=0)
						hit.InitTrailer();
					else
						hit.InitSwarmTrailer(hit.v_results[0].m_artist.c_str(), hit.v_results[0].m_album.c_str(),
						hit.v_results[0].m_title.c_str(),hit.v_results[0].m_comment.c_str(), hit.v_results[0].m_bitrate);
					hit.InitGUID();
					SendSpoofs(hit);
					break;
				}
			}
		}
	}
}

//
//
//
void GnutellaSocket::ExtractKeywordsFromQuery(vector<string> *keywords,const char *query)
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
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')) || (buf[i]=='\''))
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
/*
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
*/
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
//int GnutellaSocket::Spoof(ProjectKeywords *keywords,SupplyProject supply_project,ProjectStatus *status, int track, Query& qu, bool swarming_enabled)
//{
//	UINT i,j;
//	UINT num_sent=0;
//
//
//	//bool sent_poisons = false;
//
//	// Check for no spoof supply
//	if(supply_project.v_spoof_entries.size()==0)
//	{
//		qu.m_dists_sent = qu.m_poisons_sent = qu.m_spoofs_sent = 0;
//		return -1;
//	}
//
//	UINT first_spoof_entry_index = 0;
//	UINT last_spoof_entry_index = 0;
//	if(track > 0)
//	{
//		FindFirstAndLastSpoofEntryIndex(supply_project.v_spoof_entries, track, first_spoof_entry_index, last_spoof_entry_index);
//	}
//	else
//	{
//		last_spoof_entry_index = (int)supply_project.v_spoof_entries.size() - 1;
//	}
//
//#ifdef CLUMP_SPOOFING
//	if(track==0 && !swarming_enabled) //movies only
//	{
//		int count=0;
//		while(count <=NUM_CLUMPS)
//		{
//			count++;
//			UINT filesize=0;
//			string filename;
//			string info;
//			UINT rand_index=p_sockets->m_rng.GenerateWord32(first_spoof_entry_index,last_spoof_entry_index);
//			filesize = supply_project.v_spoof_entries[rand_index].m_file_length;
//			///* making the filesize to be dividable by 137 so that we can identify the spoofs  *///
//			filesize = (filesize / 137) * 137;
//			filename = supply_project.v_spoof_entries[rand_index].Filename();
//			info = supply_project.v_spoof_entries[rand_index].SHA1();
//			CryptoPP::SHA sha;
//			char sha1[20];
//			memset(&sha1,0,sizeof(sha1));
//			sha.Update((const unsigned char *)info.c_str(),info.length());
//			sha.Final((unsigned char *)sha1);
//			CString new_sha1;
//			new_sha1="urn:sha1:";
//			for(j=0;j<16;j++)
//				new_sha1.AppendFormat("%.2X",(byte)sha1[j]);
//			info = new_sha1;
//
//			// Init the hit
//			UINT num_of_sources=p_sockets->m_rng.GenerateWord32(2,NUM_IP_PER_CLUMP);
//
//			for(i=0; i<num_of_sources;i++)
//			{
//				FalseQueryHit hit;
//				hit.m_hdr.NumberOfHits(1);
//				hit.m_hdr.Port(6346);
//				hit.m_hdr.IP(p_sockets->SpoofIP());
//				hit.m_hdr.Speed(4+rand()%(3000-4));
//				
//				QueryHitResult result;
//				result.m_file_index=rand();
//				result.m_file_size=filesize;
//				result.m_file_name = filename;
//				result.m_info = info;
//				hit.v_results.push_back(result);
//				hit.InitTrailer();
//				hit.InitGUID();
//				Spoof15Files(hit,false);		// we want all spoofs to have a different IP, so just send 1 FalseQueryHit at a time to Spoof15Files
//				num_sent+=(int)hit.v_results.size();	// increment the number of hits sent
//				// Check to see if the socket was closed during sending
//				if(IsSocket()==false)
//				{
//					qu.m_dists_sent = qu.m_poisons_sent = 0;
//					qu.m_spoofs_sent = num_sent;
//					return num_sent;
//				}
//			}
//		}
//	}
//#endif
//	// The number of spoofs to send is the number of spoofs available, unless it is greater than the query multiplier
//	UINT num_to_send = last_spoof_entry_index - first_spoof_entry_index + 1;
//#ifdef NO_PROCESSING
//	vector<int> hash_index_used;
//	if(keywords->m_gnutella_swarming_enabled)
//	{
//		for(i=first_spoof_entry_index;i<=last_spoof_entry_index;i++)
//		{
//			hash_index_used.push_back(i);
//		}
//	}
//#endif	
//
//
//#ifndef NO_PROCESSING
//	if(num_to_send>keywords->m_query_multiplier)
//	{
//		num_to_send=keywords->m_query_multiplier;
//	}
//#else
//	num_to_send = keywords->m_query_multiplier;
//#endif
//
//#ifdef BEARSHARE_ONLY
//	num_to_send*=3;
//#endif
//	UINT total_popularity = 0;
//	for(i=first_spoof_entry_index; i<=last_spoof_entry_index; i++)
//	{
//		total_popularity += supply_project.v_spoof_entries[i].m_popularity;
//	}
////#endif
//
//	// Increment the appropriate counter
//#ifndef NO_PROCESSING
//	if(keywords->m_gnutella_spoofing_enabled)
//	{
//		status->m_spoof_count+=num_to_send;
//	}
//	else
//	{
//		status->m_poison_count+=num_to_send;
//	}
//#endif
//
//
//
//	// Limit the number of hits we send right now to around 100...letting them trickle out every second
//	//UINT limit=75+rand()%(50+1);	// 75-125
//
//
//	//look for the track title
//	CString track_title;
//	for(i=0;i<keywords->m_supply_keywords.v_keywords.size();i++)
//	{
//		if(keywords->m_supply_keywords.v_keywords[i].m_track == track)
//		{
//			track_title = keywords->m_supply_keywords.v_keywords[i].m_track_name.c_str();
//			break;
//		}
//	}
//	// Create a vector of false query hits to send back
//	GnutellaHost spoof_host;
//	
//	if(keywords->m_gnutella_decoys_enabled && last_spoof_entry_index-first_spoof_entry_index > 0)
//	{
//		int count=1;
//		//srand(keywords->m_id+track);
//		int index=first_spoof_entry_index;
//		UINT decoy_index = 0;
//		UINT decoy_sent=0;
//		//bool seeded=false;
//		string filename;
//		while(count <= NUM_DECOYS)
//		{
//			int same_filename_used= p_sockets->m_rng.GenerateWord32(1,100);
//			UINT filesize=0;
//			string filename;
//			string info="urn:sha1:";
//
//			decoy_index = rand()%(p_sockets->p_connection_manager->v_decoy_hashes.size());
//			filesize = p_sockets->p_connection_manager->v_decoy_hashes[decoy_index].m_filesize;
//			if(index > last_spoof_entry_index)
//				index=0;
//
//				if(rand()%2==0)
//				{
//					CString renamed = supply_project.v_spoof_entries[index].Filename();
//					CString appended_name;
//					appended_name.Format("(%d).mp3",same_filename_used);
//					if(! renamed.Replace(".mp3",appended_name) )
//						renamed.Replace(".MP3",appended_name);
//					filename = renamed;
//				}
//				else
//				{
//					CString renamed;
//					renamed.Format("(%d)%s",same_filename_used,supply_project.v_spoof_entries[index].Filename());
//					filename = renamed;
//				}
//
//			info += p_sockets->p_connection_manager->v_decoy_hashes[decoy_index].m_sha1;
//
//			UINT num_of_sources=p_sockets->m_rng.GenerateWord32(2,NUM_IP_PER_DECOYS);
//
//			// Init the hit
//			for(i=0; i<num_of_sources;i++)
//			{
//				FalseQueryHit hit;
//				hit.m_hdr.NumberOfHits(1);
//				hit.m_hdr.Port(6348);
//				if(i<p_sockets->p_connection_manager->v_decoyers.size())
//					hit.m_hdr.IP(p_sockets->p_connection_manager->v_decoyers[i].GetIPInt());
//				else
//				{
//					hit.m_hdr.IP(p_sockets->SpoofIP());
//				}
//				hit.m_hdr.Speed(4+rand()%(3000-4));
//				
//				QueryHitResult result;
//				result.m_file_index=rand();
//				result.m_file_size=filesize;
//				result.m_file_name = filename;
//				result.m_info = info;
//				hit.v_results.push_back(result);
//				hit.InitTrailer();
//				hit.InitGUID();
//				Spoof15Files(hit,false);		// we want all spoofs to have a different IP, so just send 1 FalseQueryHit at a time to Spoof15Files
//				decoy_sent+=(int)hit.v_results.size();	// increment the number of hits sent
//				// Check to see if the socket was closed during sending
//				if(IsSocket()==false)
//				{
//					qu.m_dists_sent = qu.m_poisons_sent = 0;
//					qu.m_spoofs_sent = num_sent;
//					return num_sent;
//				}
//			}
//			index++;
//			count++;
//		}
//		status->m_dist_count=decoy_sent;
//	}
////#ifndef NO_PROCESSING
//	for(i=0;i<num_to_send;i++)
//	{
//		FalseQueryHit hit;
//		UINT index=first_spoof_entry_index;
//		UINT popularity_index = 0;
//		UINT filesize=0;
//		string filename;
//		string info = "urn:sha1:";
//
//		UINT rand_index = p_sockets->m_rng.GenerateWord32(0, total_popularity);
//		for(index=first_spoof_entry_index; index <= last_spoof_entry_index; index++)
//		{
//			popularity_index += supply_project.v_spoof_entries[index].m_popularity;
//			if(rand_index <= popularity_index)
//				break;
//		}
//		if(index > last_spoof_entry_index)
//			index = last_spoof_entry_index;
//		
//#ifdef NO_PROCESSING
//		if(!keywords->m_gnutella_decoys_enabled)
//		{
//			//remove the index we used
//			vector<int>::iterator hash_iter=hash_index_used.begin();
//			while(hash_iter!=hash_index_used.end())
//			{
//				if(*(hash_iter) == index)
//				{
//					hash_index_used.erase(hash_iter);
//					break;
//				}
//				else
//					hash_iter++;
//			}
//		}
//#endif
//		filesize = supply_project.v_spoof_entries[index].m_file_length;
//		filename = supply_project.v_spoof_entries[index].Filename();
//		info += supply_project.v_spoof_entries[index].SHA1();
//
//		// Init the hit
//		hit.m_hdr.NumberOfHits(1);
//		hit.m_hdr.Port(6346);
//
//		// If poisoning, use the poisoner ip (if there are any)
//		//if(keywords->m_gnutella_spoofing_enabled)// else use an ip derived from a real ip //spoofing only
//		if(!swarming_enabled)
//		{
//#ifndef EDONKEY_DOS
//			hit.m_hdr.IP(p_sockets->SpoofIP());
//#else
//			p_sockets->SpoofEDonkeyIP(&spoof_host);
//			hit.m_hdr.IP(spoof_host.IP());
//			hit.m_hdr.Port(spoof_host.Port());
//#endif
//		}
//		//else if((keywords->m_gnutella_swarming_enabled) && (keywords->v_poisoners.size()>0))
//		else if(keywords->v_poisoners.size())
//		{
//			//sent_poisons = true;
//			// Randomly pick a poisoner ip for this project
//			//UINT rand_index=rand() % (UINT)keywords->v_poisoners.size();
//			UINT rand_index = p_sockets->m_rng.GenerateWord32(0, keywords->v_poisoners.size()-1);
//#ifdef BEARSHARE_ONLY
//
//			while(strstr((keywords->v_poisoners[rand_index].GetIPString()).c_str(),"38.")!=NULL)
//			{
//				rand_index = p_sockets->m_rng.GenerateWord32(0, keywords->v_poisoners.size()-1);
//			}
//#endif
//
//			hit.m_hdr.IP(keywords->v_poisoners[rand_index].GetIPInt());
//			hit.m_hdr.Port(keywords->v_poisoners[rand_index].GetPort());
//		}
//
//		if(keywords->m_gnutella_spoofing_enabled)
//		{
//			hit.m_hdr.Speed(4+rand()%(3000-4));	// 4096 - 50000
//		}
//		else
//		{
//			hit.m_hdr.Speed(20000+rand()%20000);	// 20,000 - 40,000
//		}
//		
//		QueryHitResult result;
//
//		// If it is a poison, then the index is the bitscramble of the size so that the poisoner knows what the file size is even if the
//		// remote client asks for 0-
//		if(swarming_enabled)
//		{
//			//result.m_file_index=BitScramble(size->m_entry_size);
//			result.m_file_index=BitScramble(filesize);
//		}
//		else
//		{
//			result.m_file_index=rand();
//		}
//		
//		result.m_file_size=filesize;
//		result.m_file_name = filename;
//		result.m_info = info;
//
//		hit.v_results.push_back(result);
//
//#ifdef ULTRAPEER
//		hit.InitTrailer();
//#else
//		if(!swarming_enabled || keywords->m_search_type != ProjectKeywords::search_type::audio)
//			hit.InitTrailer();
//		else
//		{
//			int bitrate=128;
//			int choices=rand()%3;
//			if(choices==1)
//				bitrate=192;
//			else if(choices==2)
//				bitrate=320;
//	
//			if(track_title.GetLength()==0)
//				track_title = filename.c_str();
//			hit.InitSwarmTrailer(keywords->m_artist_name.c_str(), keywords->m_album_name.c_str(), track_title,bitrate,track);
//		}
//#endif
//		hit.InitGUID();
//
//		// If poisoning is not set, then tweak the sizes and shas of the results
//		if(!swarming_enabled)
//		{
//			hit.Tweak();
//		}
//
//		// If we've sent the limit for this reply...cache the hits to be trickled out every second
//		/*
//		if(num_sent>limit)
//		{
//			Spoof15Files(hit,true);
//		}
//		else
//		{*/
//			Spoof15Files(hit,false);		// we want all spoofs to have a different IP, so just send 1 FalseQueryHit at a time to Spoof15Files
//		//}
//		num_sent+=(int)hit.v_results.size();	// increment the number of hits sent
//		// Check to see if the socket was closed during sending
//		if(IsSocket()==false)
//		{
//			if(swarming_enabled)
//			{
//				qu.m_dists_sent = qu.m_spoofs_sent = 0;
//				qu.m_poisons_sent = num_sent;
//			}
//			else
//			{
//				qu.m_dists_sent = qu.m_poisons_sent = 0;
//				qu.m_spoofs_sent = num_sent;
//			}	
//			return num_sent;
//		}
//	}
//#ifdef NO_PROCESSING //sending spoofs for the hashes didn't send on previous attempt
//	if(!keywords->m_gnutella_decoys_enabled)
//	{
//#ifdef BEARSHARE_ONLY
//		for(int j=0; j<3; j++)
//		{
//#endif
//			for(i=0;i<hash_index_used.size();i++)
//			{
//				FalseQueryHit hit;
//				UINT filesize=0;
//				string filename;
//				string info = "urn:sha1:";
//
//				filesize = supply_project.v_spoof_entries[hash_index_used[i]].m_file_length;
//				filename = supply_project.v_spoof_entries[hash_index_used[i]].Filename();
//				info += supply_project.v_spoof_entries[hash_index_used[i]].SHA1();
//
//				// Init the hit
//				hit.m_hdr.NumberOfHits(1);
//				hit.m_hdr.Port(6346);
//
//				// If poisoning, use the poisoner ip (if there are any)
//				if(!swarming_enabled)// else use an ip derived from a real ip //spoofing only
//				{
//					hit.m_hdr.IP(p_sockets->SpoofIP());
//				}
//				else if(keywords->v_poisoners.size())
//				{
//					//sent_poisons = true;
//					// Randomly pick a poisoner ip for this project
//					UINT rand_index=rand() % (UINT)keywords->v_poisoners.size();
//
//					hit.m_hdr.IP(keywords->v_poisoners[rand_index].GetIPInt());
//					hit.m_hdr.Port(keywords->v_poisoners[rand_index].GetPort());
//				}
//
//				if(keywords->m_gnutella_spoofing_enabled)
//				{
//					hit.m_hdr.Speed(4+rand()%(3000-4));	// 4096 - 50000
//				}
//				else
//				{
//					hit.m_hdr.Speed(20000+rand()%20000);	// 20,000 - 40,000
//				}
//				
//				QueryHitResult result;
//
//				// If it is a poison, then the index is the bitscramble of the size so that the poisoner knows what the file size is even if the
//				// remote client asks for 0-
//				if(swarming_enabled)
//				{
//					//result.m_file_index=BitScramble(size->m_entry_size);
//					result.m_file_index=BitScramble(filesize);
//				}
//				else
//				{
//					result.m_file_index=rand();
//				}
//				
//				result.m_file_size=filesize;
//				result.m_file_name = filename;
//				result.m_info = info;
//
//				hit.v_results.push_back(result);
//
//		//		hit.InitTrailer();
//				if(!swarming_enabled || keywords->m_search_type != ProjectKeywords::search_type::audio)
//					hit.InitTrailer();
//				else
//				{
//					int bitrate=128;
//					int choices=rand()%3;
//					if(choices==1)
//						bitrate=192;
//					else if(choices==2)
//						bitrate=320;
//			
//					if(track_title.GetLength()==0)
//						track_title = filename.c_str();
//					hit.InitSwarmTrailer(keywords->m_artist_name.c_str(), keywords->m_album_name.c_str(), track_title,bitrate,track);
//				}
//
//				hit.InitGUID();
//
//				// If poisoning is not set, then tweak the sizes and shas of the results
//				if(!swarming_enabled)
//				{
//					hit.Tweak();
//				}
//
//				// If we've sent the limit for this reply...cache the hits to be trickled out every second
//				/*
//				if(num_sent>limit)
//				{
//					Spoof15Files(hit,true);
//				}
//				else
//				{*/
//					Spoof15Files(hit,false);		// we want all spoofs to have a different IP, so just send 1 FalseQueryHit at a time to Spoof15Files
//				//}
//				num_sent+=(int)hit.v_results.size();	// increment the number of hits sent
//				// Check to see if the socket was closed during sending
//				if(IsSocket()==false)
//				{
//					if(swarming_enabled)
//					{
//						qu.m_dists_sent = qu.m_spoofs_sent = 0;
//						qu.m_poisons_sent = num_sent;
//					}
//					else
//					{
//						qu.m_dists_sent = qu.m_poisons_sent = 0;
//						qu.m_spoofs_sent = num_sent;
//					}	
//					return num_sent;
//				}
//			}
//#ifdef BEARSHARE_ONLY
//		}
//#endif
//	}
//#endif
//
//	//return num_sent;
//	if(swarming_enabled)
//	{
//		qu.m_dists_sent = qu.m_spoofs_sent = 0;
//		qu.m_poisons_sent = num_sent;
//	}
//	else
//	{
//		qu.m_dists_sent = qu.m_poisons_sent = 0;
//		qu.m_spoofs_sent = num_sent;
//	}
//	return num_sent;
//}

//
//
//
//void GnutellaSocket::Spoof15Files(FalseQueryHit &hit,bool cache)
//{
//	int buf_len=sizeof(GnutellaHeader)+hit.GetBufferLength();
//	char *buf=new char[buf_len];
//
//	GnutellaHeader *hdr=(GnutellaHeader *)buf;
//	hdr->Init();
//	hdr->Guid(m_hdr.Guid());
//	hdr->Op(GnutellaOpCode::QueryHit);
//	hdr->Length(hit.GetBufferLength());
//	int ttl= m_hdr.Hops() + 2;
//	hdr->TTL(ttl);
//
//	hit.WriteToBuffer(&buf[sizeof(GnutellaHeader)]);
//
//	// Check to see if we're supposed to cache these query hits
//	if(cache)
//	{
//		CSingleLock singleLock(&m_critical_section);
//		singleLock.Lock();
//		if(singleLock.IsLocked())
//		{
//			// Check to see if the cached buffer count has gotten too big
//			if(v_cached_query_hits_to_send.size()>MAX_CACHED_QUERY_HITS)
//			{
//				// Remove 25% of them
//				while(v_cached_query_hits_to_send.size()>MAX_CACHED_QUERY_HITS*(.75))
//				{
//					delete [] *v_cached_query_hits_to_send.begin();
//					v_cached_query_hits_to_send.erase(v_cached_query_hits_to_send.begin());
//				}
//			}
//
//			// Cache the buffer
//			char *cached_buf=new char[sizeof(unsigned int)+buf_len];
//			*((unsigned int *)&cached_buf[0])=buf_len;
//			memcpy(&cached_buf[sizeof(unsigned int)],buf,buf_len);
//			v_cached_query_hits_to_send.push_back(cached_buf);
//			singleLock.Unlock();
//		}
//	}
//	else
//	{
//		SendSocketData(buf,buf_len);
//	}
///*
//	for(int i=0;i<buf_len;i++)
//	{
//		if(i%16==0)
//		{
//			OutputDebugString("\n");
//		}
//		
//		char msg[10];
//		sprintf(msg,"%02x ",(unsigned char)buf[i]);
//		OutputDebugString(msg);
//	}
//*/
//	delete [] buf;
//}


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

//
//
//
void GnutellaSocket::RemoveArtistNames(vector<string>& keywords, string& artist_name)
{
	vector<string>::iterator iter=keywords.begin();
	while(iter!=keywords.end())
	{
		if(strstr(artist_name.c_str(), iter->c_str())!=NULL)
		{
			keywords.erase(iter);
		}
		else
			iter++;
	}
}
