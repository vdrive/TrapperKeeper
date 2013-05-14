#include "StdAfx.h"
#include "Torrent.h"
#include "Peer.h"

//
//
//
Torrent::Torrent()
: m_infoHash(), m_psPeers(NULL), m_nHaveCompleted(0), m_nLength(0), m_nSeedBase(0)
, m_nSeedMultiplier(1), m_nPeerBase(0), m_nPeerMultiplier(1), m_nCompletedBase(0)
, m_nCompletedMultiplier(1)
{
	m_psPeers = new set<Peer>;
}

//
//
//
Torrent::Torrent(const Torrent &rTorrent)
: m_infoHash(rTorrent.m_infoHash), m_psPeers(NULL), m_nHaveCompleted(rTorrent.m_nHaveCompleted)
, m_nLength(rTorrent.m_nLength), m_nSeedBase(rTorrent.m_nSeedBase)
, m_nSeedMultiplier(rTorrent.m_nSeedMultiplier), m_nPeerBase(rTorrent.m_nPeerBase)
, m_nPeerMultiplier(rTorrent.m_nPeerMultiplier), m_nCompletedBase(rTorrent.m_nCompletedBase)
, m_nCompletedMultiplier(rTorrent.m_nCompletedMultiplier)
{
	m_psPeers = new set<Peer>;
	*m_psPeers = *(rTorrent.m_psPeers);
}

//
//
//
Torrent::~Torrent()
{
	if( m_psPeers != NULL )
	{
		delete m_psPeers;
		m_psPeers = NULL;
	}
}

//
//
//
void Torrent::Clear(void)
{
	m_infoHash.ZeroHash();
	m_psPeers->clear();
	m_nHaveCompleted = 0;
	m_nLength = 0;
	m_nSeedBase = 0;
	m_nSeedMultiplier = 1;
	m_nPeerBase = 0;
	m_nPeerMultiplier = 1;
	m_nCompletedBase = 0;
	m_nCompletedMultiplier = 1;
}

//
//
//
Torrent &Torrent::operator=(const Torrent &rRight)
{
	m_infoHash = rRight.m_infoHash;

	if( m_psPeers == NULL )
		m_psPeers = new set<Peer>;
	else
        m_psPeers->clear();

	*m_psPeers = *(rRight.m_psPeers);

	m_nHaveCompleted = rRight.m_nHaveCompleted;
	m_nLength = rRight.m_nLength;
	m_nSeedBase = rRight.m_nSeedBase;
	m_nSeedMultiplier = rRight.m_nSeedMultiplier;
	m_nPeerBase = rRight.m_nPeerBase;
	m_nPeerMultiplier = rRight.m_nPeerMultiplier;
	m_nCompletedBase = rRight.m_nCompletedBase;
	m_nCompletedMultiplier = rRight.m_nCompletedMultiplier;

	return (*this);
}
