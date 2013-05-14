// WSocket.cpp

#include "stdafx.h"
#include "WSocket.h"

//
//
//
WSocket::WSocket()
{
	m_hSocket=INVALID_SOCKET;
	m_socket_type=SOCK_STREAM;	// SOCK_STREAM or SOCK_DGRAM

	// Initialize variables for sending data
	m_sending_data=false;
	m_num_sent=0;
	m_send_data_buf_len=0;
	p_send_data_buf=NULL;

	// Initialize variables for receiving data
	m_receiving_data=false;
	m_num_read=0;
	m_read_data_buf_len=0;
	p_read_data_buf=NULL;
}

//
//
//
WSocket::~WSocket()
{
	Close();
}

//========================
// Static Member Functions
//========================

//
//
//
int WSocket::Startup()
{
	// Start Windows Sockets
	WSADATA wsdata;
	memset(&wsdata,0,sizeof(WSADATA));
	return WSAStartup(0x0202,&wsdata);
}

//
//
//
void WSocket::Cleanup()
{
	// Cleanup Windows Sockets
	WSACleanup();
}

//
//
//
int WSocket::GetLastError()
{
	return WSAGetLastError();
}

// ===

//
// Type is SOCK_STREAM or SOCK_DGRAM
//
int WSocket::Create(unsigned int port)
{
	// Create a socket handle that is not overlapped
	//m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,WSA_FLAG_OVERLAPPED);
	m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,0);

	if(m_hSocket==INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	// bind socket to the port they entered in
	sockaddr_in addr;
	ZeroMemory(&addr,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);

	// Bind the socket, in preparation for listening
	int ret=bind(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in));

	if(ret==SOCKET_ERROR)
	{
		int error=WSocket::GetLastError();
		return SOCKET_ERROR;
	}

	return ret;
}

//
// Type is SOCK_STREAM or SOCK_DGRAM
//
int WSocket::Create(unsigned short int port,unsigned int ip/*=INADDR_ANY*/)
{
	// Create a socket handle that is not overlapped
	m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,0);

	if(m_hSocket==INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	// bind socket to the port they entered in
	sockaddr_in addr;
	ZeroMemory(&addr,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
//	addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addr.sin_addr.S_un.S_addr=ip;
	addr.sin_port=htons(port);

	// Bind the socket, in preparation for listening
	int ret=bind(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in));

	return ret;
}

//
//
//
int WSocket::Connect(char *host,unsigned int port)
{
	// Check to see if we need to get the ip address from the host address
	bool is_ip_string=false;
	unsigned int ip_int[4];
	if(sscanf(host,"%u.%u.%u.%u",&ip_int[0],&ip_int[1],&ip_int[2],&ip_int[3])==4)
	{
		is_ip_string=true;
	}

	hostent *ptr=NULL;

	// If this is not an ip string we are connecting to, then resolve the hosts ip address from the DNS
	if(is_ip_string==false)
	{
		ptr=gethostbyname(host);
	}
	else	// it is just an IP, so we should just connect to it
	{
		unsigned long ip_addr=inet_addr(host);
		ptr=gethostbyaddr((char *)&ip_addr,sizeof(unsigned long),AF_INET);
	}

	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=*((unsigned long *)ptr->h_addr_list[0]);
	addr.sin_port=htons(port);

	// Connect the socket to the listening server socket
	int ret=WSAConnect(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL,NULL,NULL);

	return ret;
}

//
//
//
int WSocket::Listen()
{
	return listen(m_hSocket,SOMAXCONN);
}

//
//
//
bool WSocket::Attach(SOCKET hSocket)
{
	// If the socket is not attached, then we can attach this handle to it
	if(m_hSocket==INVALID_SOCKET)
	{
		m_hSocket=hSocket;

		// Make the socket synchrnonous (blocking)
		unsigned long arg_in=0,arg_out,num_out;
		int ret=WSAIoctl(m_hSocket,FIONBIO,&arg_in,sizeof(unsigned long),&arg_out,sizeof(unsigned long),
			&num_out,NULL,NULL);

		if(ret==SOCKET_ERROR)
		{
			int error=GetLastError();
		}

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
SOCKET WSocket::Detach()
{
	SOCKET hSocket=m_hSocket;
	m_hSocket=INVALID_SOCKET;
	return hSocket;
}

//
//
//
int WSocket::Close()
{
	int ret;

	// free data that was queued to be sent
	for(int i=0;i<v_send_data_buffers.size();i++)
	{
		delete v_send_data_buffers[i];
	}
	v_send_data_buffers.clear();

	m_sending_data=false;

	//ret=shutdown(m_hSocket,SD_BOTH);	// kludge
	ret=closesocket(m_hSocket);

	m_hSocket=INVALID_SOCKET;
	return ret;
}

//
//
//
bool WSocket::IsSocket()
{
	if(m_hSocket==INVALID_SOCKET)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//
// Functions for sending data
//

//
// Called by the user to send data
//
int WSocket::SendData(void *buf,unsigned int buf_len)
{
	// There is a vector of data buffers that are queued up to be sent
	WSocketSendData *send_data=new WSocketSendData((unsigned char *)buf,buf_len);	// Create a new send data object
	v_send_data_buffers.push_back(send_data);	// and add it to the vector

	// If not currently sending data, then send the next buffer in the vector
	if(m_sending_data==false)
	{
		return SendQueuedDataBuffers();
	}
	else	// else we are currently sending data, so we will return a 0
	{
		return 0;
	}
}

//
//
//
int WSocket::SendQueuedDataBuffers()
{
	int ret;
	
	// Keep sending the next buffer until it either goes to the OnSend or there are no more to send
	while(1)
	{
		ret=SendNextDataBuffer();
		if((ret==0)||(ret==SOCKET_ERROR))
		{
			break;
		}
	}
		
	return ret;
}

//
//
//
int WSocket::SendNextDataBuffer()
{
	// Check to see if there is a buffer to send
	if(v_send_data_buffers.size()==0)
	{
		return 0;
	}
	
	// Get next buffer to send
	vector<WSocketSendData *>::iterator send_data_iter=v_send_data_buffers.begin();
	
	// Now I am sending data
	m_sending_data=true;
	
	// Asynchronous Send that will continue in OnSend.
	m_num_sent=0;
	m_send_data_buf_len=(*send_data_iter)->GetDataLength();

	if(p_send_data_buf!=NULL)
	{
		delete [] p_send_data_buf;
		p_send_data_buf=NULL;
	}

	p_send_data_buf=new unsigned char [m_send_data_buf_len];	// this better be deleted in OnSend
	memcpy(p_send_data_buf,(*send_data_iter)->GetDataBuffer(),m_send_data_buf_len);		// copy buf to data member buffer
	
	// Remove the buffer from the vector
	delete (*send_data_iter);
	v_send_data_buffers.erase(send_data_iter);

	unsigned int num_sent;
	while(m_num_sent<m_send_data_buf_len)
	{
		// Send as much data as we can. If the buffer's full, it'll return SOCKET_ERROR and we'll go to the OnSend
		num_sent=Send((char *)p_send_data_buf+m_num_sent,m_send_data_buf_len-m_num_sent);
		
		if(num_sent==SOCKET_ERROR)
		{
			int error_code=GetLastError();
			if(error_code == WSAEWOULDBLOCK)
			{
				return SOCKET_ERROR;
			}
			else
			{
				// Something bad has happened here, like the remote socket has died or something
				delete [] p_send_data_buf;
				p_send_data_buf=NULL;
				m_sending_data=false;
				Close();

				return 0;
			}
		}

		m_num_sent+=num_sent;
	}

	// If we sent it all this time without having to go to the OnSend
	if(m_num_sent==m_send_data_buf_len)
	{
		delete [] p_send_data_buf;
		p_send_data_buf=NULL;
		m_sending_data=false;

		DataSent(m_num_sent);
	}

	return m_num_sent;	// should only be returned when we've sent all the data without having to go to the OnSend
}

//
// Not called by the user ... called by SendNextDataBuffer()
//
int WSocket::Send(void *buf,unsigned int buf_len)
{
	WSABUF wsabuf;
	wsabuf.buf=(char *)buf;
	wsabuf.len=buf_len;

	DWORD num_sent;

	int ret=WSASend(m_hSocket,&wsabuf,1,&num_sent,0,NULL,NULL);

	if(ret==SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}
	else
	{
		return (int)num_sent;
	}
}

//
// Not called by the user ... called by the asynchronous notification object
//
void WSocket::OnSend(int error_code)
{
	if(m_sending_data==false)
	{
		return;
	}
	
	// Check to see if the send transfer is complete
	if(m_num_sent==m_send_data_buf_len)
	{
		delete [] p_send_data_buf;	// free the buffer
		p_send_data_buf=NULL;
		m_sending_data=false;
		SendQueuedDataBuffers();
		return;
	}

	// If it hasn't, then there is more data to send (this should always be the case on an OnSend)
	unsigned int num_sent;
	while (m_num_sent<m_send_data_buf_len)
	{
		// Send as much data as we can. If the buffer's full, it'll return SOCKET_ERROR and we'll go to the OnSend
		num_sent=Send((char *)p_send_data_buf+m_num_sent,m_send_data_buf_len-m_num_sent);

		if(num_sent==SOCKET_ERROR)
		{
			if(GetLastError() == WSAEWOULDBLOCK)	// another OnSend will be fired later when the buffer's empty
			{
				return;
			}
			else
			{
				// Something bad has happened here, like the remote socket has died or something
				delete [] p_send_data_buf;
				p_send_data_buf=NULL;
				m_sending_data=false;
				Close();
				return;
			}
		}

		m_num_sent+=num_sent;
	}

	// If the transfer is complete, free the buffer
	if(m_num_sent==m_send_data_buf_len)
	{
		delete [] p_send_data_buf;
		p_send_data_buf=NULL;
		m_sending_data=false;

		DataSent(m_num_sent);
	
		SendQueuedDataBuffers();
//		SendNextDataBuffer();	// send the next one, if there is one
	}
}

//
//
//
void WSocket::DataSent(unsigned int len){}

//
// Functions for receiving data
//

//
// Called by the user when they get an OnReceive
//
int WSocket::ReceiveData(unsigned int buf_len)
{
	// Initialize the receiving data buffers
	m_receiving_data=true;
	m_num_read=0;
	m_read_data_buf_len=buf_len;

	if(p_read_data_buf!=NULL)
	{
		delete [] p_read_data_buf;
		p_read_data_buf=NULL;
	}

	p_read_data_buf=new unsigned char[buf_len];
	memset(p_read_data_buf,0,buf_len);

	return ReceiveBuffer();
}

//
//
//
int WSocket::ContinueToReceiveData()
{
	return ReceiveBuffer();
}

//
//
//
int WSocket::ReceiveBuffer()
{
	unsigned int num_read;
	unsigned int num_to_read;

	while(m_num_read<m_read_data_buf_len)
	{
		// Read the data in 4K at a time to avoid the NOBUFS error
		if((m_read_data_buf_len-m_num_read)>4096)
		{
			num_to_read=4096;
		}
		else
		{
			num_to_read=m_read_data_buf_len-m_num_read;
		}

		num_read=Receive(p_read_data_buf+m_num_read,num_to_read);
				
		if(num_read==SOCKET_ERROR)
		{
			int error_code=GetLastError();

			// The assumption is that not all the data was sent if we get a WSAEWOULDBLOCK, and that a later
			// OnReceive will pick the rest of it up when it is sent.
			if(error_code==WSAEWOULDBLOCK)
			{
				return 0;
			}
			else
			{
				return error_code;
			}
		}

		if(num_read==0)	// assume that this is crap
		{
			return SOCKET_ERROR;
		}
		
		m_num_read+=num_read;
	}

	DataReceived(m_num_read);

	return 0;
}

//
//
//
int WSocket::Receive(void *buf,unsigned int buf_len)
{
	WSABUF wsabuf;
	wsabuf.buf=(char *)buf;
	wsabuf.len=buf_len;

	DWORD num_read;
	DWORD flags=0;	// this variable is both an input and an output
	
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret==SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}
	else
	{
		return (int)num_read;
	}

	return ret;
}

//
// Needs to be called at the end of the overloaded functions to free memory
//
void WSocket::DataReceived(unsigned int len)
{
	// Free memory
	if(p_read_data_buf!=NULL)
	{
		delete [] p_read_data_buf;
		p_read_data_buf=NULL;
	}

	m_read_data_buf_len=0;
	m_num_read=0;

	m_receiving_data=false;
}

//
//
//
SOCKET WSocket::Accept()
{
	return WSAAccept(m_hSocket,NULL,NULL,NULL,0);
}
