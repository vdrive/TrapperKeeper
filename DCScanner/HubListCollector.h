#pragma once
#include "tasyncsocket.h"
#include "hub.h"
#include <vector>

class DCScannerDlg;
class hublistcollector :
	public TAsyncSocket
{
public:
	void InitDlg(DCScannerDlg *dlg);
	hublistcollector(void);
	~hublistcollector(void);
	int Connect(char* host, unsigned short port);
	void OnReceive(int error_code);		//overridden functions
	void OnConnect(int error_code);
	void OnClose(int error_code);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	vector<Hub> GetHubList();	
	void DownloadHubList();	//downloads the hub list
	bool FileCheck(LPSTR lpszFilename);
	DCScannerDlg *p_dlg;
private:
	CString m_data;
	CString m_hubdata;
	vector<Hub> hubs;
	char* buf;
};
