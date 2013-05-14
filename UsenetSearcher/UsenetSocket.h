#pragma once
#include "tasyncsocket.h"
#include "ProjectKeywords.h"
#include "Newsgroup.h"
#include "Header.h"

#define WM_SCAN_THREAD		WM_USER+1

class UsenetSearcherDll;

class UsenetSocket :
	public TAsyncSocket
{
public:
	UsenetSocket(void);
	~UsenetSocket(void);

	void InitParent(UsenetSearcherDll *dll);
	UsenetSearcherDll *p_dll;

	int m_port;
	int m_previous_buffer_len;
	int m_newsgroup_index;
	int m_first_header_index;
	int m_last_header_index;
	int m_new_headers;

	DWORD m_group_tick;
	DWORD m_full_scan_tick;

	char m_server_name[256];
	char m_user[256];
	char m_password[256];
	char m_previous_buffer[8192];
	char m_nickname[256];

	bool m_receiving_headers;
	bool m_searching;

	CTime m_watchdog;

	vector<Newsgroup> v_newsgroups;
	vector<ProjectKeywords> v_my_projects;

	vector<Header> v_new_headers;

	// Overloaded functions:
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	void GetNewsGroups();
	void Parse(char* buf);
	void DoNextNewsGroup();
	void RequestNewHeaders();
	void LogHeader(char *buf);
	void StartWorkerThread();
};
