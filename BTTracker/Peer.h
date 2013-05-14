#pragma once
#include "IPPort.h"

class Peer :
	public IPPort
{
public:
	Peer(void);
	Peer(const IPPort &rPeer);
	Peer(const Peer &rPeer);
	~Peer(void) {}

	inline void UpdateLastSeen(void) { m_lastSeen = CTime::GetCurrentTime(); }
	inline void UpdateLastSeen(const CTime &rTime) { m_lastSeen = rTime; }
	void UpdateLastSeen(const char aTime[15]);
	inline int SecondsSinceLastSeen(void) const { return ( (int)(CTime::GetCurrentTime() - m_lastSeen).GetTotalSeconds() ); }

	inline void SetComplete(void) { m_bComplete = true; }
	inline void SetIncomplete(void) { m_bComplete = false; }
	inline bool IsComplete(void) const { return m_bComplete; }

	inline void SetMD(void) { m_bIsMDPeer = true; }
	inline void UnSetMD(void) { m_bIsMDPeer = false; }
	inline bool IsMD(void) const { return m_bIsMDPeer; }

	inline void SetReportedMDIP(const Peer &rPeer) { m_reportedMDIP = rPeer; }
	inline IPPort GetReportedMDIP(void) const { return m_reportedMDIP; }

	Peer &operator=(const Peer &rRight);
	inline bool operator==(const Peer &rRight) const { return ( m_nIP == rRight.m_nIP ); }
	inline bool operator!=(const Peer &rRight) const { return ( m_nIP != rRight.m_nIP ); }
	friend inline bool operator<(const Peer &rLeft, const Peer &rRight) { return ( rLeft.m_nIP < rRight.m_nIP ); }
	friend inline bool operator>(const Peer &rLeft, const Peer &rRight) { return ( rLeft.m_nIP > rRight.m_nIP ); }

private:
	CTime		m_lastSeen;

	bool		m_bComplete;
	bool		m_bIsMDPeer;

	IPPort		m_reportedMDIP;
};
