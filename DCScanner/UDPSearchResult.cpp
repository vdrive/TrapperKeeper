#include "StdAfx.h"
#include "udpsearchresult.h"

UDPSearchResult::UDPSearchResult(void)
{
	USocket::Startup();	//start up the socket
	int create_result=this->Create(rand());
	if(create_result!=0)	//create the socket
	{
		::AfxMessageBox("Error: Could not create socket");
	}
}
UDPSearchResult::~UDPSearchResult(void)
{
}
void UDPSearchResult::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		::AfxMessageBox("Error Connecting");
	}
	else
	{
		::AfxMessageBox("Sucessfully Connected");
	}
}

void UDPSearchResult::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		::AfxMessageBox("Error Receiving Data");
		return;	//return due to error
	}
	else
	{
		char buffer[1024];
		unsigned int ip;
		unsigned short port;
		unsigned int num_read;
		int result=ReceiveFrom(buffer,1024,&ip,&port,&num_read);
		if(result==0)
		{
			buffer[num_read]='\0';	//add end to string
			char * ping = strstr(buffer,"$Ping");
			//found ping request
			if(ping !=NULL)
			{
				CString ping = "$Pong";
				this->SendTo(ping.GetBuffer(0),strlen(ping.GetBuffer(0)),ip,port);
			}
			TRACE("\nBUFFER: %s",buffer);
		}
	}	
}

int UDPSearchResult::ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short *port,unsigned int*num_read)
{
	return USocket::ReceiveFrom(data,len,ip,port,num_read);
}

void UDPSearchResult::OnSend(int error_code)
{
	
}
int UDPSearchResult::SendTo(void *data,unsigned int len,unsigned int ip,unsigned short port)
{
	return UAsyncSocket::SendTo(data,len,ip,port);
}