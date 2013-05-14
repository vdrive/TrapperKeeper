// HttpSocket.cpp

#include "stdafx.h"
#include "HttpSocket.h"
#include "FrameHeader.h"
#include "NoiseSockets.h"
#include "ConnectionData.h"
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
	m_state=Idle;
}

HttpSocket::~HttpSocket()
{
	Close();
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
			return false;
		}
	}
	else
	{
		OnClose(302);
		return false;
	}

	unsigned char init[1];
	init[0]=0x31;
	SendSocketData(init,1);
	return true;
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

	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;

	// If we never got any data from this fool, it is bogus
	if(m_status.m_end==0)
	{
		p_sockets->m_num_bogus_connections++;
	}

	m_status.Clear();
	m_state=Idle;

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

	if(m_state==Filesize_Sent)
	{
		// Read in up to 8 bytes of data
		while(1)
		{
			int num_read=Receive(&m_buf[m_buf_offset],8-m_buf_offset);
			if(num_read==SOCKET_ERROR ||num_read==0)
			{
				break;
			}
			m_buf_offset+=num_read;
		}
	}
	else
	{
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
	}

	// Check to see if we've read in a full 4K...b/c we shouldn't get that far
	if(m_buf_offset==sizeof(m_buf))
	{
		OnClose(402);
		return;
	}
	
	if(m_state==Init_Sent)
	{
		if(m_buf_offset <= 3)
		{
			return;
		}
		// Check to see if we've not gotten the "GET". If not, then return
		if(strstr((char *)m_buf,"GET")==NULL)
		{
			OnClose(403);
			return;
		}

		// Extract usernamee
		char* ptr = (char*)(&m_buf[0]+3);
		char* space = strchr(ptr,' ');
		if(space==NULL)
		{
			OnClose(404);
			return;
		}
		space[0] = '\0';
		m_status.m_client=ptr;
		ptr+=m_status.m_client.GetLength();
		ptr+=2; //advance to skip the quote

		//Extract filename
		m_status.m_filename=ptr;
		m_status.m_filename.Delete(m_status.m_filename.GetLength()-3,3);
/*
		// Check to see if we need to close this connection if we uploaded 512KB to this ip with this hash already
		if(CheckConnectionData(ReturnRemoteIPAddress(),m_status.m_filename))
		{
			OnClose(401);
			return;
		}
		else
		{
			SetConnectionData(ReturnRemoteIPAddress(),m_status.m_filename);
		}
		*/

		// Everything is cool
		p_sockets->m_num_good_connections++;
		
		UINT total_size = p_sockets->p_fs_manager->GetFileSize(m_status.m_filename);
		m_status.m_total_size = total_size;
		if(total_size > 0)
		{
			m_state=Filename_Received;
			char buf[32];
			ultoa(total_size,buf,10);
			SendSocketData((unsigned char*)buf,strlen(buf));
			memset(m_buf,0,sizeof(m_buf));
			m_buf_offset=0;
			return;
		}
		else
		{
			OnClose(405);
			return;
		}
	}
	else if(m_state==Filesize_Sent)
	{
		UINT start,end;
		start=end=0;
		start=*(UINT*)m_buf;
		end=*(UINT*)&m_buf[4];
		if( (end-start) > 0)
		{
			m_status.m_start=start;
			m_status.m_end=end;
			m_file_len=end-start;
			m_file_num_sent=0;
			// Set the rand offset to be the starting byte
			m_rand_offset=start;
			m_state=Range_Received;
			unsigned char buf[8];
			memcpy(buf,m_buf,8);
			memset(m_buf,0,sizeof(m_buf));
			m_buf_offset=0;
			SendSocketData(buf,sizeof(buf));
			return;
		}
		else
		{
			memset(m_buf,0,sizeof(m_buf));
			m_buf_offset=0;
			//Close();
			return;
		}
	}
}

//
//
//
void HttpSocket::OnClose(int error_code)
{
#ifdef _DEBUG
	TRACE("Socket closed. Error code: %d State: %d Range: %d - %d\n",
		error_code,m_state,m_status.m_start,m_status.m_end);
#endif
	Close();	// just in case

	TEventSocket::OnClose(error_code);	// does nothing...yay!
}

//
// called when data has been sent
//
void HttpSocket::SocketDataSent(unsigned int len)
{
	if(m_state==Idle)
	{
		m_state=Init_Sent;
		return;
	}
	if(m_state==Filename_Received)
	{
		m_state=Filesize_Sent;
		return;
	}
	// Check to see if the file length is zero. If so, then exit
	if(m_state==Range_Received)
	{
		if(m_file_len==0)
		{
			OnClose(406);
			return;
		}
		/*
		if(m_file_num_sent > 20971520) //we have sent 20MB already, close the connection
		{
			OnClose(0);
			return;
		}
		*/
		if(m_file_len>m_file_num_sent)
		{
			SendFrames();
		}
		else
		{
			OnClose(407);
		}
	}
	if(m_state==Frames_Sent)
	{
		m_file_len=0;
		m_file_num_sent=0;
		m_rand_offset=0;
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;
		m_state=Filesize_Sent;
		//Sleep(50);
	}
}

//
//
//
void HttpSocket::SendFrames()
{
// Figure out how much to send
	unsigned int num_to_send=4096;
	if(num_to_send>(m_file_len-m_file_num_sent))
	{
		num_to_send=m_file_len-m_file_num_sent;
	}
	m_file_num_sent+=num_to_send;

	// Update the last time we sent stuff
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();
	
	// Increment offset in case we call anouther SendFrames() function before returning from this one
	m_rand_offset+=num_to_send;

	unsigned char *data=p_sockets->p_noise_data_buf;
	m_state = Frames_Sent;

	SendSocketData(&data[(m_rand_offset-num_to_send) % NOISE_DATA_LEN],num_to_send);
}

//
//
//
NoiseModuleThreadStatusData HttpSocket::ReportStatus()
{
	// Check to see if we need to whack this socket b/c i haven't sent some stuff for 1 minute
	if(IsSocket())	// if we're running
	{
		/*
		if(((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalSeconds()>10) && m_state==Init_Sent)
		{ //resend init byte
			unsigned char init[1];
			init[0]=0x31;
			m_last_time_i_sent_stuff=CTime::GetCurrentTime();
			SendSocketData(init,1);
		}
		*/
		if((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalMinutes()>0)
		{
			OnClose(555);
			//TRACE("Closing timeout connection (1 min)\n");
			//p_sockets->Log("Closing timeout connection (1 min)\n");
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
bool HttpSocket::CheckConnectionData(unsigned int ip,const char * filename)
{
	UINT i;

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
void HttpSocket::SetConnectionData(unsigned int ip,const char* filename)
{
	UINT i;
	
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