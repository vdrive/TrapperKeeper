// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"

BEGIN_MESSAGE_MAP(ConnectionModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_WM_TIMER()
	ON_MESSAGE(WM_STATUS_READY,StatusReady)
	//ON_MESSAGE(WM_VENDOR_COUNTS_READY,VendorCountsReady)
	ON_MESSAGE(WM_LOG_MSG,LogMsg)
	ON_MESSAGE(WM_SERVER_STATUS_MSG, ServerStatusMsg)
	ON_MESSAGE(WM_PARENT_STATUS_MSG, ParentStatusMsg)
	ON_MESSAGE(WM_PEER_STATUS_MSG, PeerStatusMsg)
	ON_MESSAGE(WM_SEND_TASK, SendTask)
	ON_MESSAGE(WM_ADD_PARENT, AddParent)
	ON_MESSAGE(WM_USER_NAME, SetUserName)
	ON_MESSAGE(WM_CONNECTED_SOCKETS, ConnectedSockets)
	ON_MESSAGE(WM_CONNECTING_SOCKETS, ConnectingSockets)
	ON_MESSAGE(WM_DROP_CACHED_PARENT, DropCachedParent)
	ON_MESSAGE(WM_NUM_PARENT_CONNECTING, setParentConnecting)
	ON_MESSAGE(WM_NUM_PEER_CONNECTING, setPeerConnecting)
	ON_MESSAGE(WM_WRITE_TO_LOG, WriteToLog)
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
void ConnectionModuleMessageWnd::OnTimer(UINT nIDEvent)
{
	p_mod->OnTimer(nIDEvent);
}

//
//
//
LRESULT ConnectionModuleMessageWnd::StatusReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->StatusReady(wparam,lparam);
	return 0;
}

//
//
//
/*LRESULT ConnectionModuleMessageWnd::VendorCountsReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->VendorCountsReady(wparam,lparam);
	return 0;
}
*/
//
//
//
LRESULT ConnectionModuleMessageWnd::setParentConnecting(WPARAM wparam,LPARAM lparam)
{
	p_mod->setParentConnecting(wparam,lparam);
	return 0;
}
LRESULT ConnectionModuleMessageWnd::setPeerConnecting(WPARAM wparam,LPARAM lparam)
{
	p_mod->setPeerConnecting(wparam,lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::LogMsg(WPARAM wparam,LPARAM lparam)
{
	p_mod->LogMsg(wparam,lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::WriteToLog(WPARAM wparam,LPARAM lparam)
{
	p_mod->WriteToLog(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::ServerStatusMsg(WPARAM wparam, LPARAM lparam)
{
	p_mod->ServerStatusMsg(wparam, lparam);
	return 0;
}
LRESULT ConnectionModuleMessageWnd::ParentStatusMsg(WPARAM wparam,LPARAM lparam)
{
	p_mod->ParentStatusMsg(wparam, lparam);
	return 0;
}
LRESULT ConnectionModuleMessageWnd::PeerStatusMsg(WPARAM wparam,LPARAM lparam)
{
	p_mod->PeerStatusMsg(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::SendTask(WPARAM wparam, LPARAM lparam)
{
	p_mod->SendTask(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::SetUserName(WPARAM wparam, LPARAM lparam)
{
	p_mod->SetUserName(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::AddParent(WPARAM wparam, LPARAM lparam)
{
	p_mod->AddParent(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::ConnectedSockets(WPARAM wparam, LPARAM lparam)
{
	p_mod->ChangeConnectedSockets(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::ConnectingSockets(WPARAM wparam, LPARAM lparam)
{
	p_mod->ChangeConnectingSockets(wparam, lparam);
	return 0;
}

LRESULT ConnectionModuleMessageWnd::DropCachedParent(WPARAM wparam, LPARAM lparam)
{
	p_mod->DropCachedParent((SupernodeHost *)wparam);
	return 0;
}