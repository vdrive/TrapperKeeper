// NetworkBandwidthHost.cpp

#include "stdafx.h"
#include "NetworkBandwidthHost.h"

//
//
//
NetworkBandwidthHost::NetworkBandwidthHost()
{
	Clear();
}

//
//
//
void NetworkBandwidthHost::Clear()
{
	m_ip.clear();
	m_bandwidth=0;
	m_cpu_usage=0;
	m_upload_bandwidth=0;
	m_download_bandwidth=0;
	m_files_shared=0;
	m_last_reply_time = CTime::GetCurrentTime() - CTimeSpan(0,0,5,0); // so it's down by default
}

//
//
//
bool NetworkBandwidthHost::IsUp()
{
	CTimeSpan ts = CTime::GetCurrentTime() - m_last_reply_time;
	if(ts.GetTotalSeconds() > 300) //consider the computer is down if doesn't receive status reply in 5 mins
		return false;
	else
		return true;
}