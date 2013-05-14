// SMTPSocket.cpp: implementation of the SMTPSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMTPSocket.h"
//#include "IRCDlg.h"
#include "UsenetPosterDlg.h"
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
	Cleanup();
}

//
//
//
void SMTPSocket::OnConnect(int error_code)
{
	m_file.Open("email.msc", CFile::modeRead|CFile::modeNoTruncate|CFile::typeText|CFile::shareDenyNone);
}

//
//
//
void SMTPSocket::OnReceive(int error_code)
{
	ReceiveData(8192);
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
	p_dlg->PutMsg("***Disconnected from mail server***");
	Close();
	m_file.Close();
}

//
//
//
void SMTPSocket::DataReceived(unsigned int len)
{
	p_dlg->PutMsg((char*)p_read_data_buf);
	Parse((char*)p_read_data_buf);
	WSocket::DataReceived(len);
}


//
//
//
void SMTPSocket::InitParent(CIRCDlg *parent)
{
	p_dlg = parent;
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
			char buf[2048];
			ZeroMemory(&buf, sizeof(buf));
			if(m_file.ReadString(temp) != FALSE)
			{
				if(temp.GetLength() < (sizeof(buf)+2))
				{
					_tcscpy(buf, temp);
					p_dlg->PutMsg(buf);
					strcat(buf, "\r\n");
					SendData(buf, lstrlen(buf));
				}
			}
			break;
		}
	case 354:
		{
			CString temp;
			char buf[2048];
			ZeroMemory(&buf, sizeof(buf));
			while(m_file.ReadString(temp))
			{
				if(temp.GetLength() < (sizeof(buf)+2))
				{
					_tcscpy(buf, temp);
					p_dlg->PutMsg(buf);
					strcat(buf, "\r\n");
					SendData(buf, lstrlen(buf));
				}
				if(temp.CompareNoCase(".")==0)
					break;
			}
			break;
		}
	}
}
