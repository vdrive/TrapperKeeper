// TorrentManager.cpp : implementation file
//

#include "StdAfx.h"
#include "BTDecoyClientDll.h"
#include "TorrentManager.h"
#include "TorrentManagerThreadData.h"
#include "TorrentSearchRequest.h"
#include "TorrentSearchResult.h"
#include <afxmt.h>	// for CCriticalSection
#include <map>
using namespace std;

BEGIN_MESSAGE_MAP(CTorrentManager, CWnd)
	ON_MESSAGE(WM_TMANAGER_INIT_THREAD_DATA, InitThreadData)
	ON_MESSAGE(WM_TMANAGER_TORRENT_SEARCH, TorrentSearch)
	ON_MESSAGE(WM_TMANAGER_LOG_MSG, LogMsg)
END_MESSAGE_MAP()

//
//
//
WPARAM TorrentManagerThreadProcLogMsg(const char *pDataTemplate, ...)
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
unsigned int TorrentManagerThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd = (HWND)pParam;
	map<Hash20Byte, const Torrent *> mpTorrents;
	map<Hash20Byte, const Torrent *>::const_iterator searchIter;

	CCriticalSection criticalSection;
	HANDLE aEvents[TMANAGER_EVENTS];
	aEvents[0] = CreateEvent( NULL, true, false, NULL );		// exit
	aEvents[1] = CreateEvent( NULL, true, false, NULL );		// store new set of torrents
	aEvents[2] = CreateEvent( NULL, true, false, NULL );		// search set for specific infohash
	unsigned long nEventFired;

	TorrentManagerThreadData threadData( aEvents );

	// initialize the thread data
	::PostMessage( hwnd, WM_TMANAGER_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	bool bQuit = false;
	while( !bQuit )
	{
		nEventFired = WaitForMultipleObjects( TMANAGER_EVENTS, aEvents, false, INFINITE );

		switch( nEventFired )
		{
		case WAIT_OBJECT_0 + 0:		// quit request
			TRACE( "Torrent Manager quit event\n" );
			bQuit = true;
			break;

		case WAIT_OBJECT_0 + 1:		// Update the searching torrents
			{
				{
					CSingleLock singleLock( &criticalSection, true );

					if( threadData.m_pmpTorrents != NULL )
					{
						mpTorrents.insert( threadData.m_pmpTorrents->begin(), threadData.m_pmpTorrents->end() );						delete threadData.m_pmpTorrents;
						threadData.m_pmpTorrents = NULL;

						//for( map<Hash20Byte, const Torrent *>::iterator iter = mpTorrents.begin(); iter != mpTorrents.end(); iter++ )
						//{
						//	char aBuf[41];
						//	TRACE( "Torrent: %s - pAddr: %u\n", (Hash20Byte)(iter->first).ToPrintableCharPtr( aBuf ),
						//		iter->second );
						//}
					}

					ResetEvent( aEvents[nEventFired] );
				}

				::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, TorrentManagerThreadProcLogMsg( "Updated local database with %u active torrents", mpTorrents.size() ), NULL );
			}

			break;

		case WAIT_OBJECT_0 + 2:		// Search for matching infohash
			{
				queue<TorrentSearchRequest *> *pqSearches;
				{
					CSingleLock singleLock( &criticalSection, true );

					pqSearches = threadData.m_pqSearches;
					threadData.m_pqSearches = NULL;

					ResetEvent( aEvents[nEventFired] );
				}

				if( pqSearches != NULL )
				{
					while( !pqSearches->empty() )
					{
						if( pqSearches->front() != NULL )
						{
							searchIter = mpTorrents.find( pqSearches->front()->GetHash() );

							// IF a torrent matches on the infohash
							if( searchIter != mpTorrents.end() && searchIter->second != NULL )
							{
								::PostMessage( hwnd, WM_TMANAGER_TORRENT_SEARCH,
									(WPARAM)new TorrentSearchResult( pqSearches->front(), searchIter->second ), NULL );
							}
							// ELSE no torrents match the infohash
							else
							{
								::PostMessage( hwnd, WM_TMANAGER_TORRENT_SEARCH,
									(WPARAM)new TorrentSearchResult( pqSearches->front() ), NULL );
							}

                            delete pqSearches->front();
						}
						pqSearches->pop();
					}
					delete pqSearches;
				}
			}

			break;

		default:
			// there was an error of some sort, just leave loop
			::PostMessage( hwnd, WM_TMANAGER_LOG_MSG, TorrentManagerThreadProcLogMsg( "ERROR: Thread had undefined error" ), NULL );
			break;
		}

	}

	// THREAD ENDING - STARTING CLEAN UP OF DYNAMIC MEMORY

	for( int i = 0; i < TMANAGER_EVENTS; i++ )
		CloseHandle( aEvents[i] );

	while( mpTorrents.size() > 0 )
	{
		delete mpTorrents.begin()->second;
		mpTorrents.erase( mpTorrents.begin() );
	}

	TRACE( "DatabaseManagerThreadProc ending\n" );

	return 0;	// exit the thread
}

//
//
//
CTorrentManager::CTorrentManager()
: m_pParent(NULL), m_pCriticalSection(NULL), m_pThreadData(NULL), m_pThread(NULL)
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
void CTorrentManager::Run(void)
{
	TRACE( "void CTorrentManager::Run(void)\n" );

	// Create message window so that it will accept messages posted to it
	if( this->CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) == false )
	{
		return;
	}

	m_pThread = AfxBeginThread( TorrentManagerThreadProc, (LPVOID)this->GetSafeHwnd(), THREAD_PRIORITY_LOWEST );
}

//
//
//
void CTorrentManager::Stop(void)
{
	TRACE( "void CTorrentManager::Stop(void)\n" );

	// Kill the thread
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

	// Destroy message window
	this->DestroyWindow();
}

//
//
//
void CTorrentManager::ReceivedNewTorrents(map<Hash20Byte, const Torrent *> *pmpTorrents)
{
	if( pmpTorrents == NULL )
		return;

	if( m_pCriticalSection == NULL )
	{
		while( pmpTorrents->size() > 0 )
		{
			delete pmpTorrents->begin()->second;
			pmpTorrents->erase( pmpTorrents->begin() );
		}
		delete pmpTorrents;
		return;
	}

	CSingleLock singleLock( m_pCriticalSection, true );

	if( m_pThreadData->m_pmpTorrents != NULL )
		delete m_pThreadData->m_pmpTorrents;

	m_pThreadData->m_pmpTorrents = pmpTorrents;

	WSASetEvent( m_pThreadData->m_aEvents[1] );
}

//
//
//
void CTorrentManager::TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest)
{
	if( pTorrentSearchRequest == NULL )
		return;

	if( m_pCriticalSection == NULL )
	{
		delete pTorrentSearchRequest;
		return;
	}

	CSingleLock singleLock( m_pCriticalSection, true );

	if( m_pThreadData->m_pqSearches == NULL )
		m_pThreadData->m_pqSearches = new queue<TorrentSearchRequest *>;

	m_pThreadData->m_pqSearches->push( pTorrentSearchRequest );

	WSASetEvent( m_pThreadData->m_aEvents[2] );
}

//
//
//
LRESULT CTorrentManager::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	m_pCriticalSection = (CCriticalSection *)wparam;
	m_pThreadData = (TorrentManagerThreadData *)lparam;

	return 0;
}

//
//
//
LRESULT CTorrentManager::TorrentSearch(WPARAM wparam, LPARAM lparam)
{
	m_pParent->TorrentSearch( (TorrentSearchResult *)wparam );

	return 0;
}

//
//
//
LRESULT CTorrentManager::LogMsg(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->LogMsg( (char *)wparam );
		delete (char *)wparam;
	}

	return 0;
}
