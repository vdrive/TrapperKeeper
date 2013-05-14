#include "StdAfx.h"
#include "hublistcollector.h"
#include "dcscannerdlg.h"
#include <list>

hublistcollector::hublistcollector(void)
{
}

hublistcollector::~hublistcollector(void)
{
}

void hublistcollector::InitDlg(DCScannerDlg* dlg)
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
	char *tok;
      char * tok2;
      char *temp;
      char *result;
      char *old_result;

      m_data.Delete(0,m_data.Find("octet-stream",0)+14);
      Hub current_hub;
      vector<char*> hub_string;

      //get the line with the hub info

      tok=strtok(m_data.GetBuffer(m_data.GetLength()),"\r\n");
      while(tok!=NULL)
      {
            hub_string.push_back(tok);
            tok=strtok(NULL,"\r\n");
      }
      //parse all the hub info and get the address,descrip,...

      //std::list<char*>::iterator j;

      for(int j=0;j<(int)hub_string.size();j++)
      {
            tok2=strtok(hub_string[j],"|");
            while(tok2!=NULL)
            {
                  current_hub.m_name=tok2;
                  //p_dlg->StatusUpdate(current_hub.m_name);
                  tok2=strtok(NULL,"|");
                  //CString temp;

                  temp=tok2;
                  int count=0;
                  result=strstr(temp,":");
                  if(result!=NULL)
                  {
					  old_result = strstr(temp,":");
                      old_result++;

                        current_hub.m_port=atoi(old_result);
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
						current_hub.m_port=411;	//default
                  }

                  tok2=strtok(NULL,"|");
                  current_hub.m_description=tok2;
                  tok2=strtok(NULL,"|");
                  if(tok2!=NULL)
                  {     current_hub.m_users=atoi(tok2);
                  }

                  tok2=strtok(NULL,"|");
            }
            hubs.push_back(current_hub);
      }

      //p_dlg->ReceivedHubList(hubs);     //send the hub list to the dialog box

      TRACE("SIZE: %d",hubs.size());

      //char buffer[100];

      //p_dlg->StatusUpdate(itoa(hub_string.size(),buffer,10)); //number of hubs

//adding    

      hub_string.clear();
      bool file_exists = FileCheck("c:\\dc\\hublist.txt");

      if(file_exists)
      {
            CFile dchublist("c:\\dc\\hublist.txt",CFile::modeRead);
            UINT len=(UINT)dchublist.GetLength();

            buf=new char[len];
            dchublist.Read(buf,len);
            m_hubdata = buf;
            //check if there is a file with hublist
            //get the line with the hub info
            tok=strtok(m_hubdata.GetBuffer(m_hubdata.GetLength()),"\r\n");

            while(tok!=NULL)
            {
                  hub_string.push_back(tok);
                  tok=strtok(NULL,"\r\n");
            }
            //parse all the hub info and get the address,descrip,...

            //std::list<char*>::iterator j;

            TRACE("SIZE: %d\n",hub_string.size());
            for(int j=0;j<(int)hub_string.size()-5;j++)
            {
                  tok2=strtok(hub_string[j],"|");
                  while(tok2!=NULL)
                  {
                        current_hub.m_name=tok2;
                        //p_dlg->StatusUpdate(current_hub.m_name);
                        tok2=strtok(NULL,"|");
                       

                        //CString temp;
                        temp=tok2;

                        if(temp==NULL)
                        {
                              hub_string.clear();     //clear the hubstring
                              m_data = "";
                              p_dlg->ReceivedHubList(hubs);
                              return;
                        }
                        int count=0;
                        result=strstr(temp,":");
                        if(result!=NULL)
                        {
                              //get port number
                              old_result = strstr(temp,":");
                              old_result++;

                              current_hub.m_port=atoi(old_result);
                              while(temp!=result)
                              {
                                    temp++;
                                    count++;
                              }
                              tok2[count]='\0';
                              current_hub.m_address=tok2;//"TEST";//temp.GetBuffer(temp.GetLength());
                        }
                        //no port address use default
                        else
                        {
                              current_hub.m_address=tok2;
                              current_hub.m_port=411;
                        }
                        tok2=strtok(NULL,"|");
                        current_hub.m_description=tok2;
                        tok2=strtok(NULL,"|");

                        if(tok2!=NULL)
                        {     current_hub.m_users=atoi(tok2);
                        }
                        tok2=strtok(NULL,"|");
                  }
                  hubs.push_back(current_hub);
            }
      }
      TRACE("SIZE: %d",hubs.size());
      p_dlg->ReceivedHubList(hubs);
}

void hublistcollector::DownloadHubList()
{
	WSocket::Startup();

	int result; //result of socket operation
	char buffer[50];

	result=this->Create();		//create socket
	sprintf(buffer,"Created Socket with error: %d",result);
	p_dlg->StatusUpdate(buffer);	//display listbox

	//connect to computer that contains hublist
	result=this->Connect("64.71.145.202",80);
	sprintf(buffer,"Connected with error: %d",result);
	p_dlg->StatusUpdate(buffer);
}
vector<Hub> hublistcollector::GetHubList()
{
	return hubs;
}

bool hublistcollector::FileCheck(LPSTR lpszFilename)
{
      DWORD dwAttr = GetFileAttributes(lpszFilename);
      if (dwAttr == 0xffffffff)
            return FALSE;
      else 
            return TRUE;
}
