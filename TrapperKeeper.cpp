// TrapperKeeper.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TrapperKeeper.h"
#include "DllLoader/DllLoaderApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrapperKeeperApp

BEGIN_MESSAGE_MAP(CTrapperKeeperApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTrapperKeeperApp construction
CTrapperKeeperApp::CTrapperKeeperApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

//
//
// The one and only CTrapperKeeperApp object
CTrapperKeeperApp theApp;

//
//
// CTrapperKeeperApp initialization
BOOL CTrapperKeeperApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	//InitCommonControls();
	//CWinApp::InitInstance();
	//AfxEnableControlContainer();
#ifndef _DEBUG
	SetErrorMode(SEM_NOGPFAULTERRORBOX|SEM_FAILCRITICALERRORS);
#endif

	CWnd *wnd_ptr=NULL;
	wnd_ptr=CWnd::FindWindow("Trapper Keeper V1.x",NULL);
	if(wnd_ptr!=NULL)
	{
		HWND hwnd=wnd_ptr->GetSafeHwnd();
		BOOL ret=::PostMessage(hwnd,WM_TAP,0,0);			
		return FALSE;
	}
	p_app = new DllLoaderApp();
	m_pMainWnd=(CWnd*)p_app->p_dlg;
	m_pActiveWnd=m_pMainWnd;

	if(p_app->InitInstance()==FALSE)
	{
		return FALSE;	// quit
	}

	// Create the window whose class name we can find to tap
	WNDCLASS wndclass;
	memset(&wndclass,0,sizeof(wndclass));
	wndclass.style = 0;
	wndclass.lpfnWndProc = AfxWndProc;
	wndclass.hInstance = AfxGetInstanceHandle();
	wndclass.hIcon = 0;
	wndclass.hCursor =  0;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "Trapper Keeper V1.x";
	BOOL ret=AfxRegisterClass(&wndclass);
	m_wnd.CreateEx(0,"Trapper Keeper V1.x","Trapper Keeper V1.x",0,0,0,0,0,0,0,NULL);
	
	//We can tap this window to bring up the main app if we want
	m_wnd.InitApp(this);

	return TRUE;
}

//
//
//
int CTrapperKeeperApp::ExitInstance()
{
//	m_wnd.DestroyWindow();
	delete p_app;
	return CWinApp::ExitInstance();
}

//
//
//
void CTrapperKeeperApp::Tap()
{
	// Make the main window or the app come up
	// Restore if minimized
	WINDOWPLACEMENT wp;
	m_pMainWnd->GetWindowPlacement(&wp);
	wp.showCmd=SW_SHOWNORMAL;
	m_pMainWnd->SetWindowPlacement(&wp);
			
	// Bring to front
	m_pMainWnd->SetForegroundWindow();
}

//
//
//
void CTrapperKeeperApp::ReloadDlls(WPARAM wparam,LPARAM lparam)
{
	p_app->ReloadDlls(wparam, lparam);
}
