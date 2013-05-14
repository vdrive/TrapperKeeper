#pragma once

#include <afx.h>

class ClientData
{
public:
	ClientData(void);
	ClientData(char * name, char * ip, int bytes);
	ClientData(char * name, char * ip, int bytes, CTime firsttime, CTime lasttime);
	~ClientData(void);

	ClientData & operator=(const ClientData & cd);

	char m_torrentname[256+1];
	char m_ip[15+1];
	int m_bytes;
	int m_bytes_sent;
	CTime m_first_time;
	CTime m_last_time;

	bool m_is_seed;

private:

};
