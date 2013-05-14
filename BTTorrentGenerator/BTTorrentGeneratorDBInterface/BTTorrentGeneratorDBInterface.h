// BTTorrentGeneratorDBInterface.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CBTTorrentGeneratorDBInterfaceApp:
// See BTTorrentGeneratorDBInterface.cpp for the implementation of this class
//

class CBTTorrentGeneratorDBInterfaceApp : public CWinApp
{
public:
	CBTTorrentGeneratorDBInterfaceApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CBTTorrentGeneratorDBInterfaceApp theApp;