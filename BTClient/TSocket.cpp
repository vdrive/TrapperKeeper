// TSocket.cpp

#include "stdafx.h"
#include "TSocket.h"

//
//
//
TSocket::TSocket()
{
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
SOCKET TSocket::Detach()
{
	// Reset variables for sending data
	ResetSendDataMembers();

	// Reset variables for receiving data
	ResetReceiveDataMembers();

	// Free any remaining send buffers
	for(unsigned int i=0;i<v_send_data_buffers.size();i++)
	{
		delete v_send_data_buffers[i];
	}
	v_send_data_buffers.clear();

	return WSocket::Detach();
}


//
// Functions for sending data
//

//
// Called by the user to send data
//
int TSocket::SendSocketData(unsigned char *buf,unsigned int buf_len)
{
	// There is a vector of data buffers that are queued up to be sent
	TSocketSendData *send_data=new TSocketSendData(buf,buf_len);	// Create a new send data object
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
int TSocket::Close()
{
	// Reset variables for sending data
	ResetSendDataMembers();

	// Reset variables for receiving data
	ResetReceiveDataMembers();

	// Free any remaining send buffers
	for(unsigned int i=0;i<v_send_data_buffers.size();i++)
	{
		delete v_send_data_buffers[i];
	}
	v_send_data_buffers.clear();

	return WSocket::Close();
}

int TSocket::ErrorClose()
{
	// Reset variables for sending data
	LogMessage("ResettingSendDataMembers");
	ResetSendDataMembers();

	// Reset variables for receiving data
	LogMessage("ResettingReceiveDataMembers");
	ResetReceiveDataMembers();

	// Free any remaining send buffers
	LogMessage("Entering loop to delete send data buffers");
	for(unsigned int i=0;i<v_send_data_buffers.size();i++)
	{
		delete v_send_data_buffers[i];
	}
	LogMessage("clearing send data buffers");
	v_send_data_buffers.clear();

	LogMessage("Calling WSocket Close");
	int ret = WSocket::Close();
	char message[256+1];
	sprintf(message,"WSocket Close returned %d",ret);
	LogMessage(message);
	return ret;
}

//
//
//
int TSocket::SendQueuedDataBuffers()
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
int TSocket::SendNextDataBuffer()
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
			// Additional error trap if the socket has died before the send -Ben
			/*
			else if(error_code == WSAENOTSOCK)
			{
				return 0;
			}
			*/
			else
			{
				char msg[256+1];
				sprintf(msg,"SOCKET_ERROR with error code %d",error_code);
				LogMessage(msg);
				// Something bad has happened here, like the remote socket has died or something
				ErrorClose();
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
int TSocket::Send(void *buf,unsigned int buf_len)
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
void TSocket::OnSend(int error_code)
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
void TSocket::ResetSendDataMembers()
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
int TSocket::ReceiveSocketData(unsigned int buf_len)
{
	if(p_read_data_buf!=NULL)
	{
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
int TSocket::ReceiveSomeSocketData(unsigned int buf_len)
{
	if(p_read_data_buf!=NULL)
	{
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
int TSocket::ReceiveSomeMoreSocketData(char *data,unsigned int len,unsigned int max_len)
{
	if(p_read_data_buf!=NULL)
	{
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
int TSocket::ContinueToReceiveSocketData()
{
	return ReceiveBuffer();
}

//
//
//
int TSocket::ReceiveBuffer()
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
int TSocket::Receive(void *buf,unsigned int buf_len)
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
void TSocket::ResetReceiveDataMembers()
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

void TSocket::LogMessage(char * message)
{

	CFile logfile;
	if (logfile.Open("TSocketLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone)== TRUE)
	{
		if(logfile.GetLength()> 100000000)
		{
			logfile.Close();
			remove("TSocketLog.txt");
			logfile.Open("TSocketLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite);
		}
		logfile.SeekToEnd();
		logfile.Write(message, (unsigned int)strlen(message));
		logfile.Write("\r\n",2);
		logfile.Close();
	}
	
}

//
// Virtuals
//
void TSocket::SocketDataSent(unsigned int len){}
void TSocket::SocketDataReceived(char *data,unsigned int len){}
void TSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len){}