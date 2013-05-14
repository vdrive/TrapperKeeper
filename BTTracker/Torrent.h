#pragma once
#include "Hash20Byte.h"

class Peer;
class Torrent
{
// functions
public:
	Torrent(void);
	Torrent(const Torrent &rTorrent);
	~Torrent(void);

	void Clear(void);

	Torrent &operator=(const Torrent &rRight);
	inline bool operator==(const Torrent &rRight) { m_infoHash == rRight.m_infoHash; }
	inline bool operator!=(const Torrent &rRight) { m_infoHash != rRight.m_infoHash; }
	friend inline bool operator<(const Torrent &rLeft, const Torrent &rRight) { return rLeft.m_infoHash < rRight.m_infoHash; }
	friend inline bool operator>(const Torrent &rLeft, const Torrent &rRight) { return rLeft.m_infoHash > rRight.m_infoHash; }

// variables
public:
	Hash20Byte		m_infoHash;
	set<Peer> *		m_psPeers;

	int				m_nHaveCompleted;
	int				m_nLength;

	unsigned int	m_nSeedBase;
	unsigned int	m_nSeedMultiplier;
	unsigned int	m_nPeerBase;
	unsigned int	m_nPeerMultiplier;
	unsigned int	m_nCompletedBase;
	unsigned int	m_nCompletedMultiplier;
};
