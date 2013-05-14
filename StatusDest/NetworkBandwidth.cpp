// NetworkBandwidth.cpp

#include "stdafx.h"
#include "NetworkBandwidth.h"

//
//
//
NetworkBandwidth::NetworkBandwidth(StatusDestDll* parent)
{
	Clear();
	m_dlg.InitParent(parent);
	m_dlg.Create(IDD_NETWORK_DIALOG,CWnd::GetDesktopWindow());
}

NetworkBandwidth::~NetworkBandwidth()
{
	Clear();
	m_dlg.DestroyWindow();
}
//
//
//
void NetworkBandwidth::Clear()
{
	m_network.clear();
	v_hosts.clear();
}

//
//
//
void NetworkBandwidth::ShowNetworkGUI()
{
	m_dlg.SetWindowText(m_network.c_str());
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}