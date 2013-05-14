#include "Stdafx.h"
#include "clientdata.h"


ClientData::ClientData(void)
{
	strcpy(m_torrentname,"NULL");
	strcpy(m_ip,"0.0.0.0");
	m_bytes = 0;
	m_bytes_sent = 0;
	m_is_seed = false;
}

ClientData::ClientData(char * name, char * ip, int bytes)
{
	strcpy(m_torrentname,name);
	strcpy(m_ip,ip);
	m_bytes = bytes;
	m_bytes_sent = 0;
	m_is_seed = false;
}

ClientData::ClientData(char * name, char * ip, int bytes, CTime firsttime, CTime lasttime)
{
	strcpy(m_torrentname,name);
	strcpy(m_ip,ip);
	m_bytes = bytes;
	m_bytes_sent = 0;
	m_first_time = firsttime;
	m_last_time = lasttime;
	m_is_seed = false;
}

ClientData::~ClientData(void)
{
}

ClientData & ClientData::operator =(const ClientData &cd)
{
	strcpy(m_torrentname,cd.m_torrentname);
	strcpy(m_ip,cd.m_ip);
	m_bytes = cd.m_bytes;
	m_bytes_sent = cd.m_bytes_sent;
	m_first_time = cd.m_first_time;
	m_last_time = cd.m_last_time;
	m_is_seed = cd.m_is_seed;
	
	return *this;
}
