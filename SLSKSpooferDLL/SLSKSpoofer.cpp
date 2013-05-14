// SLSKSpoofer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SLSKSpoofer.h"
#include "SLSKSpooferDlg.h"
#include "WSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSLSKSpooferApp



// CSLSKSpooferApp construction

CSLSKSpooferApp::CSLSKSpooferApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSLSKSpooferApp object

CSLSKSpooferApp theApp;


// CSLSKSpooferApp initialization

BOOL CSLSKSpooferApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	AfxInitRichEdit2();

	InitCommonControls();

	//CWinApp::InitInstance();

	AfxEnableControlContainer();


	WSocket::Startup();

	CSLSKSpooferDlg dlg;
	//m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	WSocket::Cleanup();
	return FALSE;
}
