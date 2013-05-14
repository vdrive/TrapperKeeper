#pragma once
#include "Hash20Byte.h"
#include "IPPort.h"

class Peer;
class TorrentSearchRequest
{
private:
	friend class TorrentSearchResult;

public:
	TorrentSearchRequest(const unsigned char *pHash, size_t nPeer, const IPPort &rIP)
	{
		m_hash = pHash;
		m_nPeer = nPeer;
		m_ip = rIP;
	}

	~TorrentSearchRequest(void) {}

	inline void SetConnectionModule(size_t nModule) { m_nConnectionModule = nModule; }
	inline Hash20Byte GetHash(void) const { return m_hash; }

private:
	Hash20Byte		m_hash;

	size_t			m_nConnectionModule;
	size_t			m_nPeer;

	IPPort			m_ip;
};
