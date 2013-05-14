#include "StdAfx.h"
#include "winmxsocketstatusdata.h"

WinMxSocketStatusData::WinMxSocketStatusData(void)
{
	Clear();
}

//
//
//
WinMxSocketStatusData::~WinMxSocketStatusData(void)
{
}

//
//
//
void WinMxSocketStatusData::Clear()
{
	m_host.Clear();
	v_other_hosts.clear();
	memset(&m_connect_time,0,sizeof(CTime));
	memset(&m_up_time,0,sizeof(CTime));
}