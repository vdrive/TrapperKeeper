// HttpSocket.cpp

#include "stdafx.h"
#include "HttpSocket.h"
#include "SupernodeDistributerDll.h"

//
//
//
HttpSocket::HttpSocket()
{
	p_parent=NULL;

	memset(m_ip,0,sizeof(m_ip));
	m_port=0;
}

//
//
//
HttpSocket::~HttpSocket()
{
}

//
//
//
void HttpSocket::InitParent(SupernodeDistributerDll *parent)
{
	p_parent=parent;
}

//
//
//
int HttpSocket::Connect(char *host,unsigned short int port, CTime start_time)
{
	strcpy(m_ip,host);
	m_port=port;
	m_start_time = start_time;

	return TAsyncSocket::Connect(host,port);
}

//
//
//
void HttpSocket::OnConnect(int error_code)
{
	// Check for connection error
	if(error_code!=0)
	{
		p_parent->ReportStatus(this,m_ip,m_port,error_code);
		return;
	}

	// Send the HTTP GET request
	string str;
	str+="GET / HTTP/1.1\r\n";
	str+="Host: ";
	str+=m_ip;
	str+="\r\n";
	str+="\r\n";

	SendSocketData((unsigned char *)str.c_str(),(unsigned int)str.size());
/*	
	char buf[1024];
	memset(buf,0,sizeof(buf));
	strcpy(buf,str.c_str());

	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=(unsigned long)strlen(buf);

	DWORD sent=0;

	int ret=WSASend(WAsyncSocket::m_hSocket,&wsabuf,1,&sent,0,NULL,NULL);
*/
}

//
//
//
void HttpSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		p_parent->ReportStatus(this,m_ip,m_port,error_code);
		return;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	ReceiveSomeSocketData(4096);

	//
	// Check to see if there is any more data waiting to be read. KLUDGE (???)
	//

	char buf[1];

	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;	// this variable is both an input and an output
	
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			OnReceive(0);	// bring on the recursion
		}
	}

}

//
//
//
void HttpSocket::OnClose(int error_code)
{
	TAsyncSocket::OnClose(error_code);

	// Check for connection error
	if(error_code!=0)
	{
		return;
	}
}

//
//
//
void HttpSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	// Check to see if we've not gotten the string "\r\n\n". If not, then look for "</html>")
	char* ptr = strstr((char *)data,"\r\n\n");
	if(ptr == NULL)
		 ptr = strstr((char*)data, "</html>");
	
	if(ptr==NULL)
	{
		// Check to see if we're read in the 4K...if so, then try to look for the string "Content-Type: text/html" andclose the socket...
		if(data_len==4096)
		{
			ptr = strstr((char *)data, "Content-Type: text/html");
			if(ptr == NULL)
				p_parent->ReportStatus(this,m_ip,m_port,1);
			else
				ptr[0] = '\0'; //cut off the remaining string after "Content-Type: text/html"

				//MessageBox(NULL,data,"Socket data",MB_OK);
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
		if(ptr == NULL)
			return;
	}

	// Extract the Username, SuperNode IP and Port
	char username[256+1];
	char ip[32];
	char port[10];
	memset(username,0,sizeof(username));
	memset(ip,0,sizeof(ip));
	memset(port,0,sizeof(port));

	// Extract the username
	if(strstr(data,"X-Kazaa-Username: ")!=NULL)
	{
		strcpy(username,strstr(data,"X-Kazaa-Username: ")+strlen("X-Kazaa-Username: "));
		*strstr(username,"\r\n")='\0';
	}
	else
	{
		p_parent->ReportStatus(this,m_ip,m_port,1);
		return;
	}

	// Extract the supernode ip and port
	if(strstr(data,"X-Kazaa-SupernodeIP: ")!=NULL)
	{
		strcpy(ip,strstr(data,"X-Kazaa-SupernodeIP: ")+strlen("X-Kazaa-SupernodeIP: "));
		strcpy(port,strstr(ip,":")+1);

		*strstr(ip,":")='\0';
		*strstr(port,"\r\n")='\0';
	}
	p_parent->ReportStatus(this,m_ip,m_port,0,username,ip,port);
}

//
//
//
void HttpSocket::TimeOut()
{
	p_parent->ReportStatus(this,m_ip,m_port,SOCKET_ERROR);
}