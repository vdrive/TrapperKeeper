#pragma once
#include "tasyncsocket.h"
#include "KeyManager.h"
#include "afxwin.h"
#include "stdafx.h"
#include "UDPSearchResult.h"
#include "DCCommands.h"
#include "DB.h"
#include "ProjectKeywordsVector.h"
#include "DCDownload.h"
#include "DownloadManager.h"
#include <vector>

class DCScannerDlg;

class DCClientHubSocket :
	public TAsyncSocket
{
public:
	DCClientHubSocket(void);
	~DCClientHubSocket(void);
	int HubConnect(char *host,unsigned short port);
	void InitDlg(DCScannerDlg* dlg);
	void OnReceive(int error_code);		//overridden functions
	void OnConnect(int error_code);
	void OnClose(int error_code);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void ParseClientDataReceived(char * data, int len);	//parses the data that was received by the client
	bool GetConnected();
	bool GetSearch();
	unsigned int GetIntIPFromStringIP(char* ip);
	void GetProjectData(ProjectKeywordsVector *pk);	//project keywords received from dlg
	void GetDatabaseObject(DB &conn);	//pass database connection
	void GetDownloadManager(DownloadManager &dm);	//get download manager
	string GetRandomNick();
	CString GetToken(CString& s, LPCTSTR delim);

private:
	ProjectKeywordsVector *p_projkeywords;	//pointer to project keywords
	DCScannerDlg *p_dlg;
	DCCommands commands;
	UDPSearchResult udp_sr;
	KeyManager m_keymanager;
	CString m_key;
	bool b_search;		//okay or not okay to search
	bool b_connected;	//connected to hub
	DB *p_db;			//pointer to database object
	DCDownload *downloads;	//dcdownload object
	DownloadManager* m_dm;	//downloadmanager keeps track of all downloads
	string m_nick;
	string m_connection_status;
	string m_SRPasive;
	int m_rand_number;
	char m_hubname[200];
	//char connection_status[100];
	bool m_initialized;
	char MyNick[200];

};
