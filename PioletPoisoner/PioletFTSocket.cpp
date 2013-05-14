#include "StdAfx.h"
#include "pioletftsocket.h"
#include "ConnectionSockets.h"

#define MAX_SEND_BUFFERS_ALLOWED	500		// was 1000

PioletFTSocket::PioletFTSocket(void)
{
	ResetData();
}

//
//
//
PioletFTSocket::~PioletFTSocket(void)
{
	ResetData();
}

//
//
//
//
//
//
void PioletFTSocket::InitParent(ConnectionSockets *parent)
{
	p_parent=parent;
}

//
//
//
void PioletFTSocket::ResetData()
{
	m_status=SessNew;
//	m_received_ft_starting_position_string=false;
//	m_connected=false;
//	m_md5_sent=false;
//	m_connecting=false;
	m_file_length=0;
	m_md5.erase();
	m_starting_at=0;

	m_file_len=0;
	m_file_num_sent=0;
	m_rand_offset=0;
//	m_connection_keep_alive = true;
	memset(m_buf,0,sizeof(m_buf));
	m_buf_offset=0;

/*	// If we never got any data from this fool, it is bogus
	if(m_status.m_end==0)
	{
		p_sockets->m_num_bogus_connections++;
	}

	m_status.Clear();
*/
}

//
//
//
/*
int PioletFTSocket::Connect(char* ip, int port, UINT file_length, const char* md5)
{
	m_connecting = true;
	m_start_connecting_at = CTime::GetCurrentTime();
	m_file_length = file_length;
	m_md5 = md5;

	int ret=FtTEventSocket::Connect(ip,port);
	if(ret < 0)
	{
		int err=WSAGetLastError();
//		m_connecting = false;
	}

	return ret;
}
*/

//
//
//
/*
int PioletFTSocket::CreateSocket(char* ip, int port, UINT file_length, const char* md5)
{
	Close();
//	m_connecting = true;
//	m_start_connecting_at = CTime::GetCurrentTime();
	m_file_length = file_length;
	m_md5 = md5;

	int ret=FtTEventSocket::Create(port);
	if(ret < 0)
	{
		int err=WSAGetLastError();
//		m_connecting = false;
	}
	Listen();

	return ret;
}
*/

//
//
//
bool PioletFTSocket::AcceptConnection(SOCKET hSocket, char* ip, int port, UINT file_length, const char* md5)
{
	ResetData();
	m_file_length = file_length;
	m_md5 = md5;
	m_remote_ip = ip;
	m_local_port = port;

	//m_status.m_start_time=CTime::GetCurrentTime();
	//m_last_time_i_sent_stuff=CTime::GetCurrentTime();

	bool ret=TEventSocket::Attach(hSocket);
	TRACE("Port: %d - Connection Accepted\n",m_local_port);
	
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
	
	char len[32];
	ultoa(m_file_length, len, 10);
	char* buf = new char[5+strlen(len)];
	memset(buf, 0, 5+strlen(len));
	buf[0]='S';
	buf[1]='I';
	buf[2]='Z';
	buf[3]=32;
	buf[4]=32;
	memcpy(&buf[5], len, strlen(len));

	m_status=SessFilesizeSent;
	ret = SendData(buf,5+strlen(len));
	if(ret == SOCKET_ERROR)
	{
		int err=GetLastError();
		TRACE("Port: %d - Socket error on SendData(SIZ) Error: %d\n",m_local_port,err);
	}

	delete [] buf;

	m_last_time_i_sent_stuff = CTime::GetCurrentTime();
	return ret;
}

//
//
//
int PioletFTSocket::Close()
{
	ResetData();
	return TEventSocket::Close();
}

//
//
//
/*
void PioletFTSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}
//	m_connected = true;

	char len[32];
	ultoa(m_file_length, len, 10);
	char* buf = new char[5+strlen(len)];
	memset(buf, 0, 5+strlen(len));
	buf[0]='S';
	buf[1]='I';
	buf[2]='Z';
	buf[3]=32;
	buf[4]=32;
	memcpy(&buf[5], len, strlen(len));
	SendData(buf,5+strlen(len));
	delete [] buf;
}
*/

//
//
//
/*
void PioletFTSocket::OnAccept(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}
	SOCKET socket = Accept();
	TEventSocket::Attach(socket);
	m_status=SessConnected;
		


//	m_connected = true;

	char len[32];
	ultoa(m_file_length, len, 10);
	char* buf = new char[5+strlen(len)];
	memset(buf, 0, 5+strlen(len));
	buf[0]='S';
	buf[1]='I';
	buf[2]='Z';
	buf[3]=32;
	buf[4]=32;
	memcpy(&buf[5], len, strlen(len));
	SendData(buf,5+strlen(len));
	delete [] buf;

	m_status=SessFilesizeSent;

}
*/
//
//
//
/*
void PioletFTSocket::SocketDataSent(unsigned int len)
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
*/
//
//
//
void PioletFTSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		OnClose(401);
		return;
	}

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

	if(m_status==SessFilesizeSent && strstr((char*)m_buf, "STR")!=NULL)
	{
		char* ptr = (char*)m_buf;
		ptr+=4;
		sscanf(ptr, "%u", &m_starting_at);

		int buf_len = 4 + m_md5.length();
		char* buf = new char[buf_len];
		memset(buf, 0, buf_len);
		buf[0]='M';
		buf[1]='D';
		buf[2]='5';
		buf[3]=32;
		strncpy(&buf[4], m_md5.c_str(), m_md5.length());
		//char buf[] = {'M','D','5',32,'C','6','2','C','C','A','B','1','2','4','0','4','A','8','2','3','E','C','9','4','8','0','4','5','C',
		//				'C','6','2','0','E','F','1'};
		TRACE("Port: %d Received msg: %s\n",m_local_port,m_buf);
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;

		m_status = SessStartPositionReceived;
		int ret = SendData(buf,buf_len);
		delete [] buf;
		if(ret == SOCKET_ERROR)
		{
			int err=GetLastError();
			TRACE("Port: %d - Socket error on SendData(MD5) Error: %d\n",m_local_port,err);
		}
		m_file_len=m_file_length-m_starting_at+1;
		m_file_num_sent=0;

		// Set the rand offset to be the starting byte
		m_rand_offset=m_starting_at;
		m_last_time_i_sent_stuff = CTime::GetCurrentTime();
		return;
	}
	if(m_status==SessStartPositionReceived && strstr((char*)m_buf, "GO!!")!= NULL) //should receive "GO!!" here
	{
		m_status=SessGoReceived;
		TRACE("Port: %d Received msg: %s\n",m_local_port,m_buf);
		memset(m_buf,0,sizeof(m_buf));
		m_buf_offset=0;
		SendFrames();
		return;
	}
}

//
//
//
void PioletFTSocket::OnClose(int error_code)
{
	TRACE("Port: %d disconnected - Error code:%d  - File length: %d Bytes to send: %d Bytes sent: %d \n",m_local_port,error_code,m_file_length,m_file_len,m_file_num_sent);
	m_status=SessDisconnected;
	ResetData();
	TEventSocket::OnClose(error_code);
}

//
// called when data has been sent
//
void PioletFTSocket::DataSent(unsigned int len)
{
	// Check to see if the file length is zero. If so, then exit
	if(m_status != SessGoReceived)
		return;
	if(m_file_len==0)
	{
		return;
	}
	m_file_num_sent+=len;
	//TRACE("Port: %d SendFrames - File length: %d Bytes to send: %d Bytes sent: %d Random offset: %d\n",m_local_port,m_file_length,m_file_len,m_file_num_sent,m_rand_offset);

	if(m_file_len>m_file_num_sent)
	{
		SendFrames();
	}
	else
	{
		TRACE("Port: %d Sending finished - File length: %d Bytes to send: %d Bytes sent: %d \n",m_local_port,m_file_length,m_file_len,m_file_num_sent);
		OnClose(0);
	}
}

//
//
//
void PioletFTSocket::SendFrames()
{
	unsigned int num_to_send=4096;
	if(num_to_send>(m_file_len-m_file_num_sent))
	{
		num_to_send=m_file_len-m_file_num_sent;
	}


	// Update the last time we sent stuff
	m_last_time_i_sent_stuff=CTime::GetCurrentTime();
	
	// Increment offset in case we call anouther SendFrames() function before returning from this one
	m_rand_offset+=num_to_send;

	unsigned char *data=p_parent->p_noise_data_buf;
	int ret = SendData(&data[(m_rand_offset-num_to_send) % NOISE_DATA_LEN],num_to_send);
	/*
	if(ret == SOCKET_ERROR)
	{
		int err=GetLastError();
		TRACE("Port: %d - Socket error on SendData() Error: %d Data Sent: %d bytes\n",m_local_port,err,m_file_num_sent);
	}
	*/
}

//
//
//
/*
void PioletFTSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
		// Check to see if we're read in the 4K...if so, then close the socket...this is too big of a buffer
		if(data_len==4096)
		{
			Close();
		}
		else	// else we have not filled the buffer yet, so receive some more data
		{

			if(strstr(data, "STR")!=NULL)
			{
				char* ptr = data;
				ptr+=4;
				sscanf(ptr, "%u", &m_starting_at);

				int buf_len = 4 + m_md5.length();
				char* buf = new char[buf_len];
				memset(buf, 0, buf_len);
				buf[0]='M';
				buf[1]='D';
				buf[2]='5';
				buf[3]=32;
				strncpy(&buf[4], m_md5.c_str(), m_md5.length());
				//char buf[] = {'M','D','5',32,'C','6','2','C','C','A','B','1','2','4','0','4','A','8','2','3','E','C','9','4','8','0','4','5','C',
				//				'C','6','2','0','E','F','1'};
				SendSocketData(buf,buf_len);
				delete [] buf;
				m_md5_sent=true;
			}
			else if(m_md5_sent && strstr(data, "GO!!")!= NULL) //should receive "GO!!" here
			{
				UINT buf_len = m_file_length-m_starting_at+1;
				char* poison_data = new char[buf_len];
				for(UINT i=0;i<buf_len;i++)
					poison_data[i]=rand()%256;
				SendSocketData(poison_data,buf_len);
				delete [] poison_data;
			}
		}
		return;
}
*/
//
//
//
/*
void PioletFTSocket::SocketDataReceived(char *data,unsigned int len)
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
*/
	/*
	if(m_connected) //should receive starting position here
	{
		char buf[] = {'M','D','5',32,'4','F','6','1','2','9','4','5','4','7','3','6','B','7','B','6','E','3','C','C','E','E','6','5','1',
			'B','9','3','8','F','D','C'};
		SendSocketData(buf,sizeof(buf));
		m_md5_sent=true;
	}
	else if(m_md5_sent) //should receive "GO!!" here
	{
		
	}
	*/
//}

//
//
//
void PioletFTSocket::OnSend(int error_code)
{
	WSocket::OnSend(error_code);
}

//
//
//
void PioletFTSocket::CloseIdle()
{
	if(IsSocket())
	{
		if((CTime::GetCurrentTime()-m_last_time_i_sent_stuff).GetTotalMinutes()>0)
		{
			OnClose(555);
		}
	}
}