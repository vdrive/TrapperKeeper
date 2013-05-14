#pragma once
#include "tasyncsocket.h"
#include "UsenetPosterDll.h"
#include "../UsenetSearcher/Header.h"

class UsenetPosterDll;

class UsenetSocket :
	public TAsyncSocket
{
public:
	UsenetSocket(void);
	~UsenetSocket(void);

	bool m_posting;
	bool m_is_active;	// this is the flag which is set to FALSE when we get banned, suspended, or disabled for posting.
						// basically whenever something bad happens.
	int m_port;
	int m_previous_buffer_len;

	char m_nickname[256];
	char m_server_name[256];
	char m_user[256];
	char m_password[256];
	char m_previous_buffer[8192];

	CTime m_last_server_response_time;

	vector<Header> v_headers;
	UsenetPosterDll *p_dll;
	
	void InitParent(UsenetPosterDll *dll);
	

	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);

	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void Parse(char *buf);

	void SendNextPost();
	void SendPostCommand();
	void DoNextPost();
	void LogPost();
};
