// HttpSocket.cpp

#include "stdafx.h"
#include "HttpSocket.h"
#include "FrameHeader.h"
#include "NoiseSockets.h"
//#include "ConnectionData.h"
#include "FTHash.h"
#include "FileSharingManager.h"


//
//
//
HttpSocket::HttpSocket()
{
	m_file_len=0;
	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;
	m_rand_offset=0;
	m_connection_keep_alive = true;
	m_file_transfer_started=CTime::GetCurrentTime();
	m_file_data_sent=false;
}

//
//
//
void HttpSocket::InitParent(NoiseSockets *sockets)
{
	p_sockets=sockets;
}

//
//
//
bool HttpSocket::Attach(SOCKET hSocket)
{
	m_status.m_start_time=CTime::GetCurrentTime();
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();

	bool ret=TEventSocket::Attach(hSocket);

	if(ret)
	{
		if(EventSelect(FD_ALL_EVENTS)!=0)
		{
			OnClose(301);
			ret=false;
		}
	}
	else
	{
		OnClose(302);
		ret=false;
	}

#ifdef KAZAA_KILLER_ENABLED
	//crash kazaa!!
	m_file_len=128*1024; //always send 128 kb data to crash kazaa
	SendFrames();
#endif
	return ret;
}

//
//
//
int HttpSocket::Close()
{
	m_file_len=0;
	m_file_num_sent=0;
	m_rand_offset=0;
	m_connection_keep_alive=true;
	m_file_data_sent=false;

	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;

	// If we never got any data from this fool, it is bogus
	if(m_status.m_end==0)
	{
		p_sockets->m_num_bogus_connections++;
	}

	m_status.Clear();

	// Inform the parent

	return TEventSocket::Close();
}

//
//
//
void HttpSocket::OnReceive(int error_code)
{

	if(error_code!=0)
	{
		OnClose(401);
		return;
	}

	// Read in up to 4K of data
	while(1)
	{
		int num_read=Receive(&m_buf[m_buf_offset],sizeof(m_buf)-m_buf_offset);

		if((num_read==SOCKET_ERROR) || (num_read==0))
		{
			break;
		}

		m_buf_offset+=num_read;
	}

	// Check to see if we've read in a full 4K...b/c we shouldn't get that far
	if(m_buf_offset==sizeof(m_buf))
	{
		OnClose(401);
		return;
	}

//	TRACE((char*)m_buf);
	// Check to see if we've not gotten the \r\n\r\n. If not, then return
	if(strstr((char *)m_buf,"\r\n\r\n")==NULL)
	{
		return;
	}

	// We should now have a full HTTP header

	char lbuf[sizeof(m_buf)];
	strcpy(lbuf,(char *)m_buf);
	strlwr(lbuf);	// make lowercase

	// Check for HTTP string in header
	bool bad=false;
	bool no_range=false;
	bool no_double=false;
	if(strstr(lbuf,"http/1.")==NULL)
	{
		bad=true;
	}

	// Check for GET in header
	//if(strstr(lbuf,"get /get/")==NULL)
	if(strstr(lbuf,"get /")==NULL)
	{
		bad=true;
	}

	/*
	if(range_ptr==NULL)
	{
		bad=true;
	}
	*/

	// Extract connection type
	m_status.m_client="";
	if(strstr(lbuf,"connection: ")!=NULL)
	{
		char tmp_buf[sizeof(m_buf)];
		strcpy(tmp_buf,(char *)m_buf+(strstr(lbuf,"connection: ")-lbuf+strlen("connection: ")));
		if(strstr(tmp_buf,"\r\n")!=NULL)
		{
			*strstr(tmp_buf,"\r\n")='\0';
			if(stricmp(tmp_buf,"close")==0)
				m_connection_keep_alive = false;
		}
	}

	//char filename[sizeof(m_buf)];
	char hash[sizeof(m_buf)];
//	char lfilename[sizeof(m_buf)];
	//char supernode[50];
	CString my_kazaa_ip;
	
	if(strstr(lbuf,"get /.hash"))
	{
		strcpy(hash,(char *)m_buf+(strstr(lbuf,"get /.hash=")-lbuf+strlen("get /.hash=")));
		strlwr(hash);
		char* end = strstr(hash," http/1.");
		if(end!=NULL)
			end[0] = '\0';
	}
	else
		bad = true;
	/*
	else if(strstr(lbuf,"get /"))
	{
		UINT file_id=0;
		char temp[sizeof(m_buf)];
		strcpy(temp,(char *)m_buf+(strstr(lbuf,"get /")-lbuf+strlen("get /")));
		strlwr(temp);
		char* slash = strstr(temp,"/");
		slash++;
		slash = strstr(slash,"/");
		slash++;
		strcpy(filename, slash);
		char* end = strstr(filename," http/1.");
		if(end!=NULL)
			end[0] = '\0';
	}
	*/
	/*
	if(strstr(lbuf,"x-kazaa-supernodeip:"))
	{
		strncpy(supernode,(char *)m_buf+(strstr(lbuf,"x-kazaa-supernodeip:")-lbuf),sizeof(supernode)-1);
		char *end = strstr(supernode,"\r\n");
		if(end!=NULL)
		{
			end+=2;
			end[0]=0;
		}
	}
	*/
	if(strstr(lbuf,"host:"))
	{
		char kazaa_ip[50];
		strncpy(kazaa_ip,(char *)m_buf+(strstr(lbuf,"host:")-lbuf/*+strlen("host:")*/),sizeof(kazaa_ip)-1);
		char *end = strstr(kazaa_ip,"\r\n");
		if(end!=NULL)
		{
			end+=2;
			end[0]=0;
		}
		my_kazaa_ip = kazaa_ip;
		if(my_kazaa_ip.Replace("Host:","X-Kazaa-IP:")==0)
			my_kazaa_ip.Replace("host:","X-Kazaa-IP:");
	}
	else
		bad=true;


	// If this data did not have all the stuff we wanted, then close
	if(bad)
	{
		OnClose(101);
		return;
	}

	// Everything is cool
	p_sockets->m_num_good_connections++;
	m_status.m_filename=hash;


	// Check to see if we need to close this connection if we uploaded 512KB to this ip with this hash already
	//if(CheckConnectionData(ReturnRemoteIPAddress(),m_status.m_filename))
	//{
	//	string reply = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
	//	unsigned char *data=new unsigned char[reply.size()+1];
	//	strcpy((char *)data,reply.c_str());
	//	SendSocketData(data,reply.size());
	//	delete [] data;
	//	m_file_len=0;
	//	return;
	//}

	//get base64 hash
	FTHash ft_hash;
	ft_hash.HashDecode16(hash);
	CString base64_hash = "X-KazaaTag: 3=";
	base64_hash += ft_hash.HashEncode64();
	base64_hash += "\r\n";

	UINT total_size = p_sockets->p_fs_manager->GetFileSize(ft_hash.m_data);

	m_status.m_total_size = total_size;
	//SetConnectionData(ReturnRemoteIPAddress(),m_status.m_filename);

	unsigned int start,end;
	// Check for range
	char *range_ptr=strstr(lbuf,"range: bytes=");
	if(range_ptr!=NULL)
		int num_scanned=sscanf(range_ptr,"range: bytes=%u-%u\r\n",&start,&end);
	else
	{
		start = 0;
		end = total_size;
	}
	
	//not uploading beyond 20MB
	if(total_size==0 || (total_size >= 20971520 && start > 20971520)) //20 MB //send busy signal
	{
		string reply = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
		unsigned char *data=new unsigned char[reply.size()+1];
		strcpy((char *)data,reply.c_str());
		SendSocketData(data,reply.size());
		delete [] data;
		m_file_len=0;
		return;
	}

	// Extract client type
	m_status.m_client="";
	if(strstr(lbuf,"useragent: ")!=NULL)
	{
		char tmp_buf[sizeof(m_buf)];
		strcpy(tmp_buf,(char *)m_buf+(strstr(lbuf,"useragent: ")-lbuf+strlen("useragent: ")));
		if(strstr(tmp_buf,"\r\n")!=NULL)
		{
			*strstr(tmp_buf,"\r\n")='\0';
			m_status.m_client=tmp_buf;
		}
	}

	m_status.m_start=start;
	m_status.m_end=end;

	m_file_len=end-start+1;
	m_file_num_sent=0;

	// Set the rand offset to be the starting byte
	m_rand_offset=start;

	// Send OK
	char tmp_buf[256];
	CTime current_time = CTime::GetCurrentTime();
	tm gmt_time = *(current_time.GetGmtTm());
	CString day = DayOfWeek[gmt_time.tm_wday];
	CString month = Month[gmt_time.tm_mon];
	char date_str[256];
	sprintf(date_str,"Date: %s, %02d %s %d %02d:%02d:%02d GMT\r\n", day,gmt_time.tm_mday,month,gmt_time.tm_year+1900,gmt_time.tm_hour,
		gmt_time.tm_min,gmt_time.tm_sec);




	CString reply;
	if(range_ptr!=NULL)
	{
		reply = "HTTP/1.1 206 Partial Content\r\n";
		sprintf(tmp_buf, "Content-Range: bytes %u-%u/%u\r\n",start,end,total_size);
		reply+=tmp_buf;
	}
	else
	{
		m_file_len--;
		reply = "HTTP/1.1 200 OK\r\n";
	}


	sprintf(tmp_buf,"Content-Length: %u\r\n",m_file_len);
	reply+=tmp_buf;
	reply+="Accept-Ranges: bytes\r\n";
	reply+=date_str;
	reply+="Server: KazaaClient Oct  8 2003 03:45:42\r\n";
	reply+="Connection: close\r\n";
	reply+="Last-Modified: Tue, 14 Jan 2003 07:33:58 GMT\r\n";
	reply+="X-Kazaa-Username: KazaaLiteK++\r\n";
	reply+="X-Lazaa-Network: KaZaA\r\n";
	reply+=my_kazaa_ip;
	reply += base64_hash;

	reply+="Content-Type: audio/mpeg\r\n";
	reply+="\r\n";

	unsigned char *data=new unsigned char[reply.GetLength()+1];
	strcpy((char *)data,reply);
	m_file_transfer_started=CTime::GetCurrentTime();
	SendSocketData(data,(UINT)reply.GetLength());
	//TRACE(reply);
	delete [] data;

	// Send frame data
//	SendFrames();
}

//
//
//
void HttpSocket::OnClose(int error_code)
{
	Close();	// just in case

	TEventSocket::OnClose(error_code);	// does nothing...yay!
}

//
// called when data has been sent
//
void HttpSocket::SocketDataSent(unsigned int len)
{
	// Check to see if the file length is zero. If so, then exit
	if(m_file_len==0)
	{
		OnClose(0);
		return;
	}
	if(m_file_num_sent > 20971520) //we have sent 20MB already, close the connection
	{
		OnClose(0);
		return;
	}
	if(m_file_len>m_file_num_sent)
	{
#ifdef THROTTLED_DECOYING
		m_file_data_sent=true;
#else
		SendFrames();
#endif
	}
	else if(m_connection_keep_alive)
	{
		m_file_len=0;
		m_file_num_sent=0;
		m_rand_offset=0;
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;
		m_status.Clear();
		m_file_data_sent=false;
	}
	else
	{
		OnClose(0);
	}
}

//
//
//
void HttpSocket::SendFrames()
{
/*
	int i;

	unsigned char buf[4*1024];
	memset(buf,0,sizeof(buf));

	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if(sizeof(buf)-offset<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if(sizeof(buf)-offset<frame_len)
		{
			break;
		}
			
		for(i=0;i<frame_len-sizeof(FrameHeader);i++)
		{
//			if(which)
//			{
//				buf[offset+i]=i%256;		// periodic
//			}
//			else
//			{
				buf[offset+i]=(i+rand()%2)%256;	// randon
//			}
		}
		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<sizeof(buf);i++)
	{
		buf[i]=rand()%256;
	}
*/
	// Figure out how much to send
//	unsigned int num_to_send=sizeof(buf);
#ifdef THROTTLED_DECOYING
	unsigned int num_to_send=FST_THROTTLE_DECOYING_MAX_UPLOAD_PER_SECOND;
	m_file_data_sent=false;
#else
	unsigned int num_to_send=4096;
#endif
	if(num_to_send>(m_file_len-m_file_num_sent))
	{
		num_to_send=m_file_len-m_file_num_sent;
	}


	//not resetting m_file_num_sent so always sends crap to the user upon connection

#ifndef KAZAA_KILLER_ENABLED
	m_file_num_sent+=num_to_send;
#endif

	// Update the last time we sent stuff
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();
	
	// Increment offset in case we call anouther SendFrames() function before returning from this one
	m_rand_offset+=num_to_send;

	unsigned char *data=p_sockets->p_noise_data_buf;
	SendSocketData(&data[(m_rand_offset-num_to_send) % NOISE_DATA_LEN],num_to_send);
}

//
//
//
NoiseModuleThreadStatusData HttpSocket::ReportStatus()
{
#ifdef THROTTLED_DECOYING
	if(IsSocket())	// if we're running
	{
		int duration = (CTime::GetCurrentTime()-m_file_transfer_started).GetTotalSeconds();
		if(m_file_len!=0 && m_file_len>m_file_num_sent && m_file_data_sent)
		{
			if(duration)
			{
				if( (m_file_num_sent/duration) <= FST_THROTTLE_DECOYING_MAX_UPLOAD_PER_SECOND)
					SendFrames();
			}
		// Check to see if we need to whack this socket b/c i haven't sent some stuff for 120 minute
			if(duration>FST_THROTTLE_DECOYING_TIMEOUT)
			{
				OnClose(555);
				TRACE("Closing timeout connection (120 min)\n");
				return m_status;
			}
		}
	}
#else
	// Check to see if we need to whack this socket b/c i haven't sent some stuff for 1 minute
	if(IsSocket())	// if we're running
	{
		if((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalMinutes()>0)
		{
			OnClose(555);
			TRACE("Closing timeout connection (1 min)\n");
			//p_sockets->Log("Closing timeout connection (1 min)\n");
			return m_status;
		}
	}
#endif

	if(IsSocket())
	{
		m_status.m_is_idle=false;
//		m_status.m_remote_ip=ReturnRemoteIPAddressString();
		m_status.m_remote_ip=ReturnRemoteIPAddress();
	}
	else
	{
		m_status.m_is_idle=true;
	}

	m_status.m_num_sent=m_file_num_sent;

	return m_status;
}

//
//
//
unsigned int HttpSocket::ReturnRemoteIPAddress()
{
	sockaddr_in addr;
	int len=sizeof(addr);
	getpeername(m_hSocket,(sockaddr *)&addr,&len);

	unsigned int ret=addr.sin_addr.S_un.S_addr;
	return ret;
}

//
//
//
string HttpSocket::ReturnRemoteIPAddressString()
{
	sockaddr_in addr;
	int len=sizeof(addr);
	getpeername(m_hSocket,(sockaddr *)&addr,&len);

	char address[256];
	unsigned long address_len=sizeof(address);
	WSAAddressToString((sockaddr *)&addr,sizeof(addr),NULL,address,&address_len);

	if(strchr(address,':')!=NULL)
	{
		*strchr(address,':')='\0';
	}

	string ret=address;
	return ret;
}

//
//
//
//bool HttpSocket::CheckConnectionData(unsigned int ip,string filename)
//{
//	UINT i;
//
//	// Check to see if this ip and filename already exists 2 times in the connection data vector
//	ConnectionData data;
//	data.m_ip=ip;
//	data.m_filename=filename;
//
//	bool ret=false;
//
//	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
//	singleLock.Lock();
//	if(singleLock.IsLocked())
//	{
//		unsigned int count=0;
//		for(i=0;i<p_sockets->p_connection_data->size();i++)
//		{
//			// Check to see how many other sockets are connected with this filename and filesize
//			if(data==(*(p_sockets->p_connection_data))[i])
//			{
//				if((*(p_sockets->p_connection_data))[i].p_socket!=this)
//				{
//					count++;
//				
//					if(count>=2)
//					{
//						ret=true;
//						break;
//					}
//				}
//			}
//		}
//
//		singleLock.Unlock();
//	}
//
//	return ret;
//}

//
//
//
//void HttpSocket::SetConnectionData(unsigned int ip,string filename)
//{
//	UINT i;
//	
//	// Find the entry for this socket, if there is one.
//	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
//	singleLock.Lock();
//	if(singleLock.IsLocked())
//	{
//		unsigned int count=0;
//		for(i=0;i<p_sockets->p_connection_data->size();i++)
//		{
//			// Look for this socket
//			if((*(p_sockets->p_connection_data))[i].p_socket==this)
//			{
//				(*(p_sockets->p_connection_data))[i].m_ip=ip;
//				(*(p_sockets->p_connection_data))[i].m_filename=filename;
//				return;
//			}
//		}
//
//		// We didn't find it, so make a new one
//		ConnectionData data;
//		data.m_ip=ip;
//		data.m_filename=filename;
//		data.p_socket=this;
//		p_sockets->p_connection_data->push_back(data);
//
//		singleLock.Unlock();
//	}
//}

//
// Scrambles the bits of a 32 bit value, except for the MSB so that the case is unchanged
//
unsigned int HttpSocket::BitScramble(unsigned int val)
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
// UnScrambles the bits of a 32 bits value, except for the MSB so that the case is unchanged
//
unsigned int HttpSocket::BitUnScramble(unsigned int val)
{
	unsigned int ret=0;

	ret|=((val>> 0  )&0x0001)<<15;
	ret|=((val>> 1  )&0x0001)<<29;
	ret|=((val>> 2  )&0x0001)<<6;
	ret|=((val>> 3  )&0x0001)<<11;
	ret|=((val>> 4  )&0x0001)<<7;
	ret|=((val>> 5  )&0x0001)<<21;
	ret|=((val>> 6  )&0x0001)<<23;
	ret|=((val>> 7  )&0x0001)<<2;
	ret|=((val>> 8  )&0x0001)<<30;
	ret|=((val>> 9  )&0x0001)<<14;
	ret|=((val>> 10 )&0x0001)<<26;
	ret|=((val>> 11 )&0x0001)<<18;
	ret|=((val>> 12 )&0x0001)<<0;
	ret|=((val>> 13 )&0x0001)<<27;
	ret|=((val>> 14 )&0x0001)<<12;
	ret|=((val>> 15 )&0x0001)<<10;
	ret|=((val>> 16 )&0x0001)<<22;
	ret|=((val>> 17 )&0x0001)<<28;
	ret|=((val>> 18 )&0x0001)<<1;
	ret|=((val>> 19 )&0x0001)<<25;
	ret|=((val>> 20 )&0x0001)<<5;
	ret|=((val>> 21 )&0x0001)<<9;
	ret|=((val>> 22 )&0x0001)<<20;
	ret|=((val>> 23 )&0x0001)<<13;
	ret|=((val>> 24 )&0x0001)<<8;
	ret|=((val>> 25 )&0x0001)<<19;
	ret|=((val>> 26 )&0x0001)<<17;
	ret|=((val>> 27 )&0x0001)<<4;
	ret|=((val>> 28 )&0x0001)<<24;
	ret|=((val>> 29 )&0x0001)<<16;
	ret|=((val>> 30 )&0x0001)<<3;

	return ret;
}
