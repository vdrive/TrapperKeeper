#pragma once
#include "BTPacketParser.h"
#include "BTClient.h"
#include "Peer.h"
#include "IPPort.h"

//
//
//
class TorrentIndexRequest
{
public:

	TorrentIndexRequest(size_t nClient, const IPPort &rClient, const Peer &rPeer, BTPacketParser *pBTPacket)
		: m_nMod(-1), m_nClient(nClient), m_client(rClient), m_peer(rPeer), m_pBTPacket(pBTPacket)
	{
	}

	TorrentIndexRequest(size_t nClient, const IPPort &rClient, BTPacketParser *pBTPacket)
		: m_nMod(-1), m_nClient(nClient), m_client(rClient), m_peer(), m_pBTPacket(pBTPacket)
	{
	}

	~TorrentIndexRequest(void)
	{
		if( m_pBTPacket != NULL )
		{
			delete m_pBTPacket;
			m_pBTPacket = NULL;
		}
	}

public:
	size_t				m_nMod;
	size_t				m_nClient;
	IPPort				m_client;

	Peer				m_peer;

	BTPacketParser *	m_pBTPacket;
};
