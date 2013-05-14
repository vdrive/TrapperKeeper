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
	p_send_data_buf=NULL;
	ResetSendDataMembers();

	// Initialize variables for receiving data
	p_read_data_buf=NULL;
	ResetReceiveDataMembers();
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

// ===

//
// Type is SOCK_STREAM or SOCK_DGRAM
//
int WSocket::Create(unsigned int port)
{
	// Create a socket handle that is not overlapped
	m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,WSA_FLAG_OVERLAPPED);

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

	return ret;
}

//
//
//
int WSocket::Close()
{
	int ret;

	// Free the socket handle and reset it
	ret=closesocket(m_hSocket);
	m_hSocket=INVALID_SOCKET;

	// Reset variables for sending data
	ResetSendDataMembers();

	// Reset variables for receiving data
	ResetReceiveDataMembers();

	// Free any remaining send buffers
	for(UINT i=0;i<v_send_data_buffers.size();i++)
	{
		delete v_send_data_buffers[i];
	}
	v_send_data_buffers.clear();

	return ret;
}

// ===

//
//
//
int WSocket::Connect(char *host,unsigned int port)
{
/*
	// Check to see if we need to get the ip address from the host address
	bool is_ip_string=false;
	unsigned int ip_int[4];
	if(sscanf(host,"%u.%u.%u.%u",&ip_int[0],&ip_int[1],&ip_int[2],&ip_int[3])==4)
	{
		is_ip_string=true;
	}
*/
	HOSTENT *hostent=NULL;

	// If this is not an ip string we are connecting to, then resolve the hosts ip address from the DNS
//	if(is_ip_string==false)
//	{
		hostent=gethostbyname(host);
//	}
//	else	// it is just an IP, so we should just connect to it
//	{
//		unsigned long ip_addr=inet_addr(host);
//		hostent=gethostbyaddr((char *)&ip_addr,sizeof(unsigned long),AF_INET);
//	}

	// Check for error
	if(hostent==NULL)
	{
		Close();
		return SOCKET_ERROR;
	}

	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=*((unsigned long *)hostent->h_addr_list[0]);
	addr.sin_port=htons(port);

	// Connect the socket to the listening server socket
	int ret=WSAConnect(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL,NULL,NULL);

	return ret;
}

//
// Functions for sending data
//

//
// Called by the user to send data
//
int WSocket::SendSocketData(void *buf,unsigned int buf_len)
{
	// There is a vector of data buffers that are queued up to be sent
	WSocketSendData *send_data=new WSocketSendData((unsigned char *)buf,buf_len);	// Create a new send data object
	v_send_data_buffers.push_back(send_data);	// and add it to the vector

	// If not currently sending data, then send the next buffer in the vector
	if(m_sending_socket_data==false)
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
	
	// Now I am sending data
	m_sending_socket_data=true;
	
	// Asynchronous Send that will continue in OnSend.
	m_send_data_buf_len=v_send_data_buffers[0]->GetDataLength();

	if(p_send_data_buf!=NULL)
	{
		OutputDebugString("p_send_data_buf != NULL in WSocket::SendNextDataBuffer\n");
		delete [] p_send_data_buf;
		p_send_data_buf=NULL;
	}

	p_send_data_buf=new char [m_send_data_buf_len];	// this better be deleted in OnSend
	memcpy(p_send_data_buf,v_send_data_buffers[0]->GetDataBuffer(),m_send_data_buf_len);		// copy buf to data member buffer
	
	// Remove the buffer from the vector
	delete v_send_data_buffers[0];
	v_send_data_buffers.erase(v_send_data_buffers.begin());

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
				Close();
				return 0;
			}
		}

		m_num_sent+=num_sent;
	}

	int ret=m_num_sent;

	// If we sent it all this time without having to go to the OnSend
	if(m_num_sent==m_send_data_buf_len)
	{
		SocketDataSent(m_num_sent);
		ResetSendDataMembers();	// free memory
	}

	return ret;	// should only be returned when we've sent all the data without having to go to the OnSend
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
	if(m_sending_socket_data==false)
	{
		return;
	}
	
	// Check to see if the send transfer is complete
	if(m_num_sent==m_send_data_buf_len)
	{
		SocketDataSent(m_num_sent);
		ResetSendDataMembers();	// free memory

		SendQueuedDataBuffers();	// see if there are more data buffers waiting to be sent
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
				Close();
				return;
			}
		}

		m_num_sent+=num_sent;
	}

	// If the transfer is complete, free the buffer
	if(m_num_sent==m_send_data_buf_len)
	{
		SocketDataSent(m_num_sent);
		ResetSendDataMembers();	// free memory

		SendQueuedDataBuffers();	// see if there are more data buffers waiting to be sent
	}
}
/*
//
// Needs to be called at the end of the overloaded functions to free memory
//
void WSocket::SocketDataSent(unsigned int len)
{
	ResetSendDataMembers();
}
*/
//
// Not called by the user...called by DataSent() and Close()
//
void WSocket::ResetSendDataMembers()
{
	// Reset send data members
	m_sending_socket_data=false;
	m_num_sent=0;
	m_send_data_buf_len=0;
	if(p_send_data_buf!=NULL)
	{
		delete [] p_send_data_buf;
		p_send_data_buf=NULL;
	}
}

//
// Functions for receiving data
//

//
// Called by the user when they get an OnReceive and they know how much data they want to read
//
int WSocket::ReceiveSocketData(unsigned int buf_len)
{
	if(p_read_data_buf!=NULL)
	{
		OutputDebugString("p_read_data_buf != NULL in WSocket::ReceiveSocketData()\n");
		delete p_read_data_buf;
		p_read_data_buf=NULL;
	}

	// Initialize the receiving data buffers
	m_receiving_socket_data=true;
	m_read_data_buf_len=buf_len;
	p_read_data_buf=new char[buf_len];
	memset(p_read_data_buf,0,buf_len);
	return ReceiveBuffer();
}

//
// Called by the user when they get an OnReceive and they don't know how much data they want to read, so the buf_len is the max
//
int WSocket::ReceiveSomeSocketData(unsigned int buf_len)
{
	if(p_read_data_buf!=NULL)
	{
		OutputDebugString("p_read_data_buf != NULL in WSocket::ReceiveSomeSocketData()\n");
		delete p_read_data_buf;
		p_read_data_buf=NULL;
	}

	// Initialize the receiving data buffers
	m_receiving_some_socket_data=true;
	m_read_data_buf_len=buf_len;
	p_read_data_buf=new char[buf_len];
	memset(p_read_data_buf,0,buf_len);
	return ReceiveBuffer();
}

//
//
//
int WSocket::ReceiveSomeMoreSocketData(char *data,unsigned int len,unsigned int max_len)
{
	if(p_read_data_buf!=NULL)
	{
		OutputDebugString("p_read_data_buf != NULL in WSocket::ReceiveSomeMoreSocketData()\n");
		delete [] p_read_data_buf;
		p_read_data_buf=NULL;
	}

	// Append any new data to this existing data already read in
	m_receiving_some_socket_data=true;
	m_read_data_buf_len=max_len;
	m_num_read=len;
	p_read_data_buf=new char[max_len];
	memcpy(p_read_data_buf,data,len);

	return ReceiveBuffer();
}

//
//
//
int WSocket::ContinueToReceiveSocketData()
{
//	OutputDebugString("WSocket::ContinueToReceiveSocketData()\n");

	return ReceiveBuffer();
}

//
//
//
int WSocket::ReceiveBuffer()
{
	unsigned int new_len=0;	// the amount read this function call

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
				// If receiving some data, then see if this is enough.
				if(m_receiving_some_socket_data)
				{
					char *data=new char[m_num_read+1];
					memset(data,0,m_num_read+1);
					memcpy(data,p_read_data_buf,m_num_read);
					unsigned int data_len=m_num_read;
					unsigned int max_len=m_read_data_buf_len;

					// Reset the receive data structures
					ResetReceiveDataMembers();
				
					SomeSocketDataReceived(data,data_len,new_len,max_len);
				
					delete [] data;
				}
				else	// else we are waiting for a specific amount of data...so wait for next OnReceive()
				{
					// Do nothing
				}
				
				return 0;
			}
			else
			{
				// Something very bad happened
				Close();
				return error_code;
			}
		}

		if(num_read==0)
		{
			// Check to see if we are reading a fixed size, or if we are receiving "some" data
			if(m_receiving_some_socket_data)
			{
				char *data=new char[m_num_read+1];
				memset(data,0,m_num_read+1);
				memcpy(data,p_read_data_buf,m_num_read);
				unsigned int data_len=m_num_read;
				unsigned int max_len=m_read_data_buf_len;

				// Reset the receive data structures
				ResetReceiveDataMembers();
				
				SomeSocketDataReceived(data,data_len,new_len,max_len);
				
				delete [] data;

				return 0;
			}
			else
			{
				return SOCKET_ERROR;
			}
		}
		
		new_len+=num_read;

		m_num_read+=num_read;
	}

	// Check to see if we are reading a fixed size, or if we are receiving "some" data
	if(m_receiving_some_socket_data)
	{
		char *data=new char[m_num_read+1];
		memset(data,0,m_num_read+1);
		memcpy(data,p_read_data_buf,m_num_read);
		unsigned int data_len=m_num_read;
		unsigned int max_len=m_read_data_buf_len;

		// Reset the receive data structures
		ResetReceiveDataMembers();
				
		SomeSocketDataReceived(data,data_len,new_len,max_len);
				
		delete [] data;
	}
	else
	{
		char *data=new char[m_num_read+1];
		memset(data,0,m_num_read+1);
		memcpy(data,p_read_data_buf,m_num_read);
		unsigned int data_len=m_num_read;

// temp - begin
		bool mrsd=m_receiving_socket_data;
		bool mrssd=m_receiving_some_socket_data;
		unsigned int nr=m_num_read;
		unsigned int mrdbl=m_read_data_buf_len;
// temp - end

		// Reset the receive data structures
		ResetReceiveDataMembers();
				
		SocketDataReceived(data,data_len);
				
		delete [] data;
	}

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
/*
//
//
//
void WSocket::SocketDataReceived(char *data,unsigned int len)
{
}

//
//
//
void WSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
}
*/
//
// Not called by the user ... called by DataReceived() and Close()
//
void WSocket::ResetReceiveDataMembers()
{
	// Reset send data members
	m_receiving_socket_data=false;
	m_receiving_some_socket_data=false;
	m_num_read=0;
	m_read_data_buf_len=0;
	if(p_read_data_buf!=NULL)
	{
		delete [] p_read_data_buf;
		p_read_data_buf=NULL;
	}
}
