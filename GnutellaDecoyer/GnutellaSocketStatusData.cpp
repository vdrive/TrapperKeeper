// GnutellaSocketStatusData.cpp

#include "stdafx.h"
#include "GnutellaSocketStatusData.h"


//
//
//
GnutellaSocketStatusData::GnutellaSocketStatusData()
{
	Clear();
}

//
//
//
GnutellaSocketStatusData::~GnutellaSocketStatusData()
{
}

//
//
//
void GnutellaSocketStatusData::Clear()
{
	m_host.Clear();

	m_remote_vendor.erase();
	memset(&m_connect_time,0,sizeof(CTime));
	memset(&m_up_time,0,sizeof(CTime));
	m_compress_incoming_msg=false;
	m_compress_outgoing_msg=false;
}