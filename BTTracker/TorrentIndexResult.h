#pragma once
#include "TorrentIndexRequest.h"
#include "BTPacketParser.h"
#include "Hash20Byte.h"
#include "BTClient.h"
#include "IPPort.h"
#include "Peer.h"

//
//
//
class TorrentIndexResult
{
public:

	TorrentIndexResult(const TorrentIndexRequest &rTorrentIndexRequest)
		: m_infohash(), m_bFoundTorrent(false), m_nComplete(0), m_nIncomplete(0)
		, m_nHaveCompleted(0), m_sPeers(), m_reportedMDIP(0,0)
	{
		m_nMod = rTorrentIndexRequest.m_nMod;
		m_nClient = rTorrentIndexRequest.m_nClient;
		m_client = rTorrentIndexRequest.m_client;
		m_bAnnounce = ( rTorrentIndexRequest.m_pBTPacket->GetPacketType() == BTPacketParser::BTPacketType::TYPE_GET_ANNOUNCE );
	}

	~TorrentIndexResult(void)
	{
	}

public:
	Hash20Byte		m_infohash;

	size_t			m_nMod;
	size_t			m_nClient;
	IPPort			m_client;

	bool			m_bFoundTorrent;

	int				m_nComplete;
	int				m_nIncomplete;
	int				m_nHaveCompleted;

	set<Peer>		m_sPeers;

	bool			m_bAnnounce;

	IPPort			m_reportedMDIP;
};
