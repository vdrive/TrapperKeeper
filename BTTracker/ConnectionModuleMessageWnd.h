// ConnectionModuleMessageWnd.h
#pragma once
#include "StdAfx.h"

class ConnectionModule;
class ConnectionModuleMessageWnd
	: public CWnd
{
public:
	ConnectionModuleMessageWnd();
	~ConnectionModuleMessageWnd(){};
	inline void InitParent(ConnectionModule *mod) { m_pMod = mod; }

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT ClientConnect(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ClientDisconnect(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT ClientConnectionError(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ConnectionsRepaired(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT TorrentIndexReq(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

private:
	ConnectionModule *m_pMod;
};