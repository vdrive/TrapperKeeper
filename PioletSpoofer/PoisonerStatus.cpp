#include "StdAfx.h"
#include "poisonerstatus.h"

PoisonerStatus::PoisonerStatus(void)
{
	memset(&m_ip_str, 0, sizeof(m_ip));
	m_ip=0;
	m_last_init_response_time = CTime::GetCurrentTime();
}

//
//
//
PoisonerStatus::~PoisonerStatus(void)
{
}

//
//
//
//
//
//
bool PoisonerStatus::operator == (const PoisonerStatus& dc_status) const
{
	if(m_ip == dc_status.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool PoisonerStatus::operator == (UINT ip) const
{
	if(m_ip == ip)
		return true;
	else
		return false;
}
