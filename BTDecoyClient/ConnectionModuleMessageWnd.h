// ConnectionModuleMessageWnd.h
#pragma once
#include "StdAfx.h"

class ConnectionModule;
class ConnectionModuleMessageWnd
	: public CWnd
{
public:
	ConnectionModuleMessageWnd();
	~ConnectionModuleMessageWnd();
	inline void InitParent(ConnectionModule *mod) { m_pMod = mod; }

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT ClientConnect(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ClientDisconnect(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT ClientConnectionError(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ConnectionsRepaired(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT TorrentSearch(WPARAM wparam, LPARAM lparam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

private:
	ConnectionModule *	m_pMod;
	UINT_PTR			m_oneSecondTimer;
	UINT_PTR			m_repairTimer;
};