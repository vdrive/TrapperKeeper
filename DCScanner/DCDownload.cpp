#include "StdAfx.h"
#include "dcdownload.h"

DCDownload::DCDownload(void)
{
	//m_login = new char[256];
	m_sending = false;
	file_junk = new char[40906];
	memset(file_junk,0,sizeof(file_junk));
}

DCDownload::~DCDownload(void)
{
		//delete [] m_login;
		delete [] file_junk;
}

//close download socket
int DCDownload::Close()
{
	m_sending = false;
	return TAsyncSocket::Close();
}
void DCDownload::OnClose(int error_code)
{
	//no longer connected
	m_sending = false;
	//WSocket::Cleanup();
	TAsyncSocket::OnClose(error_code);
}
int DCDownload::DownloadConnect(char* host,unsigned short port, bool init)
{
	//WSocket::Startup();
	//if(!init)
	//{
	//this->Close();		//added 2004-2-27
	this->Create();
//	}
	
	return TAsyncSocket::Connect(host, port);
}
void DCDownload::OnConnect(int error_code)
{
	//when first connected successfully, send $mynick info
	this->SendSocketData((unsigned char*)m_login,(UINT)strlen(m_login));
}
void DCDownload::OnReceive(int error_code)
{
	if(error_code!=0)
	{
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
	//::AfxMessageBox("Received Message");
}

void DCDownload::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned max_len)
{
	this->ParseData(data,data_len);
}
void DCDownload::ParseData(char* data,int data_len)
{
	char * ptr;
	ptr = strstr(data,"$MyNick");
	if(ptr != NULL)
	{
			int position;
			//CString Direction;
			CString temp;
			//char * temp_lock;
			//char* lock;
			//ptr = strstr(data,"$Key");
			//added for dc plus plus lock key
			/*if(ptr != NULL)
			{
				temp = data;
				temp = temp.Right(temp.GetLength()-temp.Find("$Lock",0));
				temp = temp.Left(temp.Find(" Pk",0));
				temp.Replace("$Lock ","");
			}
			else
			{

				//add something for fake checker code
				temp_lock= strtok(data,"|");
				while(temp_lock != NULL)
				{
					lock = temp_lock;
					temp_lock = strtok(NULL,"|");
				}
				temp = lock;
				position = temp.Find(" Pk",0);
				if(position<0)
					return;
				temp = temp.Left(temp.Find(" Pk",0));
				temp.Replace("$Lock ","");
				temp.Replace("|","");
			}*/
			temp = data;
			position = temp.Find(" Pk",0);
			if(position<0)
				return;		//error in handshake
			temp = temp.Right(temp.GetLength()-temp.Find("$Lock",0));
			temp = temp.Left(temp.Find(" Pk",0));
			temp.Replace("$Lock ","");

			/*Key = "$Key ";
			Key += this->makeKey(temp.GetBuffer(0)).c_str(); //m_keymanager.LocktoKey(temp);
			Key +="|";*/
			char buff[10];
			srand(time(NULL));
			int upload = rand() % 8000;
			Key = "$Direction Upload ";
			Key += itoa(upload,buff,10);
			Key += "|$Key ";
			Key += m_keymanager.LocktoKey(temp);
			Key += "|";
			//send direction download
	}
	ptr = strstr(data,"$Direction");
	if(ptr != NULL)
	{
		this->SendSocketData((unsigned char*)Key.GetBuffer(0),(UINT)strlen(Key.GetBuffer(0)));
	}
	ptr = strstr(data,"$Get ");
	if(ptr != NULL)
	{
		CString get;
		ptr = strstr(data,"MyList.DcLst");
		if(ptr != NULL)		//send file length for dclst
		{
			get = "$MaxedOut|";
			//get = "$FileLength 150000|";
		}
		else	//send file length of file trying to download
		{
			//parse to get the filename requested
			//ptr = strstr(data,"/");
			char* tok = strtok(data,"\\");
			char* parser;
			while(tok!=NULL)
			{
				parser = tok;
				tok = strtok(NULL,"\\");
			}
			CString parser_temp = parser;
			int parser_position = parser_temp.Find("$");
			int found_backslash = parser_temp.Find("\\");		//check for fake get command
			if(parser_position>0 && found_backslash)
			{
				parser_temp = parser_temp.Left(parser_position);
			}
			else
			{
				get = "$Error File Doesn't Exist|";
				this->SendSocketData((unsigned char*)get.GetBuffer(0),(UINT)strlen(get.GetBuffer(0)));
				return;	//error in $Get
			}

			get = "$FileLength ";
			get += p_db->GetFilesize(parser_temp.GetBuffer(0)).c_str();
			get += "|";
		}
	//	strcat(get,"$FileLength ");
	//	strcat(get,"|");
		
		//get = "$MaxedOut";

		//send file length

		this->SendSocketData((unsigned char*)get.GetBuffer(0),(UINT)strlen(get.GetBuffer(0)));
		//::AfxMessageBox("Received Get Command");
	}
	
	ptr = strstr(data,"$GetListLen|");
	if(ptr != NULL)
	{
		CString get;
		get = "$ListLen 62000|";
		//get = "$MaxedOut";

		//send file length

			this->SendSocketData((unsigned char*)get.GetBuffer(get.GetLength()),(UINT)strlen(get.GetBuffer(get.GetLength())));

		//::AfxMessageBox("Received Get Command");
	}
	ptr = strstr(data,"$Send");
	if(ptr != NULL)
	{
		m_sending=true;
		TRACE("SENDING DATA %s", this->m_login);
		//send chunks of file
		//char * file_junk = new char[80000];
		//memset(file_junk,1,sizeof(file_junk));
		this->SendSocketData((unsigned char*)file_junk,(UINT)strlen(file_junk));
		//delete [] file_junk;
	}
}

void DCDownload::SendJunk()
{
	if(this->m_sending)
	{
		//memset(file_junk,1,sizeof(file_junk));
		this->SendSocketData((unsigned char*)file_junk,(UINT)strlen(file_junk));
		TRACE("SENDING JUNK-");
		//delete [] file_junk;
	}
}
//
// gets all the lock and pk info to connect to the user
void DCDownload::GetLoginData(char* data)
{
	strcpy(m_login,data);
}


void DCDownload::GetDBConnection(DB *db)
{
	p_db = db;
}

int DCDownload::Disconnect()
{
	m_sending = false;
	return TAsyncSocket::Close();
}

//creates a port to listen on
int DCDownload::CreatePort()
{
	int port = rand();
	m_listening_port = port;
	//this->Close();	//close socket if it was open
	int result = WSocket::Create(port);//this->Create(3698);
	int listening = listen(this->m_hSocket,1);		//enable socket to be listening
	return result;
}
//returns the port that is listening
int DCDownload::GetPort()
{
	return m_listening_port;
}