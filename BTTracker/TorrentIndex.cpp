#include "TorrentIndex.h"
#include "TorrentIndexRequest.h"
#include "TorrentIndexResult.h"
#include "TorrentListUpdate.h"
#include "TinySQL.h"

#define OLDEST_STALE_IP_TIME	60

//
// TorrentIndex
//

typedef struct TorrentIndexThreadParamStruct
{
	TorrentIndexThreadData *m_pThreadData;
	CCriticalSection *m_pCriticalSection;
	HWND m_hwnd;
} TorrentIndexThreadParam;

//
//
//
UINT TorrentIndexThreadProc(LPVOID pParam)
{
	// Init message window handle
	if( pParam == NULL || ((TorrentIndexThreadParam *)pParam)->m_pCriticalSection == NULL || 
		((TorrentIndexThreadParam *)pParam)->m_pThreadData == NULL )
		return (UINT)(-1);

	seedRand();

	// Create critical section data
	CCriticalSection *pCriticalSection = ((TorrentIndexThreadParam *)pParam)->m_pCriticalSection;
	TorrentIndexThreadData *pThreadData = ((TorrentIndexThreadParam *)pParam)->m_pThreadData;
	HWND hwnd = ((TorrentIndexThreadParam *)pParam)->m_hwnd;
	delete ((TorrentIndexThreadParam *)pParam);
	pParam = NULL;

	// Temp holders for indexing and searching
	set<Torrent>::iterator torrentIter;
	set<Peer>::iterator peerIter;
	Torrent torrent;
	Peer peer;

	// Database updater holders
	CTime lastUpdate = CTime::GetCurrentTime() - CTimeSpan(0,0,5,0);

	while( !pThreadData->m_bQuit )
	{
		// IF it is a new day OR if no data has ever been collected
		if( ( lastUpdate < CTime::GetCurrentTime() - CTimeSpan(0,0,5,0) ) )
		{
			lastUpdate = CTime::GetCurrentTime();
			TinySQL db;

			// Try to connect to the database
			if( pThreadData->m_bQuit || !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
			{
				::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, (WPARAM)dupstring( "ERROR: Could not connect to database 206.161.141.35...will try again in 5 minutes" ), NULL );
				continue;
			}

			// Try to query the database
			if( pThreadData->m_bQuit || !db.Query( "SELECT info_hash FROM bt_generator_maininfo WHERE active = 'F'", true ) )
			{
				::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, (WPARAM)dupstring( "ERROR: Could not query database..." ), NULL );
				continue;
			}

			// Remove the inactive torrents
			{
				TorrentListUpdate *pTorrentListUpdate;

				CSingleLock singleLock( pCriticalSection, true );

				for( UINT i = 0; i < db.m_nRows; i++ )
				{
					torrent.Clear();

					// INFOHASH
					torrent.m_infoHash.FromHexString( db.m_ppResults[i][0].c_str() );

					// Attempt to find the infohash in the index
					torrentIter = pThreadData->m_sTorrents.find( torrent );
					if( torrentIter != pThreadData->m_sTorrents.end() )
					{
						pThreadData->m_sTorrents.erase( torrent );

						// Generate the update message
						pTorrentListUpdate = new TorrentListUpdate;
						pTorrentListUpdate->SetInfoHash( db.m_ppResults[i][0] );
						pTorrentListUpdate->SetRemove();

						::PostMessage( hwnd, WM_TMANAGER_LIST_UPDATE, (WPARAM)pTorrentListUpdate, NULL );
					}
				}
			}

			// Try to query the database
			if( pThreadData->m_bQuit || !db.Query( "SELECT info_hash,ip,port,time,total_length,bt_generator_maininfo.torrentfilename,base_seeds,seed_inflation_multiplier,base_peers,peer_inflation_multiplier,base_complete,complete_inflation_multiplier,complete FROM bt_generator_ips,bt_generator_maininfo,bt_generator_inflation WHERE bt_generator_ips.torrentfilename = bt_generator_maininfo.torrentfilename AND bt_generator_maininfo.auto_torrent_id = bt_generator_inflation.torrent_id AND bt_generator_maininfo.active = 'T'", true ) )
			{
				::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, (WPARAM)dupstring( "ERROR: Could not query database..." ), NULL );
				continue;
			}

			// Add active torrents
			{
				TorrentListUpdate *pTorrentListUpdate;

				CSingleLock singleLock( pCriticalSection, true );

				for( UINT i = 0; i < db.m_nRows; i++ )
				{
					torrent.Clear();

					// IF the ip is 127.0.0.1, ignore it
					if( (DWORD)atoi( db.m_ppResults[i][1].c_str() ) == 0x7F000001 )
						continue;

					// INFOHASH
					torrent.m_infoHash.FromHexString( db.m_ppResults[i][0].c_str() );

					// Attempt to find the infohash in the index
					torrentIter = pThreadData->m_sTorrents.find( torrent );

					// IF the infohash could not be found
					if( torrentIter == pThreadData->m_sTorrents.end() )
					{
						// SIZE
						torrent.m_nLength = atoi( db.m_ppResults[i][4].c_str() );
						torrent.m_nSeedBase = atoi( db.m_ppResults[i][6].c_str() );
						torrent.m_nSeedMultiplier = (int)atof( db.m_ppResults[i][7].c_str() );
						torrent.m_nPeerBase = atoi( db.m_ppResults[i][8].c_str() );
						torrent.m_nPeerMultiplier = (int)atof( db.m_ppResults[i][9].c_str() );
						torrent.m_nCompletedBase = atoi( db.m_ppResults[i][10].c_str() );
						torrent.m_nCompletedMultiplier = (int)atof( db.m_ppResults[i][11].c_str() );

						torrentIter = pThreadData->m_sTorrents.insert( torrent ).first;
					}
					else
					{
						torrentIter->m_nSeedBase = atoi( db.m_ppResults[i][6].c_str() );
						torrentIter->m_nSeedMultiplier = (int)atof( db.m_ppResults[i][7].c_str() );
						torrentIter->m_nPeerBase = atoi( db.m_ppResults[i][8].c_str() );
						torrentIter->m_nPeerMultiplier = (int)atof( db.m_ppResults[i][9].c_str() );
						torrentIter->m_nCompletedBase = atoi( db.m_ppResults[i][10].c_str() );
						torrentIter->m_nCompletedMultiplier = (int)atof( db.m_ppResults[i][11].c_str() );
					}

					// UPDATE LAST SEEN from timestamp
					peer.UpdateLastSeen( db.m_ppResults[i][3].c_str() );

					if( ( peer.SecondsSinceLastSeen() / 60 ) < OLDEST_STALE_IP_TIME )
					{
						// IP
						peer.SetIP( (DWORD)atoi( db.m_ppResults[i][1].c_str() ) );

						// PORT
						peer.SetPort( (USHORT)atoi( db.m_ppResults[i][2].c_str() ) );

						// COMPLETE
						if( db.m_ppResults[i][12].compare( "T" ) == 0 )
							peer.SetComplete();
						else
							peer.SetIncomplete();

						// MEDIADEFENDER
						peer.SetMD();

						peerIter = torrentIter->m_psPeers->find( peer );

						if( peerIter == torrentIter->m_psPeers->end() )
						{
							// insert a new version of the peer
							torrentIter->m_psPeers->insert( peer );
						}
						else
						{
							peerIter->UpdateLastSeen( db.m_ppResults[i][3].c_str() );
						}

						torrentIter->m_psPeers->insert( peer );
					}

					// Generate the update message
					pTorrentListUpdate = new TorrentListUpdate;
					pTorrentListUpdate->SetInfoHash( db.m_ppResults[i][0] );
					pTorrentListUpdate->SetTorrentName( db.m_ppResults[i][5] );

					int nSeedsReal = 0;
					int nPeersReal = 0;
					for( peerIter = torrentIter->m_psPeers->begin(); peerIter != torrentIter->m_psPeers->end(); peerIter++ )
					{
						if( peerIter->IsComplete() )
						{
							nSeedsReal++;
						}
						else
						{
							nPeersReal++;
						}
					}

					pTorrentListUpdate->SetSeedsReal( nSeedsReal );
					pTorrentListUpdate->SetPeersReal( nPeersReal );
					pTorrentListUpdate->SetCompletedReal( torrentIter->m_nHaveCompleted );

					pTorrentListUpdate->SetSeedsInflated( torrentIter->m_nSeedBase + 
						( torrentIter->m_nSeedMultiplier * nSeedsReal ) ) ;
					pTorrentListUpdate->SetPeersInflated( torrentIter->m_nPeerBase + 
						( torrentIter->m_nPeerMultiplier * nPeersReal ) );
					pTorrentListUpdate->SetCompletedInflated( torrentIter->m_nCompletedBase + 
						( torrentIter->m_nCompletedMultiplier * torrentIter->m_nHaveCompleted ) );

					::PostMessage( hwnd, WM_TMANAGER_LIST_UPDATE, (WPARAM)pTorrentListUpdate, NULL );
				}
			}

			::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, (WPARAM)dupstring( "Updated database..." ), NULL );
		}
		else
		{
			Sleep(1000);
		}
	}


	return 0;
}

//
//
//
TorrentIndex::TorrentIndex(HWND parentHwnd)
: m_managerHwnd(parentHwnd), m_pThread(NULL), m_criticalSection(), m_threadData()
{
	m_threadData.m_bQuit = false;
	m_threadData.m_sTorrents.clear();

	TorrentIndexThreadParam *pTorrentIndexThreadParam = new TorrentIndexThreadParam;
	pTorrentIndexThreadParam->m_hwnd = parentHwnd;
	pTorrentIndexThreadParam->m_pCriticalSection = &m_criticalSection;
	pTorrentIndexThreadParam->m_pThreadData = &m_threadData;

	m_pThread = AfxBeginThread( TorrentIndexThreadProc, (LPVOID)pTorrentIndexThreadParam, THREAD_PRIORITY_LOWEST );
}

//
//
//
TorrentIndex::~TorrentIndex(void)
{
	m_threadData.m_bQuit = true;

	// Wait for the thread to die
	if( m_pThread != NULL )
	{
		WaitForSingleObject( m_pThread->m_hThread, 3000 );
		m_pThread = NULL;
	}
}

//
//
//
void TorrentIndex::TorrentQuery(TorrentIndexRequest *pTorrentIndexRequest)
{
	// IF there is a torrent search request
	if( pTorrentIndexRequest == NULL )
		return;

	// Init the searching torrent
	static set<Torrent>::iterator torrentIter;
	static Torrent torrent;
	torrent.m_infoHash = pTorrentIndexRequest->m_pBTPacket->GetInfoHash();
	if( torrent.m_infoHash.IsMajorityZero() || pTorrentIndexRequest->m_client.GetIP() == 0x7F000001 ||
		pTorrentIndexRequest->m_client.GetIP() == 0x0100007F )
	{
		return;
	}

	// Init the result
	TorrentIndexResult *pTorrentIndexResult = new TorrentIndexResult( *pTorrentIndexRequest );

	TorrentListUpdate *pTorrentListUpdate = NULL;
	int nRealSeeds = 0;
	int nRealPeers = 0;
	int nRealCompleted = 0;
	switch( pTorrentIndexRequest->m_pBTPacket->GetPacketType() )
	{
	case BTPacketParser::TYPE_GET_ANNOUNCE:
		{
			switch( pTorrentIndexRequest->m_pBTPacket->GetEvent() )
			{
			case BTPacketParser::STARTED:
			case BTPacketParser::COMPLETED:
			case BTPacketParser::EMPTY:
				{
					TRACE( "BTPacketParser::TYPE_GET_ANNOUNCE:STARTED/COMPLETED/EMPTY\n" );

					CSingleLock singleLock( &m_criticalSection, true );

					torrentIter = m_threadData.m_sTorrents.find( torrent );

					// IF the torrent could not be found
					if( torrentIter == m_threadData.m_sTorrents.end() )
					{
						TRACE( "BTPacketParser: Could not find torrent\n" );

						pTorrentIndexResult->m_bFoundTorrent = false;
						pTorrentIndexResult->m_sPeers.clear();
					}
					// ELSE the torrent was found
					else
					{
						TRACE( "BTPacketParser: Found torrent - updating peer %s\n",
							pTorrentIndexRequest->m_peer.ToString(false).c_str() );

						// Since clients will never actually complete downloading a torrent, they will never
						//  send a message that they are complete. We will determine completion as being over
						//  80% complete.
						{
							// IF the client has less than 20% of the file left to download, mark as complete
							if( (torrentIter->m_nLength / 5) >= pTorrentIndexRequest->m_pBTPacket->GetLeft() )
							{
								pTorrentIndexRequest->m_peer.SetComplete();
							}
							else
							{
								pTorrentIndexRequest->m_peer.SetIncomplete();
							}

							// Try to find an entry to the peer that has sent the announce
							set<Peer>::iterator peerIter = torrentIter->m_psPeers->find( pTorrentIndexRequest->m_peer );

							// IF the peer already exists AND at its last report it was _not_ "complete" AND
							//  it is now "complete"
							if( peerIter != torrentIter->m_psPeers->end() && !peerIter->IsComplete() &&
								pTorrentIndexRequest->m_peer.IsComplete() )
							{
								// Increment the "peers that have completed downloading" counter
								torrentIter->m_nHaveCompleted += randInt( 1, 10 );
								// Set the client as complete
								peerIter->SetComplete();

								TRACE( "Incrementing \"Have Completed\" for torrent\n" );
							}

							// IF the peer already exists
							if( peerIter != torrentIter->m_psPeers->end() )
							{
								// Update the last seen timer
								peerIter->UpdateLastSeen();

								if( pTorrentIndexRequest->m_peer.IsComplete() )
									peerIter->SetComplete();
								else
									peerIter->SetIncomplete();
							}
							else
							{
								// Insert the new peer
								torrentIter->m_psPeers->insert( pTorrentIndexRequest->m_peer );
							}
						}

						// Count all the complete and incomplete peers
						for( set<Peer>::iterator peerIter = torrentIter->m_psPeers->begin(); peerIter != torrentIter->m_psPeers->end(); peerIter++ )
						{
							if( peerIter->IsComplete() )
							{
								(pTorrentIndexResult->m_nComplete)++;
								nRealSeeds++;
							}
							else
							{
								(pTorrentIndexResult->m_nIncomplete)++;
								nRealPeers++;
							}
						}

						pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nHaveCompleted;
						nRealCompleted = torrentIter->m_nHaveCompleted;

						pTorrentIndexResult->m_nComplete = torrentIter->m_nSeedBase + 
							( torrentIter->m_nSeedMultiplier * pTorrentIndexResult->m_nComplete );

						pTorrentIndexResult->m_nIncomplete = torrentIter->m_nPeerBase + 
							( torrentIter->m_nPeerMultiplier * pTorrentIndexResult->m_nIncomplete );

						pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nCompletedBase + 
							( torrentIter->m_nCompletedMultiplier * pTorrentIndexResult->m_nHaveCompleted );

						// Generate the result to send back to the client
						pTorrentIndexResult->m_bFoundTorrent = true;

						// Get all the peers
						pTorrentIndexResult->m_sPeers = *(torrentIter->m_psPeers);

						{
							// Try to find an entry to the peer that has sent the announce
							set<Peer>::iterator peerIter = torrentIter->m_psPeers->find( pTorrentIndexRequest->m_peer );

							if( peerIter->GetReportedMDIP().GetIP() == 0 )
							{
								vector<Peer> vMDPeers;

								// Generate list of MD peers
								for( set<Peer>::iterator MDIter = torrentIter->m_psPeers->begin();
									MDIter != torrentIter->m_psPeers->end(); MDIter++ )
								{
									if( MDIter->IsMD() )
										vMDPeers.push_back( *MDIter );
								}
								if( !vMDPeers.empty() )
									peerIter->SetReportedMDIP( vMDPeers[ randInt( 0, (int)vMDPeers.size()-1 ) ] );
							}

							pTorrentIndexResult->m_reportedMDIP = peerIter->GetReportedMDIP();
						}
					}
				}

				break;

			case BTPacketParser::STOPPED:
				{
					TRACE( "BTPacketParser::TYPE_GET_ANNOUNCE:STOPPED\n" );

					CSingleLock singleLock( &m_criticalSection, true );

					torrentIter = m_threadData.m_sTorrents.find( torrent );

					// IF the torrent could not be found
					if( torrentIter == m_threadData.m_sTorrents.end() )
					{
						TRACE( "BTPacketParser: Could not find torrent\n" );

						pTorrentIndexResult->m_bFoundTorrent = false;
						pTorrentIndexResult->m_sPeers.clear();

						delete pTorrentListUpdate;
						pTorrentListUpdate = NULL;
					}
					// ELSE the torrent was found
					else
					{
						TRACE( "BTPacketParser: Found torrent - removing peer %s\n",
							pTorrentIndexRequest->m_peer.ToString(false).c_str() );

						torrentIter->m_psPeers->erase( pTorrentIndexRequest->m_peer );

						for( set<Peer>::iterator peerIter = torrentIter->m_psPeers->begin(); peerIter != torrentIter->m_psPeers->end(); peerIter++ )
						{
							if( peerIter->IsComplete() )
							{
								(pTorrentIndexResult->m_nComplete)++;
								nRealSeeds++;
							}
							else
							{
								(pTorrentIndexResult->m_nIncomplete)++;
								nRealPeers++;
							}
						}

						pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nHaveCompleted;
						nRealCompleted = torrentIter->m_nHaveCompleted;

						pTorrentIndexResult->m_nComplete = torrentIter->m_nSeedBase + 
							( torrentIter->m_nSeedMultiplier * pTorrentIndexResult->m_nComplete );

						pTorrentIndexResult->m_nIncomplete = torrentIter->m_nPeerBase + 
							( torrentIter->m_nPeerMultiplier * pTorrentIndexResult->m_nIncomplete );

						pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nCompletedBase + 
							( torrentIter->m_nCompletedMultiplier * pTorrentIndexResult->m_nHaveCompleted );

						pTorrentIndexResult->m_bFoundTorrent = true;
						pTorrentIndexResult->m_sPeers = *(torrentIter->m_psPeers);
					}
				}
				break;
			}
		}
		break;

	case BTPacketParser::TYPE_GET_SCRAPE:
		{
			TRACE( "BTPacketParser::TYPE_GET_SCRAPE\n" );

			CSingleLock singleLock( &m_criticalSection, true );

			torrentIter = m_threadData.m_sTorrents.find( torrent );

			// IF the torrent could not be found
			if( torrentIter == m_threadData.m_sTorrents.end() )
			{
				TRACE( "BTPacketParser: Could not find torrent\n" );

				pTorrentIndexResult->m_bFoundTorrent = false;
				pTorrentIndexResult->m_sPeers.clear();
			}
			// ELSE the torrent was found
			else
			{
				for( set<Peer>::iterator peerIter = torrentIter->m_psPeers->begin(); peerIter != torrentIter->m_psPeers->end(); peerIter++ )
				{
					if( peerIter->IsComplete() )
					{
						(pTorrentIndexResult->m_nComplete)++;
						nRealSeeds++;
					}
					else
					{
						(pTorrentIndexResult->m_nIncomplete)++;
						nRealPeers++;
					}
				}

				pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nHaveCompleted;
				nRealCompleted = torrentIter->m_nHaveCompleted;

				pTorrentIndexResult->m_nComplete = torrentIter->m_nSeedBase + 
					( torrentIter->m_nSeedMultiplier * pTorrentIndexResult->m_nComplete );

				pTorrentIndexResult->m_nIncomplete = torrentIter->m_nPeerBase + 
					( torrentIter->m_nPeerMultiplier * pTorrentIndexResult->m_nIncomplete );

				pTorrentIndexResult->m_nHaveCompleted = torrentIter->m_nCompletedBase + 
					( torrentIter->m_nCompletedMultiplier * pTorrentIndexResult->m_nHaveCompleted );

				pTorrentIndexResult->m_infohash = pTorrentIndexRequest->m_pBTPacket->GetInfoHash();
				pTorrentIndexResult->m_bFoundTorrent = true;
				pTorrentIndexResult->m_sPeers.clear();
			}
		}
		break;
	}

	if( pTorrentIndexResult->m_bFoundTorrent )
	{
		pTorrentListUpdate = new TorrentListUpdate;
		pTorrentListUpdate->SetInfoHash( pTorrentIndexRequest->m_pBTPacket->GetInfoHash() );

		pTorrentListUpdate->SetSeedsReal( nRealSeeds );
		pTorrentListUpdate->SetSeedsInflated( pTorrentIndexResult->m_nComplete );

		pTorrentListUpdate->SetPeersReal( nRealPeers );
		pTorrentListUpdate->SetPeersInflated( pTorrentIndexResult->m_nIncomplete );

		pTorrentListUpdate->SetCompletedReal( nRealCompleted );
		pTorrentListUpdate->SetCompletedInflated( pTorrentIndexResult->m_nHaveCompleted );

		::PostMessage( m_managerHwnd, WM_TMANAGER_LIST_UPDATE, (WPARAM)pTorrentListUpdate, NULL );
		pTorrentListUpdate = NULL;
	}

	::PostMessage( m_managerHwnd, WM_TMANAGER_INDEX_RESULT, (WPARAM)pTorrentIndexResult, NULL );
	pTorrentIndexResult = NULL;

	delete pTorrentIndexRequest;
	pTorrentIndexRequest = NULL;
}

//
//
//
void TorrentIndex::AddTorrent(Torrent *pTorrent)
{
	// IF there is a torrent search request
	if( pTorrent != NULL )
	{
		set<Torrent>::iterator torrentIter;

		{
			CSingleLock singleLock( &m_criticalSection, true );

			torrentIter = m_threadData.m_sTorrents.find( *pTorrent );

			if( torrentIter == m_threadData.m_sTorrents.end() )
			{
				m_threadData.m_sTorrents.insert( *pTorrent );

				// Update the tree
				TorrentListUpdate *pTorrentListUpdate = new TorrentListUpdate;
				pTorrentListUpdate->SetInfoHash( pTorrent->m_infoHash );
				::PostMessage( m_managerHwnd, WM_TMANAGER_LIST_UPDATE, (WPARAM)pTorrentListUpdate, NULL );
			}
		}

		delete pTorrent;
		pTorrent = NULL;
	}
}

//
//
//
void TorrentIndex::RemoveTorrent(const Hash20Byte &rInfoHash)
{
	static Torrent torrent;
	torrent.m_infoHash = rInfoHash;

	CSingleLock singleLock( &m_criticalSection, true );

	m_threadData.m_sTorrents.erase( torrent );
}

//
//
//
void TorrentIndex::KillStaleIPs(void)
{
	CSingleLock singleLock( &m_criticalSection, true );

	int nRemovedTotal = 0;
	int nRemovedFromTorrent = 0;
	int nRealSeeds = 0;
	int nInflatedSeeds = 0;
	int nRealPeers = 0;
	int nInflatedPeers = 0;
	TorrentListUpdate *pTorrentListUpdate = NULL;

	for( set<Torrent>::iterator torrentIter = m_threadData.m_sTorrents.begin();
		torrentIter != m_threadData.m_sTorrents.end(); torrentIter++ )
	{
		nRealSeeds = 0;
		nInflatedSeeds = 0;
		nRealPeers = 0;
		nInflatedPeers = 0;

		for( set<Peer>::iterator peerIter = torrentIter->m_psPeers->begin();
			peerIter != torrentIter->m_psPeers->end(); peerIter++ )
		{
			if( (peerIter->SecondsSinceLastSeen() / 60) > OLDEST_STALE_IP_TIME )
			{
				torrentIter->m_psPeers->erase( peerIter );
				peerIter = torrentIter->m_psPeers->begin();

				nRemovedTotal++;
				nRemovedFromTorrent++;
			}
			else
			{
				if( peerIter->IsMD() )
					nRealSeeds++;

				if( !peerIter->IsMD() && peerIter->IsComplete() )
					nInflatedSeeds++;

				if( !peerIter->IsMD() && !peerIter->IsComplete() )
					nRealPeers++;
			}
		}

		if( nRemovedFromTorrent > 0 )
		{
			// Generate the update message
			pTorrentListUpdate = new TorrentListUpdate;
			pTorrentListUpdate->SetInfoHash( torrentIter->m_infoHash );
			pTorrentListUpdate->SetCompletedInflated( torrentIter->m_nHaveCompleted );

			pTorrentListUpdate->SetSeedsReal( nRealSeeds );
			pTorrentListUpdate->SetSeedsInflated( nInflatedSeeds );

			pTorrentListUpdate->SetPeersReal( nRealPeers );
			pTorrentListUpdate->SetPeersInflated( 0 );

			::PostMessage( m_managerHwnd, WM_TMANAGER_LIST_UPDATE, (WPARAM)pTorrentListUpdate, NULL );

			nRemovedFromTorrent = 0;
		}

	}

	if( nRemovedTotal > 0 )
	{
		char *pMsg = new char[256];
		sprintf( pMsg, "Removed %d stale IPs", nRemovedTotal );

		::PostMessage( m_managerHwnd, WM_TMANAGER_LOG_MSG, (WPARAM)pMsg, NULL );
	}
}

//
//
//
void TorrentIndex::GetConnectedPeers(const Hash20Byte *pHash)
{
	if( pHash == NULL )
		return;

	{
		CSingleLock singleLock( &m_criticalSection, true );

		set<Peer> *pPeers = new set<Peer>;

		Torrent torrent;
		torrent.m_infoHash = *pHash;
		set<Torrent>::iterator torrentIter = m_threadData.m_sTorrents.find( torrent );

		// IF the torrent could not be found
		if( torrentIter == m_threadData.m_sTorrents.end() )
		{
			TRACE( "GetConnectedPeers: Could not find torrent\n" );
		}
		else
		{
			TRACE( "GetConnectedPeers: Found torrent, copying peers\n" );
			(*pPeers) = *(torrentIter->m_psPeers);

			::PostMessage( m_managerHwnd, WM_TMANAGER_PEER_RESULT, (WPARAM)pPeers, NULL );
		}
	}

	delete pHash;
}

