// TKLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TKLauncher.h"
#include "TKLauncherDlg.h"
#include <Psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTKLauncherDlg dialog



CTKLauncherDlg::CTKLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTKLauncherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_closing = false;
}

void CTKLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTKLauncherDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTKLauncherDlg message handlers

BOOL CTKLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MINIMIZE);
	LaunchTrapperKeeper();
	SetTimer(1, 15*60*1000, 0);	// 15 mins

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTKLauncherDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTKLauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
//
//
void CTKLauncherDlg::OnTimer(UINT nIDEvent)
{
	KillTimer(nIDEvent);

	if(nIDEvent==1)
	{
		LaunchTrapperKeeper();
	
		SetTimer(1, 15*60*1000, 0);	// 15 mins
	}
	
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CTKLauncherDlg::OnClose()
{
	if(MessageBox("Do you really want to exit?","Question",MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)==IDNO)
	{
		return;
	}

	m_closing=true;
	
	CDialog::OnClose();
}

//
//
//
void CTKLauncherDlg::OnCancel()
{
	if(m_closing)
	{
		CDialog::OnCancel();
	}
}

//
//
//
void CTKLauncherDlg::LaunchTrapperKeeper()
{
	CWnd *wnd_ptr=CWnd::FindWindow("Trapper Keeper V1.x",NULL);
	if(wnd_ptr==NULL)
	{
		if(FindProcess("TrapperKeeper.exe"))
		{
			KillProcess("TrapperKeeper.exe");
		}
		CrashLog();
		ShellExecute(NULL, "open", "TrapperKeeper.exe", NULL, NULL, SW_SHOWNORMAL);
	}
}

//
//
//
void CTKLauncherDlg::CrashLog()
{
	CStdioFile file;
	if(file.Open("crash_log.txt", CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeText|CFile::shareDenyWrite)!=0)
	{
		file.SeekToEnd();
		CString time = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
		file.WriteString(time);
		file.WriteString("\n");
		file.Close();
	}
}

//
//
//
bool CTKLauncherDlg::FindProcess(const char* process)
{
	bool found = false;
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	HANDLE handle;
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp(process,name)==0)
			{
				found = true;
				CloseHandle(handle);
				break;
			}
		}
		CloseHandle(handle);
	}
	return found;
}

//
//
//
void CTKLauncherDlg::KillProcess(const char* process)
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	HANDLE handle;
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp(process,name)==0)
			{
				TerminateProcess(handle,1);
				CloseHandle(handle);
				break;
			}
		}
		CloseHandle(handle);
	}
}