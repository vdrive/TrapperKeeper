#pragma once
#include "tasyncsocket.h"
#include "KeyManager.h"

class DCDownload :
	public TAsyncSocket
{
public:
	DCDownload(const DCDownload &dc);
	DCDownload(void);
	~DCDownload(void);
	void OnReceive(int error_code);
	void OnConnect(int error_code);
	void OnClose(int error_code);
	int DownloadConnect(char *host,unsigned short port);
	void GetLoginData(char* data);
	bool GetConnectionStatus();
	void ParseData(char* data, int data_len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned max_len);
	DCDownload operator = (const DCDownload& dc);
private:
	char m_login[200];
	KeyManager m_keymanager;	//key manager object
	bool b_connected;		//flag the represents connected via tcp
};
