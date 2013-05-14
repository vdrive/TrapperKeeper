// OverNetLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OverNetLauncherDlg.h"

// COverNetLauncherDlg dialog

IMPLEMENT_DYNAMIC(COverNetLauncherDlg, CDialog)
COverNetLauncherDlg::COverNetLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COverNetLauncherDlg::IDD, pParent)
{
}

COverNetLauncherDlg::~COverNetLauncherDlg()
{
}

void COverNetLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL COverNetLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_num_launched = 0;
	return true;
}

BEGIN_MESSAGE_MAP(COverNetLauncherDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// This is the callback function
BOOL CALLBACK COverNetLauncherDlg::enumwndfn(HWND hWnd, LPARAM lParam)
{
	COverNetLauncherDlg *me = (COverNetLauncherDlg *)lParam;
	return me->enumwndfn(hWnd);
}

//
//	Enumerate the window handles
//
BOOL COverNetLauncherDlg::enumwndfn(HWND hwnd)
{
	char name[1024+1];
	memset(name,0,sizeof(name));

	// Is this window Kazaa?
	GetClassName(hwnd, name, sizeof(name)-1);
	if(stricmp(name,"ConsoleWindowClass")==0)
	{
		// First check to see if any other Client objects have claimed this handle
		bool found = false;
		for (int j=0; j < (int)v_clients.size(); j++)
		{
			if (v_clients[j].m_hWnd == hwnd)
			{
				found = true;
				break;
			}
		}

		// If not, then give it to the open one
		if (found == false)
		{
			for (int i=0; i < (int)v_clients.size(); i++)
			{
				if (v_clients[i].m_hWnd == NULL)	// is this the open one?
				{
					v_clients[i].m_hWnd = hwnd;
					break;
				}
			}
		}
		
	}

	return true;
}

// OnTimer() function
void COverNetLauncherDlg::OnTimer(UINT nIDEvent)
{
	KillTimer(nIDEvent);

	switch(nIDEvent)
	{
	case 1:
		{
			int ret_err=0;
			OvernetClient client;

			// If this return value is greater than 32, then it is the value of the handle to the 
			client.m_hInst = ShellExecute(NULL, "open", "overnetClc.exe", "a \\FastTrack Shared", "C:\\Program Files\\Overnet", SW_SHOWNORMAL);
			Sleep(2000);	// let the window appear

			// Check for an error on ShellExecute()
			ret_err = (int)client.m_hInst;
			if (ret_err <= 32)
			{
				// An error occured.
				MessageBox("An error occurred, exiting...", "Launch Error", MB_OK);
				exit(1);
			}
			else
			{
				client.m_id = m_num_launched+1;
				v_clients.push_back(client);

				// Enumerate child windows of the desktop, (top-level windows)
				EnumWindows(enumwndfn, (LPARAM)this);
			}

			//
			//	Change this number to change the number of Overnet Clients
			//	that will be launched on each machine.
			//
			if (++m_num_launched < 5)
				SetTimer(1, 3000, NULL);
			else
				InitClients();

			break;
		}

	default:
		{
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

//
//	After we have launched the appropriate number of clients, get them on seperate ports (if neccessary)
//	share the \Fasttrack Shared folder, give them username's, etc...
//
void COverNetLauncherDlg::InitClients()
{
	// Type a command in each client
	for (int i=0; i < (int)v_clients.size(); i++)
	{
		char *cmd = new char[256];

		// Share the \Fasttrack directory
		strcpy(cmd, "\ba+ \\FastTrack Shared\r");
		SendConsoleString(v_clients[i].m_hWnd, cmd);

		// Share the \My Music directory
		// ** Not for release mode
//		strcpy(cmd, "\b\ba+ \\Documents and Settings\\All Users\\Documents\\My Music\r");
//		SendConsoleString(v_clients[i].m_hWnd, cmd);

		// Set the TCP port
		sprintf(cmd, "\b\bport %d\r", 5000+v_clients[i].m_id*3);	// Set's the TCP port that this client listens for other clients on.
		SendConsoleString(v_clients[i].m_hWnd, cmd);

		// Set the UDP port
		sprintf(cmd, "\b\buport %d\r", 7000+v_clients[i].m_id*3);	// Set's the TCP port that this client listens for other clients on.
		SendConsoleString(v_clients[i].m_hWnd, cmd);

		// Clean up the quote
		strcpy(cmd, "\b");
		SendConsoleString(v_clients[i].m_hWnd, cmd);

		delete [] cmd;
	}
}

//
//
//
void COverNetLauncherDlg::KillClients()
{
	// Type the quite command for each client
	for (int i=0; i < (int)v_clients.size(); i++)
	{
		char *cmd = new char[256];
		strcpy(cmd, "q\r");

		SendConsoleString(v_clients[i].m_hWnd, cmd);

		strcpy(cmd, "y\r");
		SendConsoleString(v_clients[i].m_hWnd, cmd);

		delete [] cmd;
	}
}

//
//	Sends the commmand to the window
//
void COverNetLauncherDlg::SendConsoleString(HWND hwnd, LPTSTR pCmdString)
{
    HGLOBAL     hDropFiles;
    LPDROPFILES pDropFiles;
    LPTSTR      pFileNames;
    DWORD       dwLength, dwBytes;

    //
    // Alloc mem for DROPFILES structure and initialize.  Filename
    // list has one entry that is double null-terminated.
    //
    dwLength = lstrlen(pCmdString);
    dwBytes = sizeof(DROPFILES) + (dwLength + 2) * sizeof(TCHAR);
    hDropFiles = GlobalAlloc(GHND, dwBytes);
    if (hDropFiles != NULL) {
        pDropFiles = (LPDROPFILES)GlobalLock(hDropFiles);
        if (pDropFiles != NULL) {
            pDropFiles->pFiles = sizeof(DROPFILES);
#ifdef UNICODE
            pDropFiles->fWide = TRUE;
#endif
            pFileNames = (LPTSTR)((char *)pDropFiles + \
                            sizeof(DROPFILES));
            lstrcpy(pFileNames, pCmdString);
            GlobalUnlock(hDropFiles);
			::PostMessage(hwnd,WM_DROPFILES,(WPARAM) hDropFiles,0L);
        }
    }
}
