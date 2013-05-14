#pragma once
#include "WSocket.h"
#include "TorrentSearchRequest.h"
#include <queue>
#include <map>
using namespace std;

class TorrentManagerThreadData
{
public:

	TorrentManagerThreadData(HANDLE aEvents[])
		: m_pqSearches(NULL), m_pmpTorrents(NULL)
	{
		memcpy( m_aEvents, aEvents, sizeof( HANDLE ) * TMANAGER_EVENTS );
	}

	~TorrentManagerThreadData(void)
	{
		memset( m_aEvents, 0, sizeof( HANDLE ) * TMANAGER_EVENTS );

		if( m_pqSearches != NULL )
		{
			while( !m_pqSearches->empty() )
			{
				if( m_pqSearches->front() != NULL )
				{
					delete m_pqSearches->front();
				}
				m_pqSearches->pop();
			}
			m_pqSearches = NULL;
		}

		if( m_pmpTorrents != NULL )
		{
			while( m_pmpTorrents->size() > 0 )
			{
				delete m_pmpTorrents->begin()->second;
				m_pmpTorrents->erase( m_pmpTorrents->begin() );
			}
			m_pmpTorrents = NULL;
		}
	}

public:
	HANDLE m_aEvents[TMANAGER_EVENTS];
	queue<TorrentSearchRequest *> *m_pqSearches;
	map<Hash20Byte, const Torrent *> *m_pmpTorrents;
};
