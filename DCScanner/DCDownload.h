#pragma once
#include "tasyncsocket.h"
#include "KeyManager.h"
#include "DB.h"

class DCDownload :
	public TAsyncSocket
{
public:
	DCDownload(void);
	~DCDownload(void);
	void OnReceive(int error_code);
	void OnConnect(int error_code);
	void OnClose(int error_code);
	int Close();
	int DownloadConnect(char *host,unsigned short port, bool init);
	int Disconnect();
	void GetLoginData(char* data);
	void ParseData(char* data, int data_len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned max_len);
	void GetDBConnection(DB* db);
	void SendJunk();
	int GetPort();		//returns the port
	int CreatePort();	//creates a random port socket 

private:
	CString Key;
	char m_login[200];
	KeyManager m_keymanager;	//key manager object
	bool m_sending;		//flag that represents currently sending data
	DB * p_db;		//pointer to database object
	char * file_junk;
	int m_listening_port;	//port that is listening for incoming connections
};
