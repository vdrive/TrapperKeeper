#include "StdAfx.h"
#include "hublistcollector.h"
#include "DCSupplyDlg.h"
#include <list>

hublistcollector::hublistcollector(void)
{
}

hublistcollector::~hublistcollector(void)
{
}

void hublistcollector::InitDlg(DCSupplyDlg* dlg)
{
	p_dlg=dlg;
	m_data="";
}

void hublistcollector::OnReceive(int error_code)
{
	//error checking
	if(error_code!=0)
	{
		p_dlg->StatusUpdate("Error Receiving Data");
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	ReceiveSomeSocketData(1516);

	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			OnReceive(0);	// recursive call
		}
	}
}

void hublistcollector::OnConnect(int error_code)
{
	char buffer[100];	//buffer to hold string
	int result;
	
	//error checking
	if(error_code!=0)
	{
		p_dlg->StatusUpdate("Error in connecting");
		return;	//error so return
	}
	string request_hublist;
	request_hublist+="GET /PublicHubList.config HTTP/1.1\r\n";
	request_hublist+="Connection: Keep-Alive\r\n";
	request_hublist+="Host: ";
	request_hublist+="www.neo-modus.com";
	request_hublist+="\r\n";
	request_hublist+="\r\n";
	result=SendSocketData((unsigned char *)request_hublist.c_str(),(unsigned int)request_hublist.size());
	sprintf(buffer,"Sent Data with error: %d Last Error: %d",result,GetLastError());
	p_dlg->StatusUpdate(buffer);
	p_dlg->StatusUpdate("Receiving Hub List...");
}

int hublistcollector::Connect(char* host, unsigned short port)
{
	return TAsyncSocket::Connect(host, port);
}
void hublistcollector::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	CString temp=data;
	m_data=m_data+data;
}

void hublistcollector::OnClose(int error_code)
{
	m_data.Delete(0,m_data.Find("octet-stream",0)+14);
	Hub current_hub;
	vector<char*> hub_string;
	//get the line with the hub info
	char * tok=strtok(m_data.GetBuffer(m_data.GetLength()),"\r\n");
	while(tok!=NULL)
	{
		hub_string.push_back(tok);
		tok=strtok(NULL,"\r\n");
	}

	//parse all the hub info and get the address,descrip,...
	//std::list<char*>::iterator j;
	for(int j=0;j<(int)hub_string.size();j++)
	{
		char * tok2=strtok(hub_string[j],"|");
		while(tok2!=NULL)
		{
			current_hub.m_name=tok2;
			//p_dlg->StatusUpdate(current_hub.m_name);
			tok2=strtok(NULL,"|");
			
			/*string temp;
			temp=tok2;
			
			size_t found_colon=temp.find(":");
			if(found_colon>0)
			{
				temp.erase(found_colon);
				current_hub.m_address=(char *)temp.c_str();
			}*/

			//CString temp;
			char *temp;
			temp=tok2;
			int count=0;
			char *result=strstr(temp,":");
			if(result!=NULL)
			{
				while(temp!=result)
				{
					temp++;
					count++;
				}
				tok2[count]='\0';
				current_hub.m_address=tok2;//"TEST";//temp.GetBuffer(temp.GetLength());

			}
			else
			{
				current_hub.m_address=tok2;
			}
			tok2=strtok(NULL,"|");

			current_hub.m_description=tok2;
			tok2=strtok(NULL,"|");
			if(tok2!=NULL)
			{	current_hub.m_port=atoi(tok2);
			}
			tok2=strtok(NULL,"|");
		}

		hubs.push_back(current_hub);
	}
	p_dlg->ReceivedHubList(hubs);	//send the hub list to the dialog box
	//char buffer[100];
	//p_dlg->StatusUpdate(itoa(hub_string.size(),buffer,10)); //number of hubs
}

void hublistcollector::DownloadHubList()
{
	//start up the socket
	WSocket::Startup();

	int result;	//result of socket operation
	char buffer[50];

	result=this->Create();		//create socket
	sprintf(buffer,"Created Socket with error: %d",result);
	p_dlg->StatusUpdate(buffer);	//display in listbox

	//connect to computer that contains hublist
	result=this->Connect("64.71.145.202",80);
	//result=this->Connect("195.113.124.99",80);
	sprintf(buffer,"Connected with error: %d",result);
	p_dlg->StatusUpdate(buffer);
}

vector<Hub> hublistcollector::GetHubList()
{
	return hubs;
}