// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"

BEGIN_MESSAGE_MAP(ConnectionModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_MESSAGE(WM_CMOD_LOG_MSG,LogMsg)
	ON_MESSAGE(WM_CMOD_CLOSE_CONNECTION,CloseSocket)
	ON_MESSAGE(WM_CMOD_RECEIVED_CONNECTION,ReceivedConnection)
	ON_MESSAGE(WM_CMOD_SEARCH_REQUEST,ProcessSearchRequest)
END_MESSAGE_MAP()

//
//
//
ConnectionModuleMessageWnd::ConnectionModuleMessageWnd()
{
	p_mod=NULL;
}

//
//
//
ConnectionModuleMessageWnd::~ConnectionModuleMessageWnd()
{
}

//
//
//
void ConnectionModuleMessageWnd::InitParent(ConnectionModule *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::LogMsg(WPARAM wparam,LPARAM lparam)
{
	if( wparam == NULL )
		return -1;

	p_mod->LogMsg( (char *)wparam );
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::CloseSocket(WPARAM wparam,LPARAM lparam)
{
	p_mod->IncrementSocketsAvail();
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ReceivedConnection(WPARAM wparam,LPARAM lparam)
{
	p_mod->ReceivedConnection( (unsigned int)wparam );
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ProcessSearchRequest(WPARAM wparam,LPARAM lparam)
{
	p_mod->ProcessSearchRequest( (SearchRequest *)wparam );
	return 0;
}
