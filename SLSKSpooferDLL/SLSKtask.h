

#pragma once
#include "buffer.h"

struct track 
{
public:
	int m_track_number;
	CString m_track_name;
};

class SLSKtask
{
public:
	CString m_user_name;
	CString m_ip;
	CString m_artist;
	CString m_album;
	int m_port;
	int m_state;
	//int m_project_id;
	int m_token;
//	vector<track> tracks;
//public:
	SLSKtask(void);
//	SLSKtask(CString username, buffer Buffer);
	//SLSKtask(CString username, CString connection_ip, buffer Buffer, int Port);
	~SLSKtask(void);
//	void setUN(CString username);
	//void setBuf(buffer &Buffer);
	void setIP(CString connection_ip);
	//void setPort(int Port);
	//CString getUN(void);
	CString getIP(void);
	//buffer getBuf(void);
	//int getPort(void);
};

