// TorrentCreationManager.cpp : implementation file
//

#include "StdAfx.h"
#include "TorrentCreationManager.h"
#include "BTTorrentGeneratorDll.h"
#include "TorrentCreator.h"

#define WM_TCM_INIT_THREAD_DATA			WM_USER+01
#define WM_TCM_CHECK_DATABASE			WM_USER+02
#define WM_TCM_DISPLAY_TORRENTS			WM_USER+03
#define WM_TCM_RESET_PROGRESS_BAR		WM_USER+04
#define WM_TCM_UPDATE_PROGRESS_BAR		WM_USER+05
#define WM_TCM_UPDATE_PROGRESS_TEXT		WM_USER+06

// CTorrentCreationManager

IMPLEMENT_DYNAMIC(CTorrentCreationManager, CWnd)

BEGIN_MESSAGE_MAP(CTorrentCreationManager, CWnd)
	ON_MESSAGE(WM_TCM_INIT_THREAD_DATA, InitThreadData)
	ON_MESSAGE(WM_TCM_DISPLAY_TORRENTS, DisplayTorrents)
	ON_MESSAGE(WM_TCM_RESET_PROGRESS_BAR, ResetProgressBar)
	ON_MESSAGE(WM_TCM_UPDATE_PROGRESS_BAR, UpdateProgressBar)
	ON_MESSAGE(WM_TCM_UPDATE_PROGRESS_TEXT, UpdateProgressText)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////
//          THREAD FUNCTIONS           //
/////////////////////////////////////////

//
//
//
WPARAM TorrentCreationManagerThreadProcLogMsg(const char *pDataTemplate, ...)
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
UINT TorrentCreationManagerThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd = (HWND)pParam;
	seedRand();

	CCriticalSection criticalSection;
	HANDLE aEvents[2];
	aEvents[0] = CreateEvent( NULL, true, false, NULL ); 
	aEvents[1] = CreateEvent( NULL, true, false, NULL ); 
	DWORD nEventFired;

	// Local variable for generating spoof results
	TorrentCreator torrentCreator;

	// initialize the thread data
	::PostMessage( hwnd, WM_TCM_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)aEvents );

	bool bQuit = false;
	while( !bQuit )
	{
		nEventFired = WaitForMultipleObjects( 2, aEvents, false, INFINITE );

		switch( nEventFired )
		{
		case WAIT_OBJECT_0 + 0:		// quit request
			bQuit = true;
			break;

		case WAIT_OBJECT_0 + 1:		// generate request
			{
				vector<string> *pvTorrentNames = new vector<string>;
				vector<string> *pvTorrentHashes = new vector<string>;

				vector<BTInput *> vTorrents;
				torrentCreator.GetTorrents( vTorrents );

				::PostMessage( hwnd, WM_TCM_UPDATE_PROGRESS_TEXT, TorrentCreationManagerThreadProcLogMsg( "Begining torrent generation" ), NULL );
				::PostMessage( hwnd, WM_TCM_RESET_PROGRESS_BAR, (WPARAM)vTorrents.size(), NULL );

				for( vector<BTInput *>::iterator v_iter = vTorrents.begin(); v_iter != vTorrents.end(); v_iter++ )
				{
					TRACE( "Creating Torrent for \"%s\"\n", (*v_iter)->GetTorrentFileName().c_str() );
					::PostMessage( hwnd, WM_TCM_UPDATE_PROGRESS_TEXT, TorrentCreationManagerThreadProcLogMsg( "Generating \"%s\"", (*v_iter)->GetTorrentFileName().c_str() ), NULL );

					torrentCreator.MakeTorrent( *v_iter );

					torrentCreator.SetPieceValues();

					torrentCreator.WriteHashToDatabase();

					torrentCreator.WriteOutputToFile();
					torrentCreator.WriteTorrentToDatabase();

					torrentCreator.InitiateZeroSeedBoosting();

					pvTorrentNames->push_back( (*v_iter)->GetTorrentFileName() );
					pvTorrentHashes->push_back( (*v_iter)->GetHashString() );

					TRACE( "Torrent complete for \"%s\"\n", (*v_iter)->GetTorrentFileName().c_str() );
					::PostMessage( hwnd, WM_TCM_UPDATE_PROGRESS_TEXT, TorrentCreationManagerThreadProcLogMsg( "Completed generating \"%s\"", (*v_iter)->GetTorrentFileName().c_str() ), NULL );
					::PostMessage( hwnd, WM_TCM_UPDATE_PROGRESS_BAR, NULL, NULL );

					delete (*v_iter);
				}
				::PostMessage( hwnd, WM_TCM_UPDATE_PROGRESS_TEXT, TorrentCreationManagerThreadProcLogMsg( "Torrent generation complete" ), NULL );

				::PostMessage( hwnd, WM_TCM_DISPLAY_TORRENTS, (WPARAM)pvTorrentNames, (LPARAM)pvTorrentHashes );

				{
					CSingleLock( &criticalSection, true );
					ResetEvent( aEvents[1] );
				}
			}
			break;

		default:
			// there was an error of some sort, just leave loop
			break;
		}

	}

	// THREAD ENDING - STARTING CLEAN UP OF DYNAMIC MEMORY

	for( size_t i = 0; i < 2; i++ )
		CloseHandle( aEvents[i] );

	return 0;	// exit the thread
}

/////////////////////////////////////////
//           CLASS FUNCTIONS           //
/////////////////////////////////////////

//
//
//
CTorrentCreationManager::CTorrentCreationManager()
: m_pParent(NULL), m_pThread(NULL), m_pCriticalSection(NULL), m_pingTimer(NULL)
{
}

//
//
//
CTorrentCreationManager::~CTorrentCreationManager()
{
	Stop();
}

//
//
//
LRESULT CTorrentCreationManager::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL || lparam == NULL )
	{
		return -1;
	}
	else
	{
		m_pCriticalSection = (CCriticalSection *)wparam;
		memcpy( m_aEvents, (HANDLE *)lparam, sizeof(m_aEvents) );

		m_pingTimer = SetTimer( WM_TCM_CHECK_DATABASE, 60 * 1000, NULL );

		CSingleLock( m_pCriticalSection, true );
		SetEvent( m_aEvents[1] );

		return 0;
	}
}

//
//
//
LRESULT CTorrentCreationManager::DisplayTorrents(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL || lparam == NULL )
		return -1;

	m_pParent->DisplayTorrents( (vector<string> *)wparam, (vector<string> *)lparam );

	return 0;
}

//
//
//
LRESULT CTorrentCreationManager::ResetProgressBar(WPARAM wparam, LPARAM lparam)
{
	m_pParent->ResetProgressBar( (size_t)wparam );

	return 0;
}

//
//
//
LRESULT CTorrentCreationManager::UpdateProgressBar(WPARAM wparam, LPARAM lparam)
{
	m_pParent->UpdateProgressBar();

	return 0;
}

//
//
//
LRESULT CTorrentCreationManager::UpdateProgressText(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->UpdateProgressText( (const char *)wparam );
	}

	return 0;
}

//
//
//
void CTorrentCreationManager::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_TCM_CHECK_DATABASE:
		{
			if( m_pCriticalSection != NULL )
			{
				CSingleLock( m_pCriticalSection, true );
				SetEvent( m_aEvents[1] );
			}
		}
		break;
	}

	__super::OnTimer( nIDEvent );
}

//
//
//
void CTorrentCreationManager::Init(BTTorrentGeneratorDll *pParent)
{
	m_pParent = pParent;
}

//
//
//
void CTorrentCreationManager::Run(void)
{
	if( !this->CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) )
	{
		ASSERT(false);
		return;
	}

	m_pThread = AfxBeginThread( TorrentCreationManagerThreadProc, (LPVOID)this->GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
}

//
//
//
void CTorrentCreationManager::Stop(void)
{
	// Kill the thread
	if( m_pCriticalSection != NULL )
	{
		SetEvent( m_aEvents[0] );
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
