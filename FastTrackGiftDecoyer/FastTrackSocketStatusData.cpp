// GnutellaSocketStatusData.cpp

#include "stdafx.h"
#include "FastTrackSocketStatusData.h"


//
//
//
FastTrackSocketStatusData::FastTrackSocketStatusData()
{
	Clear();
}

//
//
//
FastTrackSocketStatusData::~FastTrackSocketStatusData()
{
}

//
//
//
void FastTrackSocketStatusData::Clear()
{
	m_host.Clear();
	m_remote_vendor.erase();
	memset(&m_connect_time,0,sizeof(CTime));
	memset(&m_up_time,0,sizeof(CTime));
}