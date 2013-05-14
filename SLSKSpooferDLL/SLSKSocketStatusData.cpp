// GnutellaSocketStatusData.cpp

#include "stdafx.h"
#include "SLSKSocketStatusData.h"


//
//
//
SLSKSocketStatusData::SLSKSocketStatusData()
{
	Clear();
}

//
//
//
SLSKSocketStatusData::~SLSKSocketStatusData()
{
}

//
//
//
void SLSKSocketStatusData::Clear()
{
	m_host.Clear();
	//m_remote_vendor.erase();
	memset(&m_connect_time,0,sizeof(CTime));
	memset(&m_up_time,0,sizeof(CTime));
}