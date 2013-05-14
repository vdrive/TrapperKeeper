#include "StdAfx.h"
#include "Peer.h"

//
//
//
Peer::Peer(void)
: m_lastSeen(CTime::GetCurrentTime()), m_bComplete(false), m_bIsMDPeer(false)
, m_reportedMDIP(0,0)
{
}

//
//
//
Peer::Peer(const IPPort &rPeer)
: m_lastSeen(CTime::GetCurrentTime()), m_bComplete(false), m_bIsMDPeer(false)
, m_reportedMDIP(0,0)
{
	SetIP( rPeer.GetIP() );
	SetPort( rPeer.GetPort() );
}

//
//
//
Peer::Peer(const Peer &rPeer)
: m_lastSeen(rPeer.m_lastSeen), m_bComplete(rPeer.m_bComplete), m_bIsMDPeer(rPeer.m_bIsMDPeer)
, m_reportedMDIP(rPeer.m_reportedMDIP)
{
	m_nIP = rPeer.m_nIP;
	m_nPort = rPeer.m_nPort;
}

//
//
//
void Peer::UpdateLastSeen(const char aTime[15])
{
	if( aTime == NULL )
		return;

	int nYear, nMonth, nDay, nHour, nMinute, nSecond;

	sscanf( aTime, "%4d%2d%2d%2d%2d%2d", &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond );

	m_lastSeen = CTime( nYear, nMonth, nDay, nHour, nMinute, nSecond );
}

//
//
//
Peer &Peer::operator=(const Peer &rRight)
{
	m_lastSeen = rRight.m_lastSeen;

	m_bComplete = rRight.m_bComplete;
	m_bIsMDPeer = rRight.m_bIsMDPeer;

	m_reportedMDIP = rRight.m_reportedMDIP;

	m_nIP = rRight.m_nIP;
	m_nPort = rRight.m_nPort;

	return (*this);
}
