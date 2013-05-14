// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "ClientModuleMessageWnd.h"
#include "ClientModule.h"

BEGIN_MESSAGE_MAP(ClientModuleMessageWnd,CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_MESSAGE(WM_CMOD_LOG_MSG,LogMsg)
	ON_MESSAGE(WM_CMOD_CLOSE_CONNECTION,CloseSocket)
	ON_MESSAGE(WM_CMOD_CONNECTED,Connected)
	ON_MESSAGE(WM_CMOD_START_TIMER,StartTimer) 
	ON_MESSAGE(WM_CMOD_STOP_TIMER,StopTimer)
	ON_MESSAGE(WM_CMOD_KILL,DeleteThread)
	ON_MESSAGE(WM_CMOD_WRITE_TO_LOG,WriteToLog)
END_MESSAGE_MAP()

//
//
//
ClientModuleMessageWnd::ClientModuleMessageWnd()
{
	p_mod=NULL;
	m_twentySecondTimer = NULL;
}

//
//
//
ClientModuleMessageWnd::~ClientModuleMessageWnd()
{
}

//
//
//
void ClientModuleMessageWnd::InitParent(ClientModule *mod)
{
	p_mod=mod;
}
LRESULT ClientModuleMessageWnd::StartTimer(WPARAM wparam,LPARAM lparam)
{
   if(m_twentySecondTimer != NULL)
   {
	   KillTimer(m_twentySecondTimer);
	   m_twentySecondTimer = NULL;
   }
   m_twentySecondTimer = SetTimer(0, 16*1000, 0);
   return 0;
}
LRESULT ClientModuleMessageWnd::StopTimer(WPARAM wparam,LPARAM lparam)
{
   if(m_twentySecondTimer != NULL)
   {
	   KillTimer(m_twentySecondTimer);
	   m_twentySecondTimer = NULL;
   }
   return 0;
}
void ClientModuleMessageWnd::OnTimer(UINT nIDEvent)
{
	p_mod->SetEvent(2);
}

//
//
//
LRESULT ClientModuleMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT ClientModuleMessageWnd::LogMsg(WPARAM wparam,LPARAM lparam)
{
	if( wparam == NULL )
		return -1;

	p_mod->LogMsg( (char *)wparam );
	return 0;
}

//
//
//
LRESULT ClientModuleMessageWnd::CloseSocket(WPARAM wparam,LPARAM lparam)
{
	p_mod->IncrementSocketsAvail();
	p_mod->CloseConnection();
	return 0;
}
LRESULT ClientModuleMessageWnd::Connected(WPARAM wparam,LPARAM lparam)
{
	//p_mod->DecrementSocketsAvail();
	p_mod->OpenAnotherConnection();
	return 0;
}
LRESULT ClientModuleMessageWnd::DeleteThread(WPARAM wparam,LPARAM lparam)
{
	p_mod->DeleteThread();
	return 0;
}
LRESULT ClientModuleMessageWnd::WriteToLog(WPARAM wparam,LPARAM lparam)
{
	p_mod->WriteToFile((char *)wparam);
	return 0;
}