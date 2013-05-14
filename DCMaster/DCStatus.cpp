#include "StdAfx.h"
#include "dcstatus.h"

DCStatus::DCStatus(void)
{
	memset(&m_ip, 0, sizeof(m_ip));
	m_last_init_response_time = CTime::GetCurrentTime();
}

//
//
//
DCStatus::~DCStatus(void)
{
}

//
//
//
bool DCStatus::operator == (const DCStatus& dc_status) const
{
	if(strcmp(this->m_ip, dc_status.m_ip)==0)
		return true;
	else
		return false;
}

//
//
//
bool DCStatus::operator < (const DCStatus& dc_status) const
{
	if(strcmp(this->m_ip, dc_status.m_ip) < 0)
		return true;
	else
		return false;
}

//
//
//
bool DCStatus::operator == (char* ip) const
{
	if(strcmp(this->m_ip, ip)==0)
		return true;
	else
		return false;
}
