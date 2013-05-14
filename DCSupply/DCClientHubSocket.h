#pragma once
#include "tasyncsocket.h"
#include "KeyManager.h"
#include "afxwin.h"
#include "stdafx.h"
#include "UDPSearchResult.h"
#include "DCCommands.h"
#include "Hub.h"
#include "DCDownload.h"
#include "DownloadManager.h"
#include <vector>

class DCSupplyDlg;

class DCClientHubSocket :
	public TAsyncSocket
{
public:
	DCClientHubSocket(void);
	~DCClientHubSocket(void);
	int HubConnect(char *host,unsigned short port);
	void InitDlg(DCSupplyDlg* dlg);
	void OnReceive(int error_code);		//overridden functions
	void OnConnect(int error_code);
	void OnClose(int error_code);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void Search(string search_for);
	void ParseClientDataReceived(char * data, int len);	//parses the data that was received by the client
	bool GetConnected();
	void Disconnect();	//disconnect from hub
	bool GetSearch();
	unsigned int GetIntIPFromStringIP(char* ip);
	bool CheckKillwords(char* search_string);
	bool CheckKeywords(char* search_string);
	char* GetConnectedHost();	//returns the name of the connected host
	void SetSocket(int socket);
	void GetRetryHubObject(vector<Hub> &current_hub);
	void GetDownloadManager(DownloadManager &dm);	//get download manager

private:
	DCDownload* downloads;
	vector<Hub> *m_hub;
	DCSupplyDlg *p_dlg;
	DCCommands commands;
	//UDPSearchResult udp_sr;
	KeyManager m_keymanager;
	CString m_key;
	int m_socket;	//socket
	bool b_search;		//okay or not okay to search
	bool b_connected;	//connected to hub
	char* m_host;
	char space[100];
	bool test_tcp;
	DownloadManager* m_dm;	//download manager object from dlg class
};
