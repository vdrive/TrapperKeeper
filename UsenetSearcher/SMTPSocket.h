// SMTPSocket.h: interface for the SMTPSocket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SMTP_SOCKET_H
#define SMTP_SOCKET_H

#include "TAsyncSocket.h"

class UsenetSearcherDll;
class SMTPSocket : 
	public TAsyncSocket  
{
public:

	void InitParent(UsenetSearcherDll* parent);
	SMTPSocket();
	~SMTPSocket();

	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnAccept(int error_code);
	void OnClose(int error_code);

	void SendMail(char *msg);
	CString m_email_message;

	int m_previous_buffer_len;
	char m_previous_buffer[8192];

private:
	void Parse(char* buf);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	UsenetSearcherDll* p_dll;
	CStdioFile m_file;
};

#endif
