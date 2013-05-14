#pragma once
#include "TorrentIndexRequest.h"
#include "Torrent.h"
#include <vector>
using namespace std;

#define TORRENT_MANAGER_EVENTS	5

class TorrentManagerThreadData
{
public:

	TorrentManagerThreadData(HANDLE aEvents[TORRENT_MANAGER_EVENTS])
		: m_pvSearchRequests(NULL), m_pvAddedTorrents(NULL), m_pGetConnectedPeers(NULL)
	{
		memcpy( m_aEvents, aEvents, sizeof(m_aEvents) );
	}

	~TorrentManagerThreadData(void)
	{
		if( m_pvSearchRequests != NULL )
		{
			for( size_t i = 0; i < m_pvSearchRequests->size(); i++ )
			{
				if( m_pvSearchRequests->at(i) != NULL )
				{
					delete m_pvSearchRequests->at(i);
				}
			}
			delete m_pvSearchRequests;
			m_pvSearchRequests = NULL;
		}

		if( m_pvAddedTorrents != NULL )
		{
			for( size_t i = 0; i < m_pvAddedTorrents->size(); i++ )
			{
				if( m_pvAddedTorrents->at(i) != NULL )
				{
					delete m_pvAddedTorrents->at(i);
				}
			}
			delete m_pvAddedTorrents;
			m_pvAddedTorrents = NULL;
		}

		if( m_pGetConnectedPeers != NULL )
		{
			delete m_pGetConnectedPeers;
			m_pGetConnectedPeers = NULL;
		}

		ZeroMemory( m_aEvents, sizeof(m_aEvents) );
	}

public:
	vector<TorrentIndexRequest *> *	m_pvSearchRequests;
	vector<Torrent *> *				m_pvAddedTorrents;
	const Hash20Byte *				m_pGetConnectedPeers;
	HANDLE							m_aEvents[TORRENT_MANAGER_EVENTS];
};
