#include "StdAfx.h"
#include "dcdownload.h"

DCDownload::DCDownload(void)
{
	//m_login = new char[256];
	b_connected = false;
}

DCDownload::~DCDownload(void)
{
		//delete [] m_login;
}

void DCDownload::OnClose(int error_code)
{
	//no longer connected
	b_connected = false;
}
int DCDownload::DownloadConnect(char* host,unsigned short port)
{
	WSocket::Startup();
	int connect_result=this->Create();
	// error connecting
	if(connect_result!=0)
	{
		b_connected = false;
	}
	//connection good
	else
	{
		b_connected = true;
	}
	return TAsyncSocket::Connect(host,port);
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
			CString Direction;
			CString temp;
			//char * temp_lock;
			//char* lock;
			ptr = strstr(data,"$Key");
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

			Direction = "$Direction Upload 7575";
			Direction += "|$Key ";
			Direction += m_keymanager.LocktoKey(temp);
			Direction += "|";
			//send direction download
			if(this->SendSocketData((unsigned char*)Direction.GetBuffer(Direction.GetLength()),(UINT)strlen(Direction.GetBuffer(Direction.GetLength())))==0)
			{
				this->SendSocketData((unsigned char*)Direction.GetBuffer(Direction.GetLength()),(UINT)strlen(Direction.GetBuffer(Direction.GetLength())));
			}
	}
	
	ptr = strstr(data,"$Get");
	if(ptr != NULL)
	{
		CString get;
		get = "$FileLength 1542400";

		//send file length
		if(this->SendSocketData((unsigned char*)get.GetBuffer(get.GetLength()),(UINT)strlen(get.GetBuffer(get.GetLength())))==0)
		{
			this->SendSocketData((unsigned char*)get.GetBuffer(get.GetLength()),(UINT)strlen(get.GetBuffer(get.GetLength())));
		}
		//::AfxMessageBox("Received Get Command");
	}

	ptr = strstr(data,"$Send");
	if(ptr != NULL)
	{
		TRACE("SENDING DATA %s", this->m_login);
		//send either full or send chunks of file
		CString junk;
		junk = "111111111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		junk = junk+"1111111111111111111111111111111111111111111111111111111111111111111111";
		if(this->SendSocketData((unsigned char*)junk.GetBuffer(junk.GetLength()),(UINT)strlen(junk.GetBuffer(junk.GetLength())))==0)
		{
			this->SendSocketData((unsigned char*)junk.GetBuffer(junk.GetLength()),(UINT)strlen(junk.GetBuffer(junk.GetLength())));
		}
	}


}

//
// gets all the lock and pk info to connect to the user
void DCDownload::GetLoginData(char* data)
{
	strcpy(&m_login[0],data);
}

bool DCDownload::GetConnectionStatus()
{
	return b_connected;
}

//copy constructor
DCDownload::DCDownload(const DCDownload& dc)
{
	this->b_connected = dc.b_connected;
	strcpy(this->m_login, dc.m_login);
	this->m_keymanager = dc.m_keymanager;
	this->m_hSocket = dc.m_hSocket;
	this->m_port = dc.m_port;
	strcpy(this->m_hostent_buf,dc.m_hostent_buf);
	this->m_hwnd = dc.m_hwnd;
//	this->m_num_read = dc.m_num_read;
//	this->m_num_sent = dc.m_num_sent;
//	this->m_read_data_buf_len = dc.m_read_data_buf_len;
	this->m_receiving_socket_data = dc.m_receiving_socket_data;
	this->m_receiving_some_socket_data = dc.m_receiving_some_socket_data;
//	this->m_send_data_buf_len = dc.m_send_data_buf_len;
	this->m_sending_socket_data = dc.m_sending_socket_data;
	this->m_socket_type = this->m_socket_type;
//	this->m_wnd(m_wnd);
//	this->p_read_data_buf = dc.p_read_data_buf;
//	this->p_send_data_buf = dc.p_send_data_buf;
	this->v_send_data_buffers = dc.v_send_data_buffers;
	//this-> = dc;
}
DCDownload DCDownload::operator =( const DCDownload& dc)
{
	DCDownload dc_new(dc);
	return dc_new;
}