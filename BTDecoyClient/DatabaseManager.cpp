// CDatabaseManager.cpp : implementation file
//

#include "stdafx.h"
#include "DatabaseManager.h"
#include "DatabaseManagerThreadData.h"
#include "DatabaseManagerThreadParam.h"
#include "TorrentSearchRequest.h"
#include "TorrentSearchResult.h"
#include "BTDecoyClientDll.h"
#include "Torrent.h"
#include "TinySQL.h"
#include "IP.h"
#include <afxmt.h>	// for CCriticalSection

BEGIN_MESSAGE_MAP(CDatabaseManager, CWnd)
	ON_MESSAGE(WM_DBMANAGER_INIT_THREAD_DATA, InitThreadData)
	ON_MESSAGE(WM_DBMANAGER_RECEIVED_TORRENTS, ReceivedNewTorrents)
	ON_MESSAGE(WM_DBMANAGER_LOG_MSG, LogMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//
//
//
WPARAM DatabaseManagerThreadProcLogMsg(const char *pDataTemplate, ...)
{
	if( pDataTemplate == NULL )
		return NULL;

	char *pLogMsg = new char[256];

	va_list ap;
	va_start( ap, pDataTemplate );
	vsprintf( pLogMsg, pDataTemplate, ap );
	va_end( ap );

	return (WPARAM)pLogMsg;
}

//
//
//
unsigned int DatabaseManagerThreadProc(LPVOID pParam)
{
	if( pParam == NULL )
		return -1;

	// Init message window handle
	HWND hwnd = ((DatabaseManagerThreadParam *)pParam)->m_hwnd;
	bool bComplete = ((DatabaseManagerThreadParam *)pParam)->m_bComplete;
	unsigned int nVer = ((DatabaseManagerThreadParam *)pParam)->m_nVer;
	delete ((DatabaseManagerThreadParam *)pParam);

	IP decoyClientIP;
	{
		char hostName[256];
		ZeroMemory( hostName, sizeof(hostName) );

		if( gethostname( hostName, sizeof(hostName) ) != SOCKET_ERROR )
		{
			unsigned long nServerIPBuf;
			struct hostent *pHost = gethostbyname( hostName );
			if( pHost != NULL )
			{
				memcpy( &nServerIPBuf, pHost->h_addr_list[0], sizeof(nServerIPBuf) );
			}
			decoyClientIP.SetIP( nServerIPBuf );
		}
		::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG, DatabaseManagerThreadProcLogMsg( "Decoy Client IP: %s", decoyClientIP.ToString(false).c_str() ), NULL );
	}

	CCriticalSection criticalSection;
	HANDLE aEvents[DBMANAGER_EVENTS];
	aEvents[0] = CreateEvent( NULL, true, false, NULL ); // exit
	aEvents[1] = CreateEvent( NULL, true, false, NULL ); // update
	unsigned long nEventFired;

	DatabaseManagerThreadData threadData( aEvents );
	unsigned int nLastTorrentID = 0;
	set<unsigned int> sActiveTorrents;

	// initialize the thread data
	::PostMessage( hwnd, WM_DBMANAGER_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	bool bQuit = false;
	while( !bQuit )
	{
		nEventFired = WaitForMultipleObjects( DBMANAGER_EVENTS, aEvents, false, INFINITE );

		switch( nEventFired )
		{
		case WAIT_OBJECT_0 + 0:		// quit request
			bQuit = true;
			break;

		case WAIT_OBJECT_0 + 1:		// Get new torrents from database
			{
				{
					CSingleLock singleLock( &criticalSection, true );
					ResetEvent( aEvents[1] );
				}

				TinySQL db;

				// Try to contact the database
				if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
				{
					::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG,
						DatabaseManagerThreadProcLogMsg( "ERROR: Could not connect to database" ), NULL );
					break;
				}

				// Update the database with the current protected torrents
				char aQuery[1024];
				for( set<unsigned int>::iterator iter = sActiveTorrents.begin(); iter != sActiveTorrents.end(); iter++ )
				{
					sprintf( aQuery, "UPDATE bt_generator_ips SET time=now(),version='%u.%u.%u.%u' where torrent_id=%u AND ip=%u AND port=%u",
						(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff, *iter, decoyClientIP.GetIPForDB(),
						6881 );
					db.Query( aQuery, false );
				}

				map<Hash20Byte, Torrent *> *pmpTorrents = new map<Hash20Byte, Torrent *>;
				unsigned int nNewLastTorrentID = -1;
				bool bFailed = false;

				// Query the database for the new torrents with active protection
				if( !db.Query( "SELECT max(auto_torrent_id) from bt_generator_maininfo", true ) || db.m_nRows != 1 )
				{
					::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG,
						DatabaseManagerThreadProcLogMsg( "ERROR: Could not query bt_generator_maininfo for active projects" ), NULL );

					bFailed = true;
					break;
				}
				else
				{
					nNewLastTorrentID = (unsigned int)atoi( db.m_ppResults[0][0].c_str() );
				}

				for( unsigned int i = nLastTorrentID; i <= nNewLastTorrentID; i++ )
				{
					// Query the database for the new torrents with active protection
					sprintf( aQuery, "SELECT auto_torrent_id,torrentfilename,info_hash,piece_length,total_length,activation_date FROM bt_generator_maininfo WHERE auto_torrent_id = %u AND active='T' AND info_hash != ''", i );
					if( !db.Query( aQuery, true ) )
					{
						::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG,
							DatabaseManagerThreadProcLogMsg( "ERROR: Could not query bt_generator_maininfo for active projects" ), NULL );

						bFailed = true;
						break;
					}

					// IF the torrent with auto_torrent_id = i is not active, continue
					if( db.m_nRows == 0 )
					{
						continue;
					}

					// Insert the active torrents
					Hash20Byte keyHash;
					Torrent *pValueTorrent;

					keyHash.FromHexString( db.m_ppResults[0][2].c_str() );
					pValueTorrent = new Torrent( (unsigned int)atoi( db.m_ppResults[0][0].c_str() ), db.m_ppResults[0][1].c_str(),
						db.m_ppResults[0][2].c_str(), (unsigned int)atoi( db.m_ppResults[0][3].c_str() ),
						(unsigned int)atoi( db.m_ppResults[0][4].c_str() ), db.m_ppResults[0][5].c_str() );

					// Query for the seedless pieces
					sprintf( aQuery, "SELECT piece_num FROM bt_generator_seedless_pieces WHERE torrent_id = %u", i );
					if( !db.Query( aQuery, true ) )
					{
						::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG,
							DatabaseManagerThreadProcLogMsg( "ERROR: Could not query bt_generator_seedless_pieces for seedless pieces" ), NULL );

						bFailed = true;
						break;
					}

					// Add seedless pieces
					for( unsigned int j = 0; j < db.m_nRows; j++ )
					{
						pValueTorrent->AddSeedlessPiece( atol( db.m_ppResults[j][0].c_str() ) );
					}

					// Query for the seeded pieces
					sprintf( aQuery, "SELECT piece_num,time_interval FROM bt_generator_seeded_pieces WHERE torrent_id = %u", i );
					if( !db.Query( aQuery, true ) )
					{
						::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG,
							DatabaseManagerThreadProcLogMsg( "ERROR: Could not query bt_generator_seeded_pieces for seeded pieces" ), NULL );

						bFailed = true;
						break;
					}

					// Add seeded pieces
					for( unsigned int j = 0; j < db.m_nRows; j++ )
					{
						pValueTorrent->AddSeededPiece( atoi( db.m_ppResults[j][0].c_str() ), atoi( db.m_ppResults[j][1].c_str() ) );
					}

					// try to add the torrent to the map of torrents
					if( !( pmpTorrents->insert( make_pair( keyHash, pValueTorrent ) ) ).second )
					{
						delete pValueTorrent;
						continue;
					}

					sActiveTorrents.insert( i );

					TRACE( "DatabaseManager: Finished torrent %u\n", i );

				}	// end for( unsigned int i = nLastTorrentID; i <= nNewLastTorrentID; i++ )

				// check if the queries worked or there was an error
				if( bFailed )
				{
					if( pmpTorrents != NULL )
					{
						for( map<Hash20Byte, Torrent *>::iterator deletionIter = pmpTorrents->begin();
							deletionIter != pmpTorrents->end(); deletionIter++ )
						{
							if( deletionIter->second != NULL )
							{
								delete deletionIter->second;
							}
						}
						delete pmpTorrents;
					}
					break;
				}
				else
				{
					// Update database with current ip
					for( map<Hash20Byte, Torrent *>::iterator iter = pmpTorrents->begin(); iter != pmpTorrents->end(); iter++ )
					{
						if( iter->second->GetActivationTime().GetYear() == 1980 )
						{
							iter->second->SetActivationTime();

							sprintf( aQuery, "UPDATE bt_generator_maininfo set activation_date = now() where auto_torrent_id = %u",
								iter->second->GetTorrentID() );
							db.Query( aQuery, false );
						}

						sprintf( aQuery, "INSERT INTO bt_generator_ips (torrentfilename,ip,ip_str,port,time,torrent_id,complete,version) VALUES ('%s',%u,'%s',%u,now(),%u,'%s','%u.%u.%u.%u')",
							iter->second->GetFilename(), decoyClientIP.GetIPForDB(), decoyClientIP.ToString(false).c_str(), 6881, iter->second->GetTorrentID(),
							( bComplete ? "T" : "F" ), (nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff );

						if( !db.Query( aQuery, false ) )
						{
							sprintf( aQuery, "UPDATE bt_generator_ips SET time=now(),complete='%s',version='%u.%u.%u.%u' where ip=%u AND port=%u AND torrent_id=%u",
								( bComplete ? "T" : "F" ), (nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff,
								decoyClientIP.GetIPForDB(), 6881, iter->second->GetTorrentID() );
							db.Query( aQuery, false );
						}
					}

					nLastTorrentID = nNewLastTorrentID + 1;

					::PostMessage( hwnd, WM_DBMANAGER_RECEIVED_TORRENTS, (WPARAM)pmpTorrents, NULL );
				}

			}	// end case WAIT_OBJECT_0 + 1

			break;

		default:
			// there was an error of some sort, just leave loop
			::PostMessage( hwnd, WM_DBMANAGER_LOG_MSG, DatabaseManagerThreadProcLogMsg( "ERROR: Thread had undefined error" ), NULL );
			break;
		}

	}

	// THREAD ENDING - STARTING CLEAN UP OF DYNAMIC MEMORY

	for( int i = 0; i < DBMANAGER_EVENTS; i++ )
		CloseHandle( aEvents[i] );

	return 0;	// exit the thread
}

//
//
//
CDatabaseManager::CDatabaseManager()
: m_pParent(NULL), m_pCriticalSection(NULL), m_pThreadData(NULL), m_pThread(NULL)
, m_getTorrentsTimer(NULL), m_bComplete(false), m_nVer(-1)
{
}

//
//
//
CDatabaseManager::~CDatabaseManager()
{
	Stop();
}

//
//
//
void CDatabaseManager::Run(void)
{
	// Create message window so that it will accept messages posted to it
	if( this->CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) == false )
	{
		return;
	}

	DatabaseManagerThreadParam *pDatabaseManagerThreadParam = new DatabaseManagerThreadParam;
	pDatabaseManagerThreadParam->m_hwnd = this->GetSafeHwnd();
	pDatabaseManagerThreadParam->m_bComplete = m_bComplete;
	pDatabaseManagerThreadParam->m_nVer = m_nVer;

	m_pThread = AfxBeginThread( DatabaseManagerThreadProc, (LPVOID)pDatabaseManagerThreadParam, THREAD_PRIORITY_LOWEST );

	m_getTorrentsTimer = SetTimer( WM_DBMANAGER_GET_TORRENTS, 10 * 60 * 1000, NULL );
}

//
//
//
void CDatabaseManager::Stop(void)
{
	if( m_pCriticalSection != NULL )
	{
		WSASetEvent( m_pThreadData->m_aEvents[0] );
		m_pCriticalSection = NULL;
	}

	// Wait for the thread to die
	if( m_pThread != NULL )
	{
		WaitForSingleObject( m_pThread->m_hThread, 2000 );
		m_pThread = NULL;
	}

	if( m_getTorrentsTimer != NULL )
	{
		KillTimer( m_getTorrentsTimer );
		m_getTorrentsTimer = NULL;
	}

	// Destroy message window
	this->DestroyWindow();
}

//
//
//
void CDatabaseManager::GetTorrents(void)
{
	if( m_pCriticalSection != NULL )
	{
		CSingleLock singleLock( m_pCriticalSection, true );
		WSASetEvent( m_pThreadData->m_aEvents[1] );
	}			
}

//
//
//
LRESULT CDatabaseManager::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	m_pCriticalSection = (CCriticalSection *)wparam;
	m_pThreadData = (DatabaseManagerThreadData *)lparam;

	GetTorrents();

	return 0;
}

//
//
//
LRESULT CDatabaseManager::ReceivedNewTorrents(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL )
		return -1;

	m_pParent->ReceivedNewTorrents( (map<Hash20Byte, const Torrent *> *)wparam );

	return 0;
}

//
//
//
LRESULT CDatabaseManager::LogMsg(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->LogMsg( (char *)wparam );
		delete (char *)wparam;
	}

	return 0;
}

//
//
//
void CDatabaseManager::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_DBMANAGER_GET_TORRENTS:
		{
			GetTorrents();
		}
		break;
	}

	__super::OnTimer( nIDEvent );
}
