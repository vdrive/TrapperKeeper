// ConnectionModuleThreadData.h

#pragma once
#include "WSocket.h"
#include "TorrentSearchResult.h"

class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData(WSAEVENT aReservedEvents[4])
		: m_pqAcceptedSockets(NULL), m_pqSearchResults(NULL), m_bRepair(false), m_bTimer(false)
	{
		memcpy( m_aReservedEvents, aReservedEvents, sizeof( m_aReservedEvents ) );
	}

	~ConnectionModuleThreadData()
	{
		memset( m_aReservedEvents, 0, sizeof( m_aReservedEvents ) );

		if( m_pqAcceptedSockets != NULL )
		{
			delete m_pqAcceptedSockets;
			m_pqAcceptedSockets = NULL;
		}

		if( m_pqSearchResults != NULL )
		{
			while( !m_pqSearchResults->empty() )
			{
				if( m_pqSearchResults->front() != NULL )
				{
					delete m_pqSearchResults->front();
				}
				m_pqSearchResults->pop();
			}
			m_pqSearchResults = NULL;
		}
	}

public:
	WSAEVENT							m_aReservedEvents[4];
	queue<SOCKET> *						m_pqAcceptedSockets;
	queue<TorrentSearchResult *> *		m_pqSearchResults;

	bool								m_bRepair;
	bool								m_bTimer;
};