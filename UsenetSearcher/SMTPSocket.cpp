// SMTPSocket.cpp: implementation of the SMTPSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMTPSocket.h"
#include "UsenetSearcherDll.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SMTPSocket::SMTPSocket()
{
}

//
//
//
SMTPSocket::~SMTPSocket()
{
}

//
//
//
void SMTPSocket::OnConnect(int error_code)
{
	CString strEmail = "searcher_email.msc";
	m_file.Open(strEmail, CFile::modeRead|CFile::modeNoTruncate|CFile::typeText|CFile::shareDenyNone);
	m_file.SeekToBegin();
}

//
//
//
void SMTPSocket::SendMail(char *msg)
{
	Close();

	m_email_message = "\n\n";
	m_email_message += msg;

	CStdioFile file;

	CString strEmail= "searcher_email.msc";
	file.Open(strEmail, CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::typeText|CFile::shareDenyNone);

	file.SeekToEnd();
//	file.WriteString(msg);
//	file.WriteString("\r\n.\n");
//	file.WriteString("quit\n");
	file.Close();

	CString strCmd;
	strCmd = "tellsmtp ";
	strCmd += "\"";
	strCmd += strEmail;
	strCmd += "\"";

	system(strCmd);

	Create();
	Connect("mail.onsystems.com", 25);
}


//
//
//
void SMTPSocket::OnReceive(int error_code)
{
	//error checking
	if(error_code!=0)
	{
		TRACE("Error Receiving Data\n");
		return;	//return due to error
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	ReceiveSomeSocketData(4096);

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

//
//
//
void SMTPSocket::OnAccept(int error_code)
{

}

//
//
//
void SMTPSocket::OnClose(int error_code)
{
//	p_dll->Log("SMTP Socket", "***Disconnected from mail server***");
	Close();
	m_file.Close();
}

void SMTPSocket::SomeSocketDataReceived(char *data, unsigned int data_len, unsigned int new_len, unsigned int max_len)
{
	char buf[8192];
	int buf_pos = 0;
	ZeroMemory(&buf, 8192);

	if(strlen(m_previous_buffer) != 0)
	{
		strcpy(buf, m_previous_buffer);
		buf_pos = (int)strlen(m_previous_buffer);
	}

	for (UINT x = 0; x < data_len; x++)
    {
	    buf[buf_pos] = data[x];
		if(x == data_len-1 && data[x] != '\n' && data[x-1] != '\r')
		{
			strcpy(m_previous_buffer, buf);
		}
		else
		{
			::ZeroMemory(&m_previous_buffer, 8192);
		}
  
		buf_pos++;
        if (data[x] == '\r')
           buf[buf_pos-1] = 0;
        if (data[x] == '\n')
        {
           buf[buf_pos-1] = 0;
           Parse(buf);
           ZeroMemory(&buf, 8192);
           buf_pos = 0;
        }
     }
	m_previous_buffer_len = data_len;
}

/*
//
//
//
void SMTPSocket::DataReceived(unsigned int len)
{
	p_dll->Log((char*)p_read_data_buf);
	Parse((char*)p_read_data_buf);
	WSocket::DataReceived(len);
}
*/

//
//
//
void SMTPSocket::InitParent(UsenetSearcherDll *parent)
{
	p_dll = parent;
}

//
//
//
void SMTPSocket::Parse(char* buffer)
{
	int op_code = 0;
	sscanf(buffer, "%d", &op_code);
	switch(op_code)
	{
	case 220:
	case 250:
		{
			CString temp;
			unsigned char *buf = new unsigned char[2048];
			memset(buf, 0, 2048);

			if(m_file.ReadString(temp) != FALSE)
			{
				if(temp.GetLength() < 2050)
				{
					_tcscpy((char *)buf, temp);
//					p_dll->Log("SMTP Socket", (char *)buf);
					strcat((char *)buf, "\r\n");
					SendSocketData(buf, (UINT)strlen((const char *)buf));
				}
			}
			delete [] buf;
			break;
		}
	case 354:
		{
			CString temp;
			unsigned char *buf = new unsigned char[2048];
			memset(buf, 0, 2048);

			while(m_file.ReadString(temp))
			{
				if(temp.CompareNoCase(".")==0)
				{
					_tcscpy((char *)buf, m_email_message);
//					p_dll->Log("SMTP Socket", (char *)buf);
					strcat((char *)buf, "\r\n");
					SendSocketData(buf, (UINT)strlen((const char *)buf));
				}

				if(temp.GetLength() < 2050)
				{
					_tcscpy((char *)buf, temp);
//					p_dll->Log("SMTP Socket", (char *)buf);
					strcat((char *)buf, "\r\n");
					SendSocketData(buf, (UINT)strlen((const char *)buf));
				}
				if(temp.CompareNoCase(".")==0)
				{
					break;
				}
			}
			delete [] buf;
			break;
		}
	case 221:
		{	
			// Goodbye
			Close();
			bool goodbye = true;
			break;
		}
	}
}
