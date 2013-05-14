// SMTPSocket.h: interface for the SMTPSocket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SMTP_SOCKET_H
#define SMTP_SOCKET_H

#include "TAsyncSocket.h"

class CIRCDlg;
class SMTPSocket : public TAsyncSocket  
{
public:
	void InitParent(CIRCDlg* parent);
	SMTPSocket();
	~SMTPSocket();
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnAccept(int error_code);
	void OnClose(int error_code);
private:
	void Parse(char* buf);
	void DataReceived(unsigned int len);
	CIRCDlg* p_dlg;
	CStdioFile m_file;
};

#endif
