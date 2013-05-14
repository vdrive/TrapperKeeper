// SearchManager.h

#pragma once
#include <queue>
#include <vector>
using namespace std;

#include "SearchManagerMessageWnd.h"
#include "SearchManagerThreadData.h"

class CCriticalSection;
class SoulSeekServerDll;

class SearchManager
{
public:
	SearchManager();
	~SearchManager();
	void Run();
	void InitParent(SoulSeekServerDll *parent);
	void SetSMEvent(unsigned int theEvent);

	// Messages
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void ReturnResults(WPARAM wparam,LPARAM lparam);
	void ProcessSearchRequest(SearchRequest *pSearch);
	void UpdateAffectiveness(unsigned int* affectiveness);

private:
	SoulSeekServerDll *p_parent;

	CCriticalSection *p_critical_section;
	SearchManagerThreadData *p_thread_data;
	CWinThread *p_thread;

	SearchManagerMessageWnd m_wnd;
};