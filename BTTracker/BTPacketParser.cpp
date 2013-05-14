#include "StdAfx.h"
#include "BTPacketParser.h"
#include <winsock2.h>

//
//
//
BTPacketParser::BTPacketParser(const char *pData, const size_t nSize)
: m_ePacketType(TYPE_UNINIT), m_infoHash(), m_peerID(), m_nIP(0), m_nPort(0), m_nUploaded(0), m_nDownloaded(0)
, m_nLeft(0), m_eEvent(EMPTY), m_bNoPeerID(false), m_nNumWant(50), m_trackerID(), m_bCompact(false)
{
	if( pData == NULL || nSize == 0 )
		return;

	// Init
	ZeroMemory( m_aKey, sizeof( m_aKey ) );

	// Copy the received packet into a string for processing
	string recvPacket( pData, nSize );

	// IF the packet is a GET packet
	if( recvPacket.compare( 0, 4, "GET " ) == 0 )
	{
		size_t nLoc = 0;

		// Determine if it is an announce or scrape packet
		if( recvPacket.find( " /announce?" ) != -1 || recvPacket.find( " /announce/announce?" ) != -1 ||
			recvPacket.find( " ?info_hash=" ) != -1 )
		{
			m_ePacketType = TYPE_GET_ANNOUNCE;
		}
		else if( recvPacket.find( " /scrape?" ) != -1 || recvPacket.find( " /scrape/scrape?" ) != -1 )
		{
			m_ePacketType = TYPE_GET_SCRAPE;
		}
		else
		{
			m_ePacketType = TYPE_INVALID_URL;
			return;
		}

		// Find the infohash the client is requesting
		nLoc = recvPacket.find( "info_hash=" );
		if( nLoc == -1 || (nLoc + 10 + 19) > nSize )
		{
			m_ePacketType = TYPE_MISSING_INFOHASH;
			return;
		}
		else
		{
			m_infoHash.FromEscapedHexString( pData + nLoc + 10 );
		}

        // IF the packet is just a scrape packet, all that is needed is the infohash,
		//  otherwise more fields must occur and be parsed
		if( m_ePacketType == TYPE_GET_ANNOUNCE )
		{
			// Find the client's peer id
			nLoc = recvPacket.find( "peer_id=" );
			if( nLoc == -1 || (nLoc + 8 + 19) > nSize )
			{
				m_ePacketType = TYPE_PARSE_ERROR;
				return;
			}
			else
			{
				m_peerID.FromEscapedHexString( pData + nLoc + 8 );
			}

			// Find the client's port
			nLoc = recvPacket.find( "port=" );
			if( nLoc == -1 )
			{
				m_ePacketType = TYPE_PARSE_ERROR;
				return;
			}
			else
			{
				sscanf( recvPacket.c_str() + nLoc, "port=%d", &m_nPort );
				if( m_nPort <= 0 || m_nPort > 0xffff )
				{
					m_ePacketType = TYPE_PARSE_ERROR;
					return;
				}
			}

			// Get uploaded count
			nLoc = recvPacket.find( "uploaded=" );
			if( nLoc == -1 )
			{
				m_ePacketType = TYPE_PARSE_ERROR;
				return;
			}
			else
			{
				sscanf( recvPacket.c_str() + nLoc, "uploaded=%d", &m_nUploaded );
			}

			// Get downloaded count
			nLoc = recvPacket.find( "downloaded=" );
			if( nLoc == -1 )
			{
				m_ePacketType = TYPE_PARSE_ERROR;
				return;
			}
			else
			{
				sscanf( recvPacket.c_str() + nLoc, "downloaded=%d", &m_nDownloaded );
			}

			// Get left (to download) count
			nLoc = recvPacket.find( "left=" );
			if( nLoc == -1 )
			{
				m_ePacketType = TYPE_PARSE_ERROR;
				return;
			}
			else
			{
				sscanf( recvPacket.c_str() + nLoc, "left=%d", &m_nLeft );
			}

			// Get the status of the client
			if( recvPacket.find( "event=started" ) != -1 )
				m_eEvent = STARTED;
			else if( recvPacket.find( "event=completed" ) != -1 )
				m_eEvent = COMPLETED;
			else if( recvPacket.find( "event=stopped" ) != -1 )
				m_eEvent = STOPPED;

			// Get the IP of the client (OPTIONAL)
			nLoc = recvPacket.find( "ip=" );
			if( nLoc != -1 )
			{
				DWORD nA = 0;
				DWORD nB = 0;
				DWORD nC = 0;
				DWORD nD = 0;
				sscanf( recvPacket.c_str() + nLoc, "ip=%d.%d.%d.%d", &nA, &nB, &nC, &nD );

				m_nIP = (nA&0xff) | ((nB&0xff) << 8) | ((nC&0xff) << 16) | ((nD&0xff) << 24);

				if( m_nIP == 0 )
				{
					m_ePacketType = TYPE_PARSE_ERROR;
					return;
				}
			}

			// Does the client accept a "no peer id" response?
			m_bNoPeerID = ( recvPacket.find( "no_peer_id=1" ) != -1 );

			// Does the client accept a "compact" response? (Should be optional, but we will require it)
			nLoc = recvPacket.find( "compact=1" );
			if( nLoc == -1 )
			{
				m_bCompact = true;
				/*m_ePacketType = TYPE_NON_COMPACT;
				return;*/
			}

			// 
			nLoc = recvPacket.find( "numwant=" );
			if( nLoc != -1 )
			{
				sscanf( recvPacket.c_str() + nLoc, "numwant=%d", &m_nNumWant );
			}

			// 
			nLoc = recvPacket.find( "key=" );
			if( nLoc != -1 && (nLoc + 4 + 5) > nSize )
			{
				memcpy( m_aKey, pData + nLoc + 4, 6 );
			}

			// 
			nLoc = recvPacket.find( "trackerid=" );
			if( nLoc != -1 && (nLoc + 10 + 19) > nSize )
			{
				m_trackerID.FromEscapedHexString( pData + nLoc + 10 );
			}
		}
	}
	else
	{
		m_ePacketType = TYPE_PARSE_ERROR;
		return;
	}
}

//
//
//
BTPacketParser::~BTPacketParser(void)
{
}
