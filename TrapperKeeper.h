// TrapperKeeper.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "TrapperKeeperWnd.h"

// CTrapperKeeperApp:
// See TrapperKeeper.cpp for the implementation of this class
//

class DllLoaderApp;
class CTrapperKeeperApp : public CWinApp
{
public:
	CTrapperKeeperApp();

// Overrides
	public:
	virtual BOOL InitInstance();
	void Tap();
	void ReloadDlls(WPARAM wparam,LPARAM lparam);
	TrapperKeeperWnd m_wnd;
	DllLoaderApp* p_app;

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CTrapperKeeperApp theApp;