// AresLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AresLauncherDll.h"
#include "AresLauncherDlg.h"


// Global to carry the text to use to rename the
// Ares process to the EnumWindows() function
CString g_newText;

// CAresLauncherDlg dialog

IMPLEMENT_DYNAMIC(CAresLauncherDlg, CDialog)
CAresLauncherDlg::CAresLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAresLauncherDlg::IDD, pParent)
{
}

CAresLauncherDlg::~CAresLauncherDlg()
{
}

void CAresLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_CEditOperateOn);
	DDX_Control(pDX, IDC_EDIT2, m_CEditRunning);
	DDX_Control(pDX, IDC_EDIT3, m_CEditMaxRunning);
	DDX_Control(pDX, IDC_LIST1, m_CListRunning);
}


BEGIN_MESSAGE_MAP(CAresLauncherDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedRunAres)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedKillAres)
END_MESSAGE_MAP()


// CAresLauncherDlg message handlers

BOOL CAresLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Variable used to set registry values
	DWORD value = 1;

	// Sets the array of usernames
	m_userNames[ 0 ] = USERNAME00;
	m_userNames[ 1 ] = USERNAME01;
	m_userNames[ 2 ] = USERNAME02;
	m_userNames[ 3 ] = USERNAME03;
	m_userNames[ 4 ] = USERNAME04;
	m_userNames[ 5 ] = USERNAME05;
	m_userNames[ 6 ] = USERNAME06;
	m_userNames[ 7 ] = USERNAME07;
	m_userNames[ 8 ] = USERNAME08;
	m_userNames[ 9 ] = USERNAME09;
	m_userNames[ 10 ] = USERNAME10;
	m_userNames[ 11 ] = USERNAME11;
	m_userNames[ 12 ] = USERNAME12;
	m_userNames[ 13 ] = USERNAME13;
	m_userNames[ 14 ] = USERNAME14;
	m_userNames[ 15 ] = USERNAME15;
	m_userNames[ 16 ] = USERNAME16;
	m_userNames[ 17 ] = USERNAME17;
	m_userNames[ 18 ] = USERNAME18;
	m_userNames[ 19 ] = USERNAME19;
	m_userNames[ 20 ] = USERNAME20;
	m_userNames[ 21 ] = USERNAME21;
	m_userNames[ 22 ] = USERNAME22;
	m_userNames[ 23 ] = USERNAME23;
	m_userNames[ 24 ] = USERNAME24;
	m_userNames[ 25 ] = USERNAME25;
	m_userNames[ 26 ] = USERNAME26;
	m_userNames[ 27 ] = USERNAME27;
	m_userNames[ 28 ] = USERNAME28;
	m_userNames[ 29 ] = USERNAME29;

	// Open the registry key for Ares, and set the shared directory
	HKEY hKey;
	char subkey[]="Software\\Ares";
	RegCreateKeyEx( HKEY_CURRENT_USER, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL );

	// Change the shared folder to C:\My Shared Folder, using the registry
	char data[256];
	atoh( data, "C:\\FastTrack Shared" );
	DWORD dataSize = (DWORD)( strlen( data ) + 1 );
	RegSetValueEx( hKey, "Download.Folder", 0, REG_SZ, (unsigned char *)&data, dataSize );
	
	// Setting for close when we send it the OnClose
	value = 1;
	RegSetValueEx( hKey, "General.CloseOnQuery", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Setting to automatically connect when Ares Launches
	value = 1;
	RegSetValueEx( hKey, "General.AutoConnect", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Setting for maximum number of uploads
	value = 25;
	RegSetValueEx( hKey, "Transfer.MaxUpCount", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Setting for shared types (Audio + Video)
	value = 48;
	RegSetValueEx( hKey, "ChatRoom.SharedTypes", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Setting to block private messages
	value = 1;
	RegSetValueEx( hKey, "PrivateMessage.BlockAll", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Setting to display a message that we are away, when a private message is blocked
	value = 1;
	RegSetValueEx( hKey, "PrivateMessage.SetAway", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD) );

	// Close the registry
	RegCloseKey( hKey );

	// Initialize the number to run/kill
#ifdef _DEBUG
	m_operatingOn = 2;
	m_CEditOperateOn.SetWindowText( "2" );
#else
	m_operatingOn = 12;
	m_CEditOperateOn.SetWindowText( "25" );
#endif

	// Initialize the number of running to 0
	m_running = 0;
	m_CEditRunning.SetWindowText( "0" );
	
	// Initialize the max number to run/kill to
	// the NUM_OF_USERNAMES constant
	char buf[128];
	itoa( NUM_OF_USERNAMES, buf, 10 );
	m_CEditMaxRunning.SetWindowText( buf );

	m_usernameCounter = 0;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAresLauncherDlg::OnBnClickedRunAres()
{
	// Find out how many to run
	char buf[128];
	m_CEditOperateOn.GetWindowText( buf, 128 );
	m_operatingOn = atoi( buf );

	// Loop until all the max number of Ares are running, or
	// no more need to be created.
	for( int i = m_operatingOn; (i > 0) && (m_running < NUM_OF_USERNAMES ); i-- )
	{
		// Open the registry key for Ares
		HKEY hKey;
		char subkey[]="Software\\Ares";
		RegCreateKeyEx( HKEY_CURRENT_USER, subkey, 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey, NULL );

		// Create the nickname key and enter it into the registry
		char data[256];
		g_newText = m_userNames[ m_usernameCounter ];
		atoh( data, m_userNames[ m_usernameCounter++ ] );
		DWORD dataSize = (DWORD)( strlen( data ) + 1 );
		RegSetValueEx( hKey, "Personal.Nickname", 0, REG_SZ,
			(unsigned char *)&data, dataSize );
		
		// Close the registry
		RegCloseKey( hKey );

		// If the m_usernameCounter is past the number of usernames possible,
		// then restart the m_usernameCounter from 0
		if( m_usernameCounter >= NUM_OF_USERNAMES )
			m_usernameCounter = 0;

		PROCESS_INFORMATION pi;
		STARTUPINFOA si = {sizeof si};
		si.lpDesktop = "Default";

		LPTSTR lpszCommand = "C:\\Program Files\\Ares\\Ares.exe";
		LPTSTR lpszRunIn = "C:\\Program Files\\Ares";

		BOOL bret = CreateProcess( NULL, lpszCommand, NULL, NULL, FALSE,
			BELOW_NORMAL_PRIORITY_CLASS, NULL, lpszRunIn, &si, &pi);

		if(bret == 0)
			TRACE0("Failed to launch Ares\n");

		Sleep(2000);  // Take a nap

		EnumWindows( ChangeAresName, m_processPid );

		m_running++;
		itoa( m_running, buf, 10 );
		m_CEditRunning.SetWindowText( buf );
	}
}

void CAresLauncherDlg::OnBnClickedKillAres()
{
	BOOL found = false;

	// Loop until all the Ares are closed
	while (found == false)
	{
		found = EnumWindows( KillAres, m_processPid );
		Sleep(500);  // Take a nap
	}

/*
	// Find out how many to kill
	char buf[128];
	m_CEditOperateOn.GetWindowText( buf, 128 );
	m_operatingOn = atoi( buf );

	// Loop until all the max number of Ares are running, or
	// no more need to be created.
	for( int i = m_operatingOn; (i > 0) && (m_running > 0); i-- )
	{
		EnumWindows( KillAres, m_processPid );

		Sleep(1000);  // Take a nap

		m_running--;
		itoa( m_running, buf, 10 );
		m_CEditRunning.SetWindowText( buf );
	}
*/
}

BOOL CALLBACK CAresLauncherDlg::ChangeAresName( HWND hwnd, LPARAM lParam )
{
	DWORD wndPid;
	CString Title;

	// This gets the windows handle and pid of enumerated window.
	GetWindowThreadProcessId(hwnd, &wndPid);

	// This gets the windows title text
	// from the window, using the window handle
	CWnd::FromHandle( hwnd )->GetWindowText( Title );

	TRACE( "%s\n", Title );

	if( strcmp( Title, " Ares " ) == 0 )
	{
		TRACE( "\t**** Found ' Ares '! ****\n" );

		g_newText.Insert( 0, "ARES:" );
		CWnd::FromHandle( hwnd )->SetWindowText( g_newText );
		CWnd::FromHandle( hwnd )->GetWindowText( Title );

		TRACE( "\tChanged ' Ares ' to '%s'\n", Title );
		return false;
	}

	return true;
}


BOOL CALLBACK CAresLauncherDlg::KillAres( HWND hwnd, LPARAM lParam )
{
	DWORD wndPid;
	CString Title;

	// This gets the windows handle and pid of enumerated window.
	GetWindowThreadProcessId(hwnd, &wndPid);

	// This gets the windows title text
	// from the window, using the window handle
	CWnd::FromHandle( hwnd )->GetWindowText( Title );

	TRACE( "%s\n", Title );

	if( strnicmp( Title, "ARES:", 5 ) == 0 )
	{
		TRACE( "\t**** Found renamed Ares! ****\n" );

		::PostMessage(hwnd, WM_CLOSE, 0, 0);

		TRACE( "\t**** Killed '%s'! ****\n", Title );
		return false;
	}

	return true;
}


void atoh( char *hexString, const char *alphaString )
{
	int len = (int)strlen( alphaString );

	// Define HEX to ASCII map
	char hex2asciiMap[256] =
	{ '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
	  ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
	  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
	  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', 
	  '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
	  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '(', '|', ')', '~', '\0' };

	for( int i = 0, j = 0; i < len; i++, j += 2 )
	{
		for( int k = 32; k < 255; k++ )
		{
			if( hex2asciiMap[k] == alphaString[i] )
			{
				hexString[j] = hexToChar( k / 16 );
				hexString[j+1] = hexToChar( k % 16 );
				break;
			}
		}
	}

	hexString[len*2] = '\0';
}

char hexToChar( int hex )
{
	switch( hex )
	{
	case 0:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	case 10:
		return 'A';
	case 11:
		return 'B';
	case 12:
		return 'C';
	case 13:
		return 'D';
	case 14:
		return 'E';
	case 15:
		return 'F';
	default:
		return '\0';
	}
}