#pragma once
#include "TorrentSearchRequest.h"
#include "Hash20Byte.h"
#include "IPPort.h"

class Peer;
class Torrent;
class TorrentSearchResult
{
public:
	TorrentSearchResult(TorrentSearchRequest *pRequest, const Torrent *pTorrent)
		: m_pTorrent(pTorrent), m_bComplete(false)
	{
		m_nConnectionModule = pRequest->m_nConnectionModule;
		m_nPeer = pRequest->m_nPeer;
		m_ip = pRequest->m_ip;
	}

	TorrentSearchResult(TorrentSearchRequest *pRequest)
		: m_pTorrent(NULL), m_bComplete(false)
	{
		m_nConnectionModule = pRequest->m_nConnectionModule;
		m_nPeer = pRequest->m_nPeer;
		m_ip = pRequest->m_ip;
	}

	~TorrentSearchResult(void) {}

public:
	size_t			m_nConnectionModule;
	size_t			m_nPeer;

	IPPort			m_ip;

	const Torrent *	m_pTorrent;

	bool			m_bComplete;
};
