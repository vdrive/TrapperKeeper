// MediaSwarmerSource.h : main header file for the MediaSwarmerSource DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CMediaSwarmerSourceApp
// See MediaSwarmerSource.cpp for the implementation of this class
//

class CMediaSwarmerSourceApp : public CWinApp
{
public:
	CMediaSwarmerSourceApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
