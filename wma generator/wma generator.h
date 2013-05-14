// wma generator.h : main header file for the wma generator application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "MusicInfo.h"
#include <vector>
using namespace std;

int ProcessTemplate(char * buf, MusicInfo * info);
void GenerateFilename(MusicInfo * info);
void RenameInputs();
int Runthis();


// CwmageneratorApp:
// See wma generator.cpp for the implementation of this class
//

class CwmageneratorApp : public CWinApp
{
public:
	CwmageneratorApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CwmageneratorApp theApp;