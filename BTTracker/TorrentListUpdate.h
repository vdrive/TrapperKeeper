#pragma once
#include "Hash20Byte.h"
#include "IPPort.h"
#include "Peer.h"
#include <string>
using namespace std;

//
//
//
class TorrentListUpdate
{
public:
	TorrentListUpdate(void)
		: m_torrentName(), m_nSeedsReal(0), m_nSeedsInflated(0)
		, m_nPeersReal(0), m_nPeersInflated(0), m_nCompletedInflated(0)
		, m_nCompletedReal(0), m_bRemove(false)
	{
		ZeroMemory( m_aInfoHash, sizeof(m_aInfoHash) );
	}

	~TorrentListUpdate(void) {}

	inline void SetTorrentName(const string &rTorrentName) { m_torrentName = rTorrentName; }
	inline string GetTorrentName(void) const { return m_torrentName; }

	inline void SetInfoHash(const string &rInfoHash) { strcpy( m_aInfoHash, rInfoHash.c_str() ); }
	inline void SetInfoHash(const Hash20Byte &rInfoHash) { rInfoHash.CopyPrintableToCharBuf( m_aInfoHash ); }
	inline const char *GetInfoHash(void) const { return m_aInfoHash; } 

	inline void SetSeedsReal(int nSeedsReal) { m_nSeedsReal = nSeedsReal; }
	inline void SetSeedsInflated(int nSeedsInflated) { m_nSeedsInflated = nSeedsInflated; }
	inline int GetSeedsReal(void) const { return m_nSeedsReal; }
	inline int GetSeedsInflated(void) const { return m_nSeedsInflated; }

	inline void SetPeersReal(int nPeersReal) { m_nPeersReal = nPeersReal; }
	inline void SetPeersInflated(int nPeersInflated) { m_nPeersInflated = nPeersInflated; }
	inline int GetPeersReal(void) const { return m_nPeersReal; }
	inline int GetPeersInflated(void) const { return m_nPeersInflated; }

	inline void SetCompletedReal(int nCompletedReal) { m_nCompletedReal = nCompletedReal; }
	inline int GetCompletedReal(void) const { return m_nCompletedReal; }
	inline void SetCompletedInflated(int nCompletedInflated) { m_nCompletedInflated = nCompletedInflated; }
	inline int GetCompletedInflated(void) const { return m_nCompletedInflated; }

	inline void SetRemove(void) { m_bRemove = true; }
	inline int IsRemove(void) const { return m_bRemove; }

private:
	string			m_torrentName;
	char			m_aInfoHash[41];

	int				m_nSeedsReal;
	int				m_nSeedsInflated;

	int				m_nPeersReal;
	int				m_nPeersInflated;

	int				m_nCompletedReal;
	int				m_nCompletedInflated;

	bool			m_bRemove;
};
