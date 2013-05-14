#include "StdAfx.h"

#include "dcclienthubsocket.h"
#include "DCScannerDlg.h"
#include "UDPSearchResult.h"

DCClientHubSocket::DCClientHubSocket(void)
{
	b_search=false;
	b_connected=false;
}

DCClientHubSocket::~DCClientHubSocket(void)
{
	//WSocket::Cleanup();
	//this->Close();
}
void DCClientHubSocket::InitDlg(DCScannerDlg* dlg)
{
	p_dlg=dlg;
}

void DCClientHubSocket::GetDatabaseObject(DB &conn)
{
	p_db=&conn;
}
int DCClientHubSocket::HubConnect(char *host,unsigned short port)
{
	b_connected=true;
	//string connection_status;
	char buffer[100];
	//WSocket::Startup();
	this->Close();	//added 2004-2-27
	int connect_result=this->Create();
	//memset(connection_status,0,sizeof(connection_status));
	char connection_status[100];
	if(connect_result==0)
	{
		strcpy(connection_status,"Connecting to ");
		strcat(connection_status,host);
		p_dlg->StatusUpdate(connection_status);
		//connecting
		TRACE("Host: %s Port: %d\n",host,port);
		return TAsyncSocket::Connect(host, port);
	}
	else
	{
		strcat(connection_status,"Socket Error Connecting ");
		strcat(connection_status,itoa(connect_result,buffer,10));
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
	
	if(error_code!=0)
	{
		b_connected=false;
		this->Close();
		m_connection_status="Error connecting ";
		if(error_code==10061)
			m_connection_status+=": Connection Refused";
		else
		{
			char buffer[100];
			m_connection_status+=itoa(error_code,buffer,10);
		}
		p_dlg->StatusUpdate((char*)m_connection_status.c_str());
	}
	else
	{
		p_dlg->StatusUpdate("Successfully Connected");
	}
	//memset(m_hubname,0,sizeof(m_hubname));		//reset hubname
	strcpy(m_hubname,"");
	m_nick = this->GetRandomNick();
	commands.SetHubName("");
	commands.SetNick((char*)m_nick.c_str());	//set random nick
}
void DCClientHubSocket::OnClose(int error_code)
{
	//WSocket::Cleanup();
	p_dlg->StatusUpdate("Closing Socket");
	b_connected=false;
	b_search=false;
	TAsyncSocket::OnClose(error_code);
}

void DCClientHubSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	/*char* banned;
	char* kicked;
	banned =strstr(data,"banned");
	kicked = strstr(data,"$kick");
	//create a log file of why banned
	if(banned != NULL || kicked != NULL)
	{
		CFile banned("c:\\DCMaster Info\\DCScanner\\banned.txt",CFile::modeWrite|CFile::typeBinary);
		banned.SeekToEnd();
		banned.Write(data,(UINT)strlen(data));	//write banned to a log file
		banned.Close();

	}*/
			// Declare the variables needed
	/*char *temp = new char[strlen(data)+1];
	strcpy(temp,data);
	temp[strlen(data)] = '\0';
	char* commands = strtok(temp,"|");
	while(commands != NULL)
	{
		string temp2 = commands;
		temp2 += "|";*/
		this->ParseClientDataReceived(data,data_len);
		/*commands = strtok(NULL,"|");
	}
	delete [] temp;
	*/
}

void DCClientHubSocket::ParseClientDataReceived(char * data, int len)
{
	char * ptr;
	srand(timeGetTime());	//random seed
	//char *pk;
	//CONDITION THAT LOCK IS RECEIVED
	ptr = strstr(data,"$Lock");
	if (ptr != NULL)
	{
		m_key=commands.SendKey(data);
		this->SendSocketData((unsigned char*)m_key.GetBuffer(m_key.GetLength()),m_key.GetLength());
		//return;	removing because it wasn't getting the hub name!
	}
	//CONDITION THAT HELLO COMMAND IS RECEIVED
	ptr = strstr(data,"$Hello ");
	if(ptr != NULL)
	{
		char *MyInfo = new char[300];
		commands.SenyMyInfo(data,MyInfo);
		this->SendSocketData((unsigned char*)MyInfo,(unsigned int)strlen(MyInfo));
		b_search=true;
		delete [] MyInfo;
	}
	ptr = strstr(data,"$HubName ");
	if (ptr != NULL)
	{
		int hub_pos = 0;
		ptr += 9;
		char * found_end = strstr(ptr,"|");
		if(found_end!=NULL)
		{
			//char hubname[100];// = new char[100];
			while(*ptr !='|' && ptr!=NULL && hub_pos<100)
			{
				m_hubname[hub_pos] = *ptr;
				ptr++;
				hub_pos++;
			}
			m_hubname[hub_pos] = '\0';	//null
			commands.SetHubName(m_hubname);	//set hub name
		}
		//TRACE("Setting hubname to: %s\n",m_hubname);
		//delete [] hubname;
		return;
	}
	ptr =strstr(data,"$ConnectToMe");
	if(ptr != NULL)
	{	
		//char MyNick[200]="$MyNick";
		//memset(MyNick,0,sizeof(MyNick));
		strcpy(MyNick,"$MyNick");
	//raphael	strcat(MyNick," big_swapperr|$Lock ");
		strcat(MyNick," ");
		strcat(MyNick,m_nick.c_str());
		strcat(MyNick,"|$Lock ");
		//////////////////////////
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
		char* ConnectToMe = new char[300];
		if(commands.SendConnectToMe(ptr,ConnectToMe))
		{
				m_dm->GetDBConnection(p_db);
				m_dm->AddDownload(address.GetBuffer(address.GetLength()),atoi(port.GetBuffer(port.GetLength())),MyNick);	//change adddownload to have parameters address and port
		}

		delete[] ConnectToMe;
		return;
	}
	//SEARCH CONDITION FOR PASSIVE USERS
	ptr = strstr(data,"$Search Hub:");
	if(ptr != NULL)
	{
		vector<string> SearchResults;
		m_SRPasive = ""; //reset
		sockaddr_in addr_temp;
		memset(&addr_temp,0,sizeof(addr_temp));
		commands.GetDBConnection(p_db);
		int len=sizeof(addr_temp);
		if(getpeername(this->m_hSocket,(sockaddr*)&addr_temp,&len)==-1)
		{
			TRACE("ERROR GETTING IP ADDRESS!!");
		}
		else
		{
			commands.SetHubIpAddress(addr_temp.sin_addr.S_un.S_addr);
		}
		commands.SendSRPasive(data,SearchResults);
		//char SR[200];
		//strcpy(SR,SRPasive);
		//if(strcmp(SRPasive,"ERROR")!=0)
		//{
		UINT searchresults_size = SearchResults.size();
		for(UINT i=0;i<searchresults_size;i++)
		{
			m_SRPasive=SearchResults[i];
			//TRACE("%s\n",SearchResults[i]);
			this->SendSocketData((unsigned char*)m_SRPasive.c_str(),(unsigned int)strlen(m_SRPasive.c_str()));
		}
		p_dlg->IncrementSpoofs((int)SearchResults.size());
			//}
		//delete [] SRPasive;
		return;
	}
	//CONDITION THAT SEARCH COMMAND IS RECEIVED
	ptr = strstr(data,"$Search");
	if(ptr != NULL)
	{
			char *extract_ip=new char[200];
			char *extract_port=new char[200];
			//char *SR;
			//char SRActive[200];
			vector<string> active_class;
			commands.GetDBConnection(p_db);
			sockaddr_in addr_temp;
			memset(&addr_temp,0,sizeof(addr_temp));
			int len=sizeof(addr_temp);
			if(getpeername(this->m_hSocket,(sockaddr*)&addr_temp,&len)==-1)
			{
				TRACE("ERROR GETTING IP ADDRESS!!");
			}
			else
			{
				commands.SetHubIpAddress(addr_temp.sin_addr.S_un.S_addr);
			}
			commands.SendSRActive(data,active_class,extract_port,extract_ip);
			string temp_sra;
			UINT active_class_size = active_class.size();
			for(UINT i=0;i<active_class_size;i++)
			{
				temp_sra=active_class[i];
				udp_sr.Connect(extract_ip,(unsigned short)atoi(extract_port));
				udp_sr.SendTo((char*)temp_sra.c_str(),(UINT)strlen(temp_sra.c_str()),this->GetIntIPFromStringIP(extract_ip),(unsigned short)atoi(extract_port));
			}
			p_dlg->IncrementSpoofs((int)active_class.size());
			delete [] extract_ip;
			delete [] extract_port;
			return;
	}
	//CONDITION THAT $RevConnectToMe COMMAND IS RECEIVED
	ptr = strstr(data,"$RevConnectToMe");
	if(ptr != NULL)
	{
		//adds a passive download to the download manager
		char buff[100];
		m_dm->GetDBConnection(p_db);	//pass database connection
		int socket = m_dm->AddPassiveDownload();
		m_dm->IncrementDownloadPosition();
		CString sender;
		CString revconnect = data;
		string passive_connect;
		revconnect.Replace("$RevConnectToMe ","");
		char *rev_tok = strtok(revconnect.GetBuffer(0)," ");
		sender = rev_tok;
		passive_connect = "$ConnectToMe ";
		passive_connect += sender;
		passive_connect += " 38.119.66.29:";
		passive_connect += itoa(socket,buff,10);
		passive_connect += "|";
		this->SendSocketData((unsigned char*)passive_connect.c_str(),(unsigned int)strlen(passive_connect.c_str()));
		return;
	}

	ptr = strstr(data,"$MultiSearch ");
	if(ptr != NULL)
	{
		/*TRACE("$MultiSearch received!\n %s\n", data);
		char *extract_ip=new char[200];
		char *extract_port=new char[200];
		vector<string> active_class;
		commands.GetDBConnection(p_db);
		sockaddr_in addr_temp;
		memset(&addr_temp,0,sizeof(addr_temp));
		int len=sizeof(addr_temp);
		if(getpeername(this->m_hSocket,(sockaddr*)&addr_temp,&len)==-1)
		{
			TRACE("ERROR GETTING IP ADDRESS!!");
		}
		else
		{
			commands.SetHubIpAddress(addr_temp.sin_addr.S_un.S_addr);
		}
		commands.SendSRActive(data,active_class,extract_port,extract_ip);
		string temp_sra;
		for(UINT i=0;i<active_class.size();i++)
		{
			temp_sra=active_class[i];
			udp_sr.Connect(extract_ip,(unsigned short)atoi(extract_port));
			udp_sr.SendTo((char*)temp_sra.c_str(),(UINT)strlen(temp_sra.c_str()),this->GetIntIPFromStringIP(extract_ip),(unsigned short)atoi(extract_port));
		}
		p_dlg->IncrementSpoofs((int)active_class.size());
		delete [] extract_ip;
		delete [] extract_port;
		return;*/
	}
	ptr = strstr(data,"$MultiConnectToMe ");
	if(ptr != NULL)
	{
		TRACE("Received MultiConnectToMe!\n");
		return;
	}

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

//Gets the Project Data from the Project Manager Object
//
void DCClientHubSocket::GetProjectData(ProjectKeywordsVector *pk)
{
	p_projkeywords = pk;
	commands.GetProjectKeywordsVector(p_projkeywords);
}

void DCClientHubSocket::GetDownloadManager(DownloadManager &dm)
{
	m_dm = &dm;
}

//returns a random nick so each client can have different nicks
string DCClientHubSocket::GetRandomNick()
{

	m_rand_number = rand() % 8;
	switch(m_rand_number)
	{
		case 0:
			return "xsharerl";
		case 1:
			return "nubeezz";
		case 2:
			return "traderus";
		case 3:
			return "big_swapperr";
		case 4:
			return "lansinga";
		case 5:
			return "peimdu";
		case 6:
			return "johnnyduz";
		case 7:
			return "yrulooking";
		case 8:
			return "user911";
		default:
			return "udontcare";
	}
}
