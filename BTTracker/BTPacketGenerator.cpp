#include "StdAfx.h"
#include "BTPacketGenerator.h"
#include "TorrentIndexResult.h"

#define REREQUEST_INTERVAL	305

//
//
//
BTPacketGenerator::BTPacketGenerator(void)
: m_pFailureReason(NULL), m_pWarningMessage(NULL), m_nInterval(0), m_nMinInterval(0)
, m_trackerID(), m_nComplete(0), m_nIncomplete(0), m_nHaveCompleted(0), m_sPeers()
, m_scrapeHash(), m_bAnnounce(true)
{
}

//
//
//
BTPacketGenerator::BTPacketGenerator(TorrentIndexResult *pTorrentIndexResult)
: m_pFailureReason(NULL), m_pWarningMessage(NULL), m_nInterval(0), m_nMinInterval(0)
, m_trackerID(), m_nComplete(0), m_nIncomplete(0), m_nHaveCompleted(0), m_sPeers()
, m_scrapeHash(), m_bAnnounce(true)
{
	if( pTorrentIndexResult == NULL )
		return;

	// IF the torrent was found in the search, return the peers
	if( pTorrentIndexResult->m_bFoundTorrent )
	{
		m_bAnnounce = pTorrentIndexResult->m_bAnnounce;

		if( pTorrentIndexResult->m_bAnnounce )
		{
			SetComplete( pTorrentIndexResult->m_nComplete );
			SetIncomplete( pTorrentIndexResult->m_nIncomplete );
			SetInterval( REREQUEST_INTERVAL );
			SetMinInterval( REREQUEST_INTERVAL );

			vector<Peer> vSeedlessPeers;
			for( set<Peer>::iterator iter = pTorrentIndexResult->m_sPeers.begin(); iter != pTorrentIndexResult->m_sPeers.end(); iter++ )
			{
				if( !iter->IsMD() && iter->GetIP() != pTorrentIndexResult->m_client.GetIP() )
					vSeedlessPeers.push_back( *iter );
			}

			set<Peer> sResultPeers;

			// IF there are more than 49 peers, randomly select 49 to return
			if( vSeedlessPeers.size() > 49 )
			{
				while( sResultPeers.size() < 49 )
					sResultPeers.insert( vSeedlessPeers[ randInt( 0, (int)vSeedlessPeers.size()-1 ) ] );

			}
			// ELSE there are less than or equal to 50 peers, return all of them
			else
			{
				for( size_t i = 0; i < vSeedlessPeers.size(); i++ )
					sResultPeers.insert( vSeedlessPeers[i] );
			}

			Peer mdPeer;
			mdPeer.SetIP( pTorrentIndexResult->m_reportedMDIP.GetIPForDB() );
			mdPeer.SetPort( pTorrentIndexResult->m_reportedMDIP.GetPort() );

			sResultPeers.insert( mdPeer );

			SetPeers( sResultPeers );
		}
		else
		{
			SetComplete( pTorrentIndexResult->m_nComplete );
			SetIncomplete( pTorrentIndexResult->m_nIncomplete );
			SetHaveCompleted( pTorrentIndexResult->m_nHaveCompleted );

			SetScrapeHash( pTorrentIndexResult->m_infohash );
		}
	}
	// ELSE the torrent could not be found, return error
	else
	{
		SetFailureReason( FAILURE_UNAUTHORIZED_TORRENT );
	}
}

//
//
//
BTPacketGenerator::~BTPacketGenerator(void)
{
	if( m_pFailureReason != NULL )
	{
		delete m_pFailureReason;
		m_pFailureReason = NULL;
	}

	if( m_pWarningMessage != NULL )
	{
		delete m_pWarningMessage;
		m_pWarningMessage = NULL;
	}
}

//
//
//
void BTPacketGenerator::SetFailureReason(const char *pStr) 
{
	if( m_pFailureReason != NULL )
	{
		delete m_pFailureReason;
		m_pFailureReason = NULL;
	}

	m_pFailureReason = dupstring( pStr );
}

//
//
//
void BTPacketGenerator::SetWarningMessage(const char *pStr)
{
	if( m_pWarningMessage != NULL )
	{
		delete m_pWarningMessage;
		m_pWarningMessage = NULL;
	}

	m_pWarningMessage = dupstring( pStr );
}

//
//
//
void BTPacketGenerator::GetBencodedPacket(ByteBuffer &rBuf, bool bSupportsCompact)
{
	rBuf.ResizeBuffer( 0 );
	rBuf.ResetIndex();
	rBuf.PutCharPtr( HTTP_200 );

	char aBuf[16];

	// IF there was some failure, just generate the failure message
	if( m_pFailureReason != NULL )
	{
		rBuf.PutChar( 'd' );
		rBuf.PutCharPtr( "14:failure reason" );
		rBuf.PutCharPtr( itoa( (int)strlen(m_pFailureReason), aBuf, 10 ) );
		rBuf.PutChar( ':' );
		rBuf.PutCharPtr( m_pFailureReason );
		rBuf.PutChar( 'e' );
	}
	// ELSE IF this is an announce result
	else if( m_bAnnounce )
	{
		rBuf.PutChar( 'd' );

		if( m_pWarningMessage != NULL )
		{
			rBuf.PutCharPtr( "15:warning message" );
			rBuf.PutCharPtr( itoa( (int)strlen(m_pWarningMessage), aBuf, 10 ) );
			rBuf.PutChar( ':' );
			rBuf.PutCharPtr( m_pWarningMessage );
		}

		if( m_nComplete > 0 )
		{
			rBuf.PutCharPtr( "8:completei" );
			rBuf.PutCharPtr( itoa( m_nComplete, aBuf, 10 ) );
			rBuf.PutChar( 'e' );
		}

		if( m_nIncomplete > 0 )
		{
			rBuf.PutCharPtr( "10:incompletei" );
			rBuf.PutCharPtr( itoa( m_nIncomplete, aBuf, 10 ) );
			rBuf.PutChar( 'e' );
		}

		if( m_nInterval > 0 )
		{
			rBuf.PutCharPtr( "8:intervali" );
			rBuf.PutCharPtr( itoa( m_nInterval, aBuf, 10 ) );
			rBuf.PutChar( 'e' );
		}

		if( m_nMinInterval > 0 )
		{
			rBuf.PutCharPtr( "12:min intervali" );
			rBuf.PutCharPtr( itoa( m_nMinInterval, aBuf, 10 ) );
			rBuf.PutChar( 'e' );
		}

		rBuf.PutCharPtr( "5:peers" );
		if( bSupportsCompact )
		{
			rBuf.PutCharPtr( itoa( (int)m_sPeers.size()*6, aBuf, 10 ) );
			rBuf.PutChar( ':' );
			for( set<Peer>::iterator iter = m_sPeers.begin(); iter != m_sPeers.end(); iter++ )
			{
				rBuf.PutDword( iter->GetIP() );
				rBuf.PutUshort( swapbytes( iter->GetPort() ) );
			}
		}
		else
		{
			rBuf.PutChar( 'l' );

			char aIPBuf[38];
			for( set<Peer>::iterator iter = m_sPeers.begin(); iter != m_sPeers.end(); iter++ )
			{
				sprintf( aIPBuf, "d2:ip%u.%u.%u.%u4:porti%uee",
					((iter->GetIP() & 0xff000000)>>24), ((iter->GetIP() & 0x00ff0000)>>16),
					((iter->GetIP() & 0x0000ff00)>>8), (iter->GetIP() & 0x000000ff), iter->GetPort() );
				rBuf.PutCharPtr( aIPBuf );
			}

			rBuf.PutChar( 'e' );
		}
		rBuf.PutChar( 'e' );
	}
	// ELSE this is a scrape result
	else
	{
		rBuf.PutCharPtr( "d5:filesd20:" );
		rBuf.PutBytePtr( m_scrapeHash.ToBytePtr(), 20 );

		rBuf.PutCharPtr( "d8:completei" );
		rBuf.PutCharPtr( itoa( m_nComplete, aBuf, 10 ) );
		rBuf.PutChar( 'e' );

		rBuf.PutCharPtr( "10:downloadedi" );
		rBuf.PutCharPtr( itoa( m_nHaveCompleted, aBuf, 10 ) );
		rBuf.PutChar( 'e' );

		rBuf.PutCharPtr( "10:incompletei" );
		rBuf.PutCharPtr( itoa( m_nIncomplete, aBuf, 10 ) );
		rBuf.PutChar( 'e' );

		rBuf.PutCharPtr( "eee", 3 );
	}
}
