// HttpSocket.cpp

#include "stdafx.h"
#include "HttpSocket.h"
#include "FrameHeader.h"
#include "NoiseSockets.h"
#include "ConnectionData.h"
//#include "HashManager.h"
#include "FileSharingManager.h"
#include "UploadInfo.h"

//
//
//
HttpSocket::HttpSocket()
{
	m_connection_keep_alive=true;
	m_file_len=0;
	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;
	m_rand_offset=0;
	m_file_is_opened=false;
}
//
//
//

HttpSocket::~HttpSocket()
{
	WEventSocket::Close();
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

	return ret;
}

//
//
//
int HttpSocket::Close()
{
	if(m_file_is_opened)
	{
		m_file.Close();
		m_file_is_opened=false;
	}
	m_file_len=0;
	m_file_num_sent=0;
	m_rand_offset=0;
	m_connection_keep_alive = true;

	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;

	// If we never got any data from this fool, it is bogus
	if(m_status.m_end==0)
	{
		p_sockets->m_num_bogus_connections++;
	}

	m_status.Clear();

	// Inform the parent

	return WEventSocket::Close();
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
/*
	char buf[4096];
	char lbuf[sizeof(buf)];
	memset(buf,0,sizeof(buf));
	int num_read=Receive(buf,sizeof(buf));
*/

/*
	// If we already know the end value, then we are in a transfer. Just ignore this stuff.
	if(m_status.m_end>0)
	{
		return;
	}
*/

	// Read in up to 4K of data (if a valid HTTP header is bigger than 4K, then I'm an ebert turd)
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

	// Check for range
	char *range_ptr=strstr(lbuf,"range: bytes=");
	if(range_ptr==NULL)
	{
		bad=true;
	}

	//Check for client
	//if(strstr(lbuf,"bearshare")!=NULL)
	//{
	//	bad=true;
	//}


	char filename[sizeof(m_buf)];
	//char lfilename[sizeof(m_buf)];
/*	bool request_by_index = false;
	if(strstr(lbuf,"get /get/"))
	{
		strcpy(filename,(char *)m_buf+(strstr(lbuf,"get /get/")-lbuf+strlen("get /get/")));
		request_by_index = true;
	}
	else*/ if(strstr(lbuf,"get /uri-res/n2r?urn:sha1:"))
	{
		strcpy(filename,(char *)m_buf+(strstr(lbuf,"get /uri-res/n2r?urn:sha1:")-lbuf+strlen("get /uri-res/n2r?urn:sha1:")));
		char* space_ptr = strstr(filename," ");
		*space_ptr = '\0';
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

	/*
	strcpy(lfilename,filename);
	strlwr(lfilename);
	*(filename+(strstr(lfilename,"http/1.")-lfilename))='\0';
	if(strchr(filename,'/')!=NULL)
	{
		m_status.m_filename=strchr(filename,'/')+1;
	}
	*/
	
	//CString hash = filename;
	//hash.Trim();
	//m_status.m_filename=hash;
	//strcpy(filename,hash);


	// Read in the scrambled file index and unscramble it
	//unsigned int scrambled_index,unscrambled_filesize;
	//if(request_by_index)
	//{
	//	sscanf(filename,"%u",&scrambled_index);
	//	unscrambled_filesize=BitUnScramble(scrambled_index);

	//	if(strstr(lbuf,"no doubt - oi to the world")!=NULL)
	//	{
	//		unscrambled_filesize=2598162;	//temp kludge
	//	}
	//}
/*
	// Check to see if we need to close this connection if this user and file already has 2 connections
	if(CheckConnectionData(ReturnRemoteIPAddress(),m_status.m_filename))
	{
		OnClose(102);
		return;
	}
	else
	{*/
		// If we didn't return, then set the connectionn data
		SetConnectionData(ReturnRemoteIPAddress(),m_status.m_filename);
	//}

	unsigned int start,end;
	int num_scanned=sscanf(range_ptr,"range: bytes=%u-%u\r\n",&start,&end);
	
	// Check for limewire's Range: bytes=xxxxxx-
	m_status.m_end_was_calculated=false;
	//if(request_by_index)
	//{
	//	if(num_scanned==1)
	//	{
	//		// If they want number to end, ("#-") then get the file size from the unscrabled index
	//		end=unscrambled_filesize-1;	// the last byte number is the size-1 since bytes are counted starting with 0

	//		// Check to see if the unscrambled file size is hella big (bigger than 500 MB)...if so, put it to a max of 10 MB
	//		if(end>500*1024*1024)
	//		{
	//			m_status.m_end_was_calculated=true;

	//			// If start >= 10 MB, make end = start + 1MB
	//			if(start>=10*1024*1024)
	//			{
	//				end=start+1*1024*1024;	// start + 1 MB
	//			}
	//			else
	//			{
	//				end=10*1024*1024;	// 10 MB
	//			}
	//		}
	//	}
	//}
	
	
	// Extract client type
	m_status.m_client="";
	if(strstr(lbuf,"user-agent: ")!=NULL)
	{
		char tmp_buf[sizeof(m_buf)];
		strcpy(tmp_buf,(char *)m_buf+(strstr(lbuf,"user-agent: ")-lbuf+strlen("user-agent: ")));
		if(strstr(tmp_buf,"\r\n")!=NULL)
		{
			*strstr(tmp_buf,"\r\n")='\0';
			m_status.m_client=tmp_buf;
		}
	}

	// Extract connection type
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

	m_status.m_start=start;
	m_status.m_end=end;

	m_file_len=end-start+1;
	m_file_num_sent=0;

	// Set the rand offset to be the starting byte
	m_rand_offset=start;


	UINT filesize=0;
	//if(!request_by_index)
	//{
		FileSizeAndHash size_hash = p_sockets->p_fs_manager->GetFileSize(filename);
		filesize=size_hash.m_filesize;
	//}
	//else
	//{
	//	filesize=unscrambled_filesize;
	//}
	if(filesize==0 /*|| (filesize >= 20971520 && start > 20971520)*/) //20 MB //send busy signal
	{
		m_file_len=0;
		string reply = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
		unsigned char *data=new unsigned char[reply.size()+1];
		strcpy((char *)data,reply.c_str());
		SendSocketData(data,reply.size());
		delete [] data;
		return;
	}
	m_status.m_filename=size_hash.m_filename;


	if(m_file_is_opened)
	{
		m_file.Close();
		m_file_is_opened=false;
	}
	if(m_file.Open(m_status.m_filename.c_str(),CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)==0)
	{
		string reply = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
		unsigned char *data=new unsigned char[reply.size()+1];
		strcpy((char *)data,reply.c_str());
		m_file_len=0;
		SendSocketData(data,reply.size());
		delete [] data;
		return;
	}
	else
	{
		m_file_is_opened=true;
		m_file.Seek(start, CFile::begin);
	}


	// Send OK
	char tmp_buf[256];
	string reply;
	//if(request_by_index)
	//	reply="HTTP/1.1 206 OK\r\n";
	//else
		reply="HTTP/1.1 206 Partial Content\r\n";
//	string reply="HTTP/1.1 200 OK\r\n";
//	string reply="HTTP 200 OK\r\n";
//	reply+="Server: BearShare 2.6.0\r\n";
//	reply+="Connection: close\r\n";
//	reply+="Connection: Keep-Alive\r\n";
//	reply+="Content-length: 4414456\r\n";	// 4414455
/*
	string remote_ip = ReturnRemoteIPAddressString();
	if(strcmp(remote_ip.c_str(),"38.119.66.25")==0)
	{
		reply+="Server: LimeWire/3.8.7\r\n";
		reply+="Content-Type: application/binary\r\n";
		sprintf(tmp_buf,"Content-Length: %u\r\n",m_file_len);
		reply+=tmp_buf;
		sprintf(tmp_buf,"Content-Range: bytes %u-%u/%u\r\n",start,end,3456789);
		reply+=tmp_buf;
		sprintf(tmp_buf,"X-Gnutella-Content-URN: urn:sha1:%s\r\n", filename);
		reply+=tmp_buf;
	}
	//else if(!request_by_index)*/
	//{
	//	reply+="Server: Gnucleus 2.0.1.0 (GnucDNA 1.0.2.6)\r\n";
	//	reply+="Content-Type: application/binary\r\n";
	//	//sprintf(tmp_buf,"Content-Length: %u\r\n",end+1);
	//	sprintf(tmp_buf,"Content-Length: %u\r\n",m_file_len);
	//	reply+=tmp_buf;
	//	sprintf(tmp_buf,"Content-Range: bytes %u-%u/%u\r\n",start,end,filesize);
	//	reply+=tmp_buf;
	//	sprintf(tmp_buf,"X-Gnutella-Content-URN: urn:sha1:%s\r\n", filename);
	//	reply+=tmp_buf;
	//}
	//else
	//{
		reply+="Content-Type: application/binary\r\n";
		sprintf(tmp_buf,"Content-Length: %u\r\n",m_file_len);
		reply+=tmp_buf;
		sprintf(tmp_buf,"Content-Range: bytes %u-%u/%u\r\n",start,end,filesize);
		reply+=tmp_buf;
		sprintf(tmp_buf,"X-Gnutella-Content-URN: urn:sha1:%s\r\n", filename);
		reply+=tmp_buf;

	reply+="\r\n";

	unsigned char *data=new unsigned char[reply.size()+1];
	strcpy((char *)data,reply.c_str());
	SendSocketData(data,reply.size());
	delete [] data;

	//send log to db
	UploadInfo info;
	info.m_hash=filename;
	info.m_filename=m_status.m_filename.c_str();
	int delete_index = info.m_filename.ReverseFind('\\');
	info.m_filename.Delete(0,delete_index+1);
	info.m_ip=ReturnRemoteIPAddressString().c_str();
	if(info.m_filename.GetLength() && info.m_ip.GetLength())
		p_sockets->LogToDB(info);
}

//
//
//
void HttpSocket::OnClose(int error_code)
{
	Close();	// just in case

	WEventSocket::OnClose(error_code);	// does nothing...yay!
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

	//if(m_file_num_sent > 20971520) //we have sent 20MB already, close the connection
	//{
	//	OnClose(0);
	//	return;
	//}
	if(m_file_len>m_file_num_sent)
	{
		SendFrames();
	}
	else if(m_connection_keep_alive)//we have sent all data, resettng all data members
	{
		m_file_len=0;
		m_file_num_sent=0;
		m_rand_offset=0;
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;
		m_status.Clear();
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
	// Figure out how much to send
//	unsigned int num_to_send=sizeof(buf);
	unsigned int num_to_send=4096;
	if(num_to_send>(m_file_len-m_file_num_sent))
	{
		num_to_send=m_file_len-m_file_num_sent;
	}
	m_file_num_sent+=num_to_send;

	// Update the last time we sent stuff
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();
	
	// Increment offset in case we call anouther SendFrames() function before returning from this one
	//m_rand_offset+=num_to_send;

	//unsigned char *data=p_sockets->p_noise_data_buf;
	//SendSocketData(&data[(m_rand_offset-num_to_send) % NOISE_DATA_LEN],num_to_send);
	unsigned char filebuf[4096];
	memset(&filebuf,0,4096);
	m_file.Read(&filebuf,num_to_send);
	SendSocketData(filebuf,num_to_send);
}

//
//
//
NoiseModuleThreadStatusData HttpSocket::ReportStatus()
{
	// Check to see if we need to whack this socket b/c i haven't sent some stuff for 1 minute
	if(IsSocket())	// if we're running
	{
		if((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalMinutes()>0)
		{
			OnClose(555);
			return m_status;
		}
	}

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
bool HttpSocket::CheckConnectionData(unsigned int ip,string filename)
{
	int i;

	// Check to see if this ip and filename already exists 2 times in the connection data vector
	ConnectionData data;
	data.m_ip=ip;
	data.m_filename=filename;

	bool ret=false;

	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		unsigned int count=0;
		for(i=0;i<p_sockets->p_connection_data->size();i++)
		{
			// Check to see how many other sockets are connected with this filename and filesize
			if(data==(*(p_sockets->p_connection_data))[i])
			{
				if((*(p_sockets->p_connection_data))[i].p_socket!=this)
				{
					count++;
				
					if(count>=2)
					{
						ret=true;
						break;
					}
				}
			}
		}

		singleLock.Unlock();
	}

	return ret;
}

//
//
//
void HttpSocket::SetConnectionData(unsigned int ip,string filename)
{
	int i;
	
	// Find the entry for this socket, if there is one.
	CSingleLock singleLock(p_sockets->p_connection_data_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		unsigned int count=0;
		for(i=0;i<p_sockets->p_connection_data->size();i++)
		{
			// Look for this socket
			if((*(p_sockets->p_connection_data))[i].p_socket==this)
			{
				(*(p_sockets->p_connection_data))[i].m_ip=ip;
				(*(p_sockets->p_connection_data))[i].m_filename=filename;
				return;
			}
		}

		// We didn't find it, so make a new one
		ConnectionData data;
		data.m_ip=ip;
		data.m_filename=filename;
		data.p_socket=this;
		p_sockets->p_connection_data->push_back(data);

		singleLock.Unlock();
	}
}

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

//
//
//
void HttpSocket::SocketDataReceived(char *data,unsigned int len)
{

}

//
//
//
void HttpSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{

}