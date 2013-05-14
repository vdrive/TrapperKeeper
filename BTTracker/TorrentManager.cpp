// TorrentManager.cpp : implementation file
//

#include "StdAfx.h"
#include "BTTrackerDll.h"
#include "TorrentManager.h"
#include "TorrentManagerThreadData.h"
#include "TorrentIndex.h"
#include "TorrentIndexRequest.h"
#include "TorrentListUpdate.h"

// CTorrentManager

IMPLEMENT_DYNAMIC(CTorrentManager, CWnd)

BEGIN_MESSAGE_MAP(CTorrentManager, CWnd)
	ON_MESSAGE(WM_TMANAGER_INIT_THREAD_DATA, InitThreadData)
	ON_MESSAGE(WM_TMANAGER_INDEX_RESULT, TorrentIndexRes)
	ON_MESSAGE(WM_TMANAGER_LIST_UPDATE, UpdateTorrentList)
	ON_MESSAGE(WM_TMANAGER_PEER_RESULT, GetConnectedPeers)
	ON_MESSAGE(WM_TMANAGER_LOG_MSG, LogMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////
//          THREAD FUNCTIONS           //
/////////////////////////////////////////

//
//
//
UINT TorrentManagerThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd = (HWND)pParam;

	seedRand();

	CCriticalSection criticalSection;
	HANDLE aEvents[TORRENT_MANAGER_EVENTS];
	for( int i = 0; i < TORRENT_MANAGER_EVENTS; i++ )
		aEvents[i] = CreateEvent( NULL, true, false, NULL );
	DWORD nEventFired;

	// Thread data struct to pass critical section data
	TorrentManagerThreadData threadData( aEvents );

	//
	TorrentIndex torrentIndex( hwnd );

	// Local variables to hold file operation queues
	vector<TorrentIndexRequest *> *pvSearchRequests = NULL;
	vector<Torrent *> *pvAddedTorrents = NULL;

	// initialize the thread data
	::PostMessage( hwnd, WM_TMANAGER_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	bool bQuit = false;

	while( !bQuit )
	{
		nEventFired = WaitForMultipleObjects( TORRENT_MANAGER_EVENTS, aEvents, false, INFINITE );

		switch( nEventFired )
		{
		case WAIT_OBJECT_0 + 0:		// quit request
			bQuit = true;
			break;

		case WAIT_OBJECT_0 + 1:		// search request
			{
				CSingleLock singleLock( &criticalSection, true );
				
				if( threadData.m_pvSearchRequests != NULL )
				{
					pvSearchRequests = threadData.m_pvSearchRequests;
					threadData.m_pvSearchRequests = NULL;
				}

				ResetEvent( aEvents[1] );
			}

			while( pvSearchRequests->size() > 0  )
			{
				torrentIndex.TorrentQuery( pvSearchRequests->front() );
				pvSearchRequests->erase( pvSearchRequests->begin() );
			}
			delete pvSearchRequests;
			pvSearchRequests = NULL;

			break;

		case WAIT_OBJECT_0 + 2:		// check for stale ips
			{
				CSingleLock singleLock( &criticalSection, true );
				ResetEvent( aEvents[2] );
			}

			torrentIndex.KillStaleIPs();

			break;

		case WAIT_OBJECT_0 + 3:		// add new infohash
			{
				CSingleLock singleLock( &criticalSection, true );
				
				if( threadData.m_pvAddedTorrents != NULL )
				{
					pvAddedTorrents = threadData.m_pvAddedTorrents;
					threadData.m_pvAddedTorrents = NULL;
				}

				ResetEvent( aEvents[3] );
			}

			while( pvAddedTorrents->size() > 0  )
			{
				torrentIndex.AddTorrent( pvAddedTorrents->front() );
				pvAddedTorrents->erase( pvAddedTorrents->begin() );
			}
			delete pvAddedTorrents;
			pvAddedTorrents = NULL;

			break;

		case WAIT_OBJECT_0 + 4:		// get peers
			{
				const Hash20Byte *pHash = NULL;

				{
					CSingleLock singleLock( &criticalSection, true );
					
					if( threadData.m_pGetConnectedPeers != NULL )
					{
						pHash = threadData.m_pGetConnectedPeers;
						threadData.m_pGetConnectedPeers = NULL;
					}

					ResetEvent( aEvents[4] );
				}

				if( pHash != NULL )
				{
					torrentIndex.GetConnectedPeers( pHash );
					pHash = NULL;
				}
			}
			break;

		default:
			// there was an error of some sort, just leave loop
			break;
		}

	}

	// THREAD ENDING - STARTING CLEAN UP OF DYNAMIC MEMORY

	if( pvSearchRequests != NULL )
	{
		for( size_t i = 0; i < pvSearchRequests->size(); i++ )
		{
			delete (*pvSearchRequests)[i];
			(*pvSearchRequests)[i] = NULL;
		}
		delete pvSearchRequests;
		pvSearchRequests = NULL;
	}

	if( pvAddedTorrents != NULL )
	{
		for( size_t i = 0; i < pvAddedTorrents->size(); i++ )
		{
			delete (*pvAddedTorrents)[i];
			(*pvAddedTorrents)[i] = NULL;
		}
		delete pvAddedTorrents;
		pvAddedTorrents = NULL;
	}

	for( int i = 0; i < TORRENT_MANAGER_EVENTS; i++ )
		CloseHandle( aEvents[i] );

	return 0;	// exit the thread
}


/////////////////////////////////////////
//           CLASS FUNCTIONS           //
/////////////////////////////////////////

//
//
//
CTorrentManager::CTorrentManager()
: m_pParent(NULL), m_pThread(NULL), m_pCriticalSection(NULL), m_pThreadData(NULL), m_StaleIPTimer(NULL)
{
}

//
//
//
CTorrentManager::~CTorrentManager()
{
	Stop();
}

//
//
//
LRESULT CTorrentManager::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL || lparam == NULL )
	{
		return -1;
	}
	else
	{
		m_pCriticalSection = (CCriticalSection *)wparam;
		m_pThreadData = (TorrentManagerThreadData *)lparam;

		m_StaleIPTimer = SetTimer( WM_TMANAGER_CHECK_CLIENTS, 15 * 60 * 1000, NULL );

		return 0;
	}
}


//
//
//
void CTorrentManager::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_TMANAGER_CHECK_CLIENTS:
		{
			if( m_pCriticalSection != NULL )
			{
				CSingleLock singleLock( m_pCriticalSection, true );
				SetEvent( m_pThreadData->m_aEvents[2] );
			}
		}
		break;
	}

	__super::OnTimer( nIDEvent );
}

//
//
//
void CTorrentManager::Init(BTTrackerDll *pParent)
{
	m_pParent = pParent;
}

//
//
//
void CTorrentManager::Run(void)
{
	if( !this->CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) )
	{
		ASSERT(false);
		return;
	}

	m_pThread = AfxBeginThread( TorrentManagerThreadProc, (LPVOID)this->GetSafeHwnd(), THREAD_PRIORITY_LOWEST );
}

//
//
//
void CTorrentManager::Stop(void)
{
	// Kill the thread
	if( m_pCriticalSection != NULL )
	{
		SetEvent( m_pThreadData->m_aEvents[0] );
		m_pCriticalSection = NULL;
	}

	// Wait for the thread to die
	if( m_pThread != NULL )
	{
		WaitForSingleObject( m_pThread->m_hThread, 5000 );
		m_pThread = NULL;
	}

	// Destroy message window
	this->DestroyWindow();
}

//
//
//
void CTorrentManager::TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest)
{
	if( pTorrentIndexRequest != NULL )
	{
		if( m_pCriticalSection != NULL )
		{
			CSingleLock singleLock( m_pCriticalSection, true );

			if( m_pThreadData->m_pvSearchRequests == NULL )
			{
				m_pThreadData->m_pvSearchRequests = new vector<TorrentIndexRequest *>;
			}
            
			m_pThreadData->m_pvSearchRequests->push_back( pTorrentIndexRequest );

			SetEvent( m_pThreadData->m_aEvents[1] );
		}
		else
		{
			delete pTorrentIndexRequest;
		}
	}
}

//
//
//
void CTorrentManager::AddTorrent(Torrent *pTorrent)
{
	if( pTorrent != NULL )
	{
		if( m_pCriticalSection != NULL )
		{
			CSingleLock singleLock( m_pCriticalSection, true );

			if( m_pThreadData->m_pvAddedTorrents == NULL )
			{
				m_pThreadData->m_pvAddedTorrents = new vector<Torrent *>;
			}
            
			m_pThreadData->m_pvAddedTorrents->push_back( pTorrent );

			SetEvent( m_pThreadData->m_aEvents[3] );
		}
		else
		{
			delete pTorrent;
		}
	}
}

void CTorrentManager::CheckForStaleIPs(void)
{
	if( m_pCriticalSection != NULL )
	{
		CSingleLock singleLock( m_pCriticalSection, true );
		SetEvent( m_pThreadData->m_aEvents[2] );
	}
}

//
//
//
void CTorrentManager::GetConnectedPeers(const Hash20Byte *pHash)
{
	if( m_pCriticalSection != NULL )
	{
		CSingleLock singleLock( m_pCriticalSection, true );

		if( m_pThreadData->m_pGetConnectedPeers != NULL )
		{
			delete m_pThreadData->m_pGetConnectedPeers;
		}

		m_pThreadData->m_pGetConnectedPeers = pHash;
		
		SetEvent( m_pThreadData->m_aEvents[4] );
	}
}

//
//
//
LRESULT CTorrentManager::TorrentIndexRes(WPARAM wparam, LPARAM lparam)
{
	m_pParent->TorrentIndexRes( (TorrentIndexResult *)wparam );

	return 0;
}

//
//
//
LRESULT CTorrentManager::GetConnectedPeers(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->ConnectedPeers( (const set<Peer> *)wparam );
	}

	return 0;
}

//
//
//
LRESULT CTorrentManager::LogMsg(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->CLogEntry( (const char *)wparam );
		delete (const char *)wparam;
	}

	return 0;
}

//
//
//
LRESULT CTorrentManager::UpdateTorrentList(WPARAM wparam, LPARAM lparam)
{
	m_pParent->UpdateTorrentList( (TorrentListUpdate *)wparam );

	return 0;
}

