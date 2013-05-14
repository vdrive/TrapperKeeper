// SearchManager.cpp

#include "stdafx.h"
#include "SearchManager.h"
#include "ConnectionManager.h"

#include "SoulSeekServerDll.h"

#include "SearchRequest.h"
#include "WatchedProjects.h"

#include <afxmt.h>	// for CCriticalSection

#include "ConnectionSockets.h"

#include <vector>
using namespace std;

//
//
//
UINT SearchManagerThreadProc(LPVOID pParam)
{
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	SearchManagerThreadData thread_data;

	::PostMessage(hwnd,WM_INIT_THREAD_DATA, (WPARAM)&thread_data, (LPARAM)&critical_section );

	// Wait for events
	DWORD num_events=SEARCH_MANAGER_WAIT_EVENTS;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired

	WatchedProjects* protectedProjects;
	protectedProjects = new WatchedProjects();
	protectedProjects->GetProjects();
	unsigned int tempstats[7];
	unsigned int stats[7];
	stats[0] = protectedProjects->GetNumberOfWatchedProjects();
	stats[1] = 0;  // searches per minute
	stats[2] = 0;  // searches per hour
	stats[3] = 0;  // seaches per day
	stats[4] = 0;  // results per minute
	stats[5] = 0;  // results per hour
	stats[6] = 0;  // results per day
	::PostMessage(hwnd,WM_SM_START_TIMER, (WPARAM)NULL, (LPARAM)NULL);
	::PostMessage(hwnd,WM_SM_UPDATE_GUI, (WPARAM)stats, (LPARAM)NULL);

	while(1)
	{
		event= WaitForMultipleObjects(num_events,&thread_data.m_reserved_events[0],wait_all,timeout);
		
		if(event==WAIT_FAILED)
		{
			::MessageBox(NULL,"WaitForMultipleObjects() Failed","Error",MB_OK);
			break;
		}

		// Check to see if it the triggered event is any of the 4 reserved events

		// Event 0 - Check to see if this is the kill thread event (reserved event 0)
		if(event== WAIT_OBJECT_0 + 0)
		{
			delete protectedProjects;
			break;
		}

		// Event 1 - Process Search Results to see if we are watching the project
		//           If we are watching the project, return SearchRequest item with search results to send back
		if(event==WAIT_OBJECT_0 + 1)
		{
            queue<SearchRequest *> *pqSearches = NULL;

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				ResetEvent(thread_data.m_reserved_events[event]);	// reset event

				pqSearches = thread_data.p_sr;
				thread_data.p_sr = NULL;
				
				singleLock.Unlock();
			}

			char* searchText;
			char *token;
			if( pqSearches != NULL )
			{
                while( !pqSearches->empty() )
				{
					//TRACE( "Searches size: %u\n", pqSearches->size() );

					stats[1] += 1;
					stats[2] += 1;
					stats[3] += 1;

					SearchRequest *pSearch;
					pSearch = pqSearches->front();
					pqSearches->pop();

					searchText = pSearch->GetSeachText();
					// Tokenize search text
					char seps[]   = "\t\n,()&!@#$%^&*;'\"\\/ :|[]{}.<>?`~_+=";
					token = strtok( searchText, seps );

					vector<string> searchTokens;
					
					while( token != NULL )
					{
						searchTokens.push_back( string( token ) );
						token = strtok( NULL, seps ); //Get next token
					}

					// Compare search text to media we are watching
					vector<string>* filenames;
					filenames = protectedProjects->IsWatched(searchTokens);

					// If match
					//   Set FileName in pSearch Object
					//   Send request back to client saying we have it through connection manager (PostMessage)
					
					//if(filenames != NULL)
					//{
						stats[4] += 1;
						stats[5] += 1;
						stats[6] += 1;
						//pSearch->SetFileName(filenames);
						::PostMessage(hwnd,WM_SM_SEND_RESULTS, (WPARAM)pSearch, (LPARAM)NULL );
					//}
					//else
					//{
					//	delete pSearch;
					//	pSearch = NULL;
					//}
				}
				delete pqSearches;
			}
			ResetEvent(thread_data.m_reserved_events[event]);
		}
		if(event== WAIT_OBJECT_0 + 2)
		{
			for(int i = 0; i < 7; i++)
			{
				tempstats[i] = stats[i];
			}
			::PostMessage(hwnd,WM_SM_UPDATE_GUI, (WPARAM)tempstats, (LPARAM)NULL);
			stats[1] = 0;
			stats[4] = 0;
			ResetEvent(thread_data.m_reserved_events[event]);
		}
		if(event== WAIT_OBJECT_0 + 3)
		{
			for(int i = 0; i < 7; i++)
			{
				tempstats[i] = stats[i];
			}
			::PostMessage(hwnd,WM_SM_UPDATE_GUI, (WPARAM)tempstats, (LPARAM)NULL);
			stats[1] = 0;
			stats[2] = 0;
			stats[4] = 0;
			stats[5] = 0;
			ResetEvent(thread_data.m_reserved_events[event]);
		}
		if(event== WAIT_OBJECT_0 + 4)
		{
			for(int i = 0; i < 7; i++)
			{
				tempstats[i] = stats[i];
			}
			::PostMessage(hwnd,WM_SM_UPDATE_GUI, (WPARAM)tempstats, (LPARAM)NULL);
			stats[1] = 0;
			stats[2] = 0;
			stats[3] = 0;
			stats[4] = 0;
			stats[5] = 0;
			stats[6] = 0;
			ResetEvent(thread_data.m_reserved_events[event]);
		}
	}
	::PostMessage(hwnd,WM_SM_STOP_TIMER, (WPARAM)NULL, (LPARAM)NULL);
	return 0;	// exit the thread
}

//
//
//
SearchManager::SearchManager()
: p_critical_section(NULL), p_thread_data(NULL), p_parent(NULL), p_thread(NULL)
{
}

void SearchManager::Run()
{

	// Create message window so that it will accept messages posted to it
	if(m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL)==FALSE)
	{
		::MessageBox(NULL,"Error creating m_wnd in SearchManager()","Error",MB_OK);
	}
	m_wnd.InitParent(this);

	// Start the worker thread...passing it the handle to the message window
	HWND hwnd=m_wnd.GetSafeHwnd();
	p_thread = AfxBeginThread(SearchManagerThreadProc,(LPVOID)hwnd,THREAD_PRIORITY_LOWEST);

}
//
//
//
SearchManager::~SearchManager()
{
	// Kill the thread if we have received initialized the thread data
	if(p_critical_section!=NULL)
	{
		SetEvent(p_thread_data->m_reserved_events[0]);
	}

	if(p_thread != NULL)
	{
		WaitForSingleObject( p_thread->m_hThread, 2000 );
		p_thread = NULL;
	}

	// Destroy Window
	m_wnd.DestroyWindow();
}

//
//
//
void SearchManager::InitParent(SoulSeekServerDll *parent)
{
	p_parent=parent;
}

//
//
//
void SearchManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section = (CCriticalSection *)lparam;
	p_thread_data = (SearchManagerThreadData *)wparam;
}

void SearchManager::SetSMEvent(unsigned int theEvent)
{
	SetEvent( p_thread_data->m_reserved_events[2] );
}

void SearchManager::ReturnResults(WPARAM wparam,LPARAM lparam)
{
	p_parent->m_socket.GetClientAddress((SearchRequest *)wparam);
}

void SearchManager::ProcessSearchRequest(SearchRequest *pSearch)
{
	if( pSearch == NULL )
		return;

	if( p_critical_section == NULL )
		delete pSearch;

	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		if( p_thread_data->p_sr == NULL )
			p_thread_data->p_sr = new queue<SearchRequest *>;

		p_thread_data->p_sr->push( pSearch );

		SetEvent( p_thread_data->m_reserved_events[1] );
	}
}
void SearchManager::UpdateAffectiveness(unsigned int* affectiveness)
{
	p_parent->m_dlg.SetAffectivenessInfo(affectiveness[0], affectiveness[1], affectiveness[2], affectiveness[3], affectiveness[4], affectiveness[5], affectiveness[6]);
}