// SearchManagerMessageWnd.cpp

#include "stdafx.h"
#include "SearchManagerMessageWnd.h"
#include "SearchManager.h"

BEGIN_MESSAGE_MAP(SearchManagerMessageWnd,CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_MESSAGE(WM_SM_SEND_RESULTS,ReturnResults)
	ON_MESSAGE(WM_SM_START_TIMER,StartTimer)
	ON_MESSAGE(WM_SM_STOP_TIMER,StopTimer)
	ON_MESSAGE(WM_SM_UPDATE_GUI,UpdateGUI)
END_MESSAGE_MAP()

//
//
//
SearchManagerMessageWnd::SearchManagerMessageWnd()
{
	p_mod=NULL;
}

//
//
//
SearchManagerMessageWnd::~SearchManagerMessageWnd()
{
}

//
//
//
void SearchManagerMessageWnd::InitParent(SearchManager *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT SearchManagerMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT SearchManagerMessageWnd::ReturnResults(WPARAM wparam,LPARAM lparam)
{
	p_mod->ReturnResults(wparam,lparam);
	return 0;
}

LRESULT SearchManagerMessageWnd::StartTimer(WPARAM wparam,LPARAM lparam)
{
   m_minuteTimer = SetTimer(0, 60*1000, 0);
   m_hourTimer = SetTimer(1, 60*60*1000, 0);
   m_dayTimer = SetTimer(2, 24*60*60*1000, 0);
   return 0;
}
LRESULT SearchManagerMessageWnd::StopTimer(WPARAM wparam,LPARAM lparam)
{
   if(m_minuteTimer != NULL)
   {
	   KillTimer(m_minuteTimer);
	   m_minuteTimer = NULL;
   }
   if(m_hourTimer != NULL)
   {
	   KillTimer(m_hourTimer);
	   m_hourTimer = NULL;
   }
   if(m_dayTimer != NULL)
   {
	   KillTimer(m_dayTimer);
	   m_dayTimer = NULL;
   }
   return 0;
}

void SearchManagerMessageWnd::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == 0)
	{
		p_mod->SetSMEvent(2);
	}
	else if(nIDEvent == 1)
	{
		p_mod->SetSMEvent(3);
	}
	else if(nIDEvent == 2)
	{
		p_mod->SetSMEvent(4);
	}
}

LRESULT SearchManagerMessageWnd::UpdateGUI(WPARAM wparam,LPARAM lparam)
{
	p_mod->UpdateAffectiveness((unsigned int *) wparam);
    return 0;
}