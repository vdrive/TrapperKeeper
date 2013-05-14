#include "StdAfx.h"

#include "dcclienthubsocket.h"
#include "DCSupplyDlg.h"
#include "UDPSearchResult.h"

DCClientHubSocket::DCClientHubSocket(void)
{
	b_search=false;
	b_connected=false;
	test_tcp = true;
}

DCClientHubSocket::~DCClientHubSocket(void)
{
	delete downloads;
}
void DCClientHubSocket::InitDlg(DCSupplyDlg* dlg)
{
	p_dlg=dlg;
}

int DCClientHubSocket::HubConnect(char *host,unsigned short port)
{
	b_connected=true;
	//string connection_status;
	char buffer[10];
	WSocket::Startup();
	int connect_result=this->Create();
	if(connect_result==0)
	{
		char connection_status[100]="Connecting to ";
		strcat(connection_status,host);
		p_dlg->StatusUpdate(connection_status);
		m_host=host;	//store hostname
		//connecting
		return TAsyncSocket::Connect(host, port);
	}
	else
	{
		char connection_status[100]="Error Connecting ";
		strcat(connection_status,itoa(connect_result,buffer,100));
		p_dlg->StatusUpdate(connection_status);
		b_connected=false;
		return -1;
	}
}

void DCClientHubSocket::OnReceive(int error_code)
{
	//error checking
	if(error_code!=0)
	{
		p_dlg->StatusUpdate("Error Receiving Data");
		return;	//return due to error
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

void DCClientHubSocket::OnConnect(int error_code)
{
	string connection_status;
	if(error_code!=0)
	{
		b_connected=false;
		this->Close();
		connection_status="Error connecting ";
		if(error_code==10061)
			connection_status+=": Connection Refused";
		else
		{
			char buffer[10];
			connection_status+=itoa(error_code,buffer,10);
		}
		p_dlg->StatusUpdate((char*)connection_status.c_str());
	}
	else
	{
		p_dlg->StatusUpdate("Successfully Connected");
	}
}
void DCClientHubSocket::OnClose(int error_code)
{
	p_dlg->StatusUpdate("Closing Socket");
	Hub cur_hub;
	cur_hub.m_address = m_host;
	cur_hub.m_connected = false;
	m_hub->push_back(cur_hub);
	b_connected=false;
	b_search=false;
}
void DCClientHubSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	this->ParseClientDataReceived(data,data_len);
}

void DCClientHubSocket::ParseClientDataReceived(char * data, int len)
{
	char * ptr;
	//char *pk;
	//CONDITION THAT LOCK IS RECEIVED
	ptr = strstr(data,"$Lock");
	if (ptr != NULL)
	{
		m_key=commands.SendKey(data);
		this->SendSocketData((unsigned char*)m_key.GetBuffer(m_key.GetLength()),m_key.GetLength());
		return;
	}
	//CONDITION THAT HELLO COMMAND IS RECEIVED
	ptr = strstr(data,"$Hello");
	if(ptr != NULL)
	{
		char *MyInfo = new char[200];
		commands.SenyMyInfo(data,MyInfo);
		this->SendSocketData((unsigned char*)MyInfo,(unsigned int)strlen(MyInfo));
		b_search=true;
		delete[] MyInfo;
		return;
	}
	ptr =strstr(data,"$ConnectToMe");
	if(ptr != NULL)
	{
		char MyNick[200]="$MyNick";
		strcat(MyNick," new_big_pirates|$Lock ");
		strcat(MyNick,m_keymanager.CreateLock());
		strcat(MyNick," Pk=");
		strcat(MyNick,m_keymanager.CreatePK());
		strcat(MyNick,"|");

		CString temp=data;
		CString ip_address_temp;
		CString ip_address;
		int position = temp.Find("|",0);
		if(position<0)
			return;
		temp=temp.Left(position);
		temp.Replace("|","");
		//memset(ip_address,0,sizeof(ip_address));	//clear contents of ip_address
		ip_address=temp.GetBuffer(temp.GetLength());

		int cur_pos=0;
		ip_address_temp=temp.Tokenize("' '",cur_pos);
		while(ip_address_temp!="")
		{
			ip_address=ip_address_temp;
			ip_address_temp=temp.Tokenize("' '",cur_pos);
		}
		position = ip_address.Find(":",0);
		if(position<0)
			return;
		CString address = ip_address.Left(ip_address.Find(":",0));
		CString port = ip_address.Right(ip_address.GetLength()-ip_address.Find(":",0));
		port.Replace(":","");
		char* send_MyNick=&MyNick[0];
		char* ConnectToMe = new char[200];
		if(commands.SendConnectToMe(ptr,ConnectToMe))
		{
			//connect to host requesting data!
			//if(test_tcp)
			//{
				//TRACE("ip: %s port: %d\n",address,atoi(port.GetBuffer(port.GetLength())));
				//DCDownload downloads1;
				//DCDownload *downloads1 = new DCDownload;
				downloads = new DCDownload;
				downloads->DownloadConnect(address.GetBuffer(address.GetLength()),atoi(port.GetBuffer(port.GetLength())));
				//send data handshake data to host
				//downloads.DownloadConnect("213.89.98.47",411);
				downloads->GetLoginData(MyNick);
				m_dm->AddDownload(downloads);

				//downloads.SendSocketData((unsigned char*)MyNick,strlen(MyNick));
				test_tcp = false;
			//}
		}

		/*char* send_MyNick=&MyNick[0];
		char* ConnectToMe = new char[200];
		if(commands.SendConnectToMe(ptr,ConnectToMe))
			this->SendSocketData((unsigned char*)ConnectToMe,(unsigned int)strlen(ConnectToMe));
			*/
		delete[] ConnectToMe;
		return;

		
	}
	//return reply by downloader to start downloading file
	ptr = strstr(data,"$MyNick");
	if(ptr != NULL)
	{
		TRACE("Received $MyNick command");
		//char* MyNick= new char[200];
		//if(commands.SendMyNick(ptr,MyNick))
		//	this->SendSocketData((unsigned char*)MyNick,(unsigned int)strlen(MyNick));
		//delete[] MyNick;
		return;
	}
	//SEARCH CONDITION FOR PASSIVE USERS
	ptr = strstr(data,"$Search Hub:");
	if(ptr != NULL)
	{
		/*char *tok=strtok(data,"|");
		char *search_for;
		bool found = false;
		while(tok!=NULL && found==false)
		{
			search_for=strstr(tok,"logilogo");	//create a local function that checks all the weights from the dc master
			if(search_for!=NULL)
			{
				data=tok;	//get the tok which has the search criteria
				found=true;
			}
			tok=strtok(NULL,"|");
		}

		if(search_for !=NULL)
		{
			CString temp=data;
			char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
			char *extract_hub="";
			char *extract_nick="";
			int tok_num=0;
			while(tok!=NULL && tok_num<1)
			{
				tok=strtok(NULL," ");
				extract_hub=tok;
				tok_num++;
			}

			tok=strtok(extract_hub,":");
			tok_num=0;
			while(tok!=NULL)
			{
				if(tok_num!=0)
					extract_nick=tok;	//hub name
				tok=strtok(NULL,":");
				tok_num++;
			}

			char SR[200];
			int* ptr= NULL;//(int*)&SR[0];
			char* ptr2=&SR[0];
			strcpy(SR,"$SR new_big_pirates dc\\shared files\\logilogo.mp3");	//file path
			ptr2 += strlen(SR);
			ptr=(int*)ptr2;	
			*ptr = 0x5;	//include ascii char 5
			ptr2+=1;
			char file_size[100]="12345688 3/3";
			strcat(SR,"12345688 3/3");	//filesize
			ptr2 += strlen(file_size);
			ptr=(int*)ptr2;
			*ptr =0x05;
			ptr2+=1;
			strcat(SR,"triviahub.servemp3.com (213.112.70.30:411)");
			ptr2 +=strlen("triviahub.servemp3.com (213.112.70.30:411)");
			ptr=(int*)ptr2;
			*ptr = 0x05;
			strcat(SR,extract_nick);
			strcat(SR,"|");*/
		string SRPasive;
		vector<string> SearchResults;
		commands.SendSRPasive(data,SearchResults);
		//char SR[200];
		//strcpy(SR,SRPasive);
		//if(strcmp(SRPasive,"ERROR")!=0)
		//{
		for(UINT i=0;i<SearchResults.size();i++)
		{
			SRPasive=SearchResults[i];
			//TRACE("%s\n",SearchResults[i]);
			this->SendSocketData((unsigned char*)SRPasive.c_str(),(unsigned int)strlen(SRPasive.c_str()));
		}
			//}
		//delete [] SRPasive;
		return;
	}
	//CONDITION THAT SEARCH COMMAND IS RECEIVED
	ptr = strstr(data,"$Search");
	if(ptr != NULL)
	{
			char *extract_ip=new char[100];
			char *extract_port=new char[100];
			char *temp_sr=new char[200];
			//char *SR;
			//char SRActive[200];
			vector<string> active_class;
			commands.SendSRActive(data,active_class,extract_port,extract_ip);
			//strcpy(SRActive,SR);
			//udp_sr.Connect(extract_ip,atoi(extract_port));
			//if(strcmp(temp_sr,"ERROR")!=0)
			//{
			string temp_sra;
			for(UINT i=0;i<active_class.size();i++)
			{
				temp_sra=active_class[i];
				
				//extract_ip=(char*)temp_sra.ip_address.c_str();
				//extract_port=(char*)temp_sra.port.c_str();
				//temp_sr=(char*)temp_sra.str_sr.c_str();

				//udp_sr.Connect(extract_ip,(unsigned short)atoi(extract_port));
				//udp_sr.SendTo((char*)temp_sra.c_str(),strlen(temp_sra.c_str()),this->GetIntIPFromStringIP(extract_ip),(unsigned short)atoi(extract_port));
			}
			//this->SendSocketData((unsigned char*)SR,(unsigned int)strlen(SR));
				//strcat(SR,"Weird NAME")
				//Search="
			delete [] temp_sr;
			delete [] extract_ip;
			delete [] extract_port;
			}
		/*char *Search;
		Search="$Search 64.60.52.146:8513 F?T?0?1?raphael|";
		if(sent_search==false)
		{
			this->SendSocketData((unsigned char*)Search,(unsigned int)strlen(Search));
			sent_search=true;
		}*/
		return;
	}

//replaces all spaces with '$' to make a valid search string
void DCClientHubSocket::Search(string search_for)
{
	char buffer[20];	//buffer for itoa operation
	string search;
	size_t x=search_for.find(" ");
	while(x<string::npos)
	{
		search_for.replace(x,1,"$");
		x=search_for.find(" ",x+1);
	}
	search="$Search 38.119.64.77:";
	//search="$Search 38.119.66.29:";
	search+=itoa(m_socket,buffer,10);
	search+=" F?T?0?1?";
	search+=search_for;
	search+="|";
	this->SendSocketData((unsigned char*)search.c_str(),(unsigned int)search.length());
}

bool DCClientHubSocket::GetConnected()
{
	return b_connected;
}

bool DCClientHubSocket::GetSearch()
{
	return b_search;
}

unsigned int DCClientHubSocket::GetIntIPFromStringIP(char* ip)
{
	unsigned int ip_int = 0;
	if(strlen(ip)==0)
		return ip_int;
	
	int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	return ip_int;
}

//returns true if all the keywords were found, else false
bool DCClientHubSocket::CheckKeywords(char* search_string)
{
	char* search_for;
	int keyword_counter;
	keyword_counter=0;
	//for loop with all the keywords
	search_for=strstr(search_string,"logilogo");
	if(search_for!=NULL)
	{
		keyword_counter++;	//increment the number of keywords found
	}
	//end for loop
	/*if(keyword_counter==size)
	{
		return true
	}*/
	else
	{
		return false;
	}
	return false;
}
//returns true if a kill word was found, else zero
bool DCClientHubSocket::CheckKillwords(char* search_string)
{
	char *search_for;
	//for loop for the kill words
	search_for=strstr(search_string,"logilogo");	//plug in search string
	if(search_for!=NULL)
	{
		return true;
		//break;
	}
	//
	return false;
}

//returns the name of the host currently connected
char* DCClientHubSocket::GetConnectedHost()
{
	if(m_host!=NULL)
	{
		memset(space,0,sizeof(space));
		strcpy(space," on ");
		strcat(space,m_host);
		return space;	//inserts a space in front of the host name
	}
	return "no host";
}

void DCClientHubSocket::SetSocket(int socket)
{
	m_socket=socket;
}

//get access to the retry hub vector 
void DCClientHubSocket::GetRetryHubObject(vector<Hub> &current_hub)
{
	m_hub = &current_hub;	
}
void DCClientHubSocket::Disconnect()
{
	/*char *quit = new char[100];
	strcat(quit,"$Quit new_big_pirates");
	this->SendSocketData((unsigned char*)quit,(UINT)strlen(quit));
	delete [] quit;		//clean up*/
	this->Close();
}	

void DCClientHubSocket::GetDownloadManager(DownloadManager &dm)
{
	m_dm = &dm;
}