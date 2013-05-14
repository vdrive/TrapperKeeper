#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

#define NUM_OF_USERNAMES 30
#define USERNAME00 "Psycho666"
#define USERNAME01 "SkiJunkie8"
#define USERNAME02 "bitch50"
#define USERNAME03 "BooZeHouNd050"
#define USERNAME04 "Jen3"
#define USERNAME05 "AssFace777"
#define USERNAME06 "SnickersEatsIT4"
#define USERNAME07 "JoeySux71"
#define USERNAME08 "miranda7"
#define USERNAME09 "malonepe8"
#define USERNAME10 "sKaterdude092"
#define USERNAME11 "Lick_My_Ass11"
#define USERNAME12 "ace_man73"
#define USERNAME13 "catalinamateus431"
#define USERNAME14 "anon_423f124"
#define USERNAME15 "pimpdbond987"
#define USERNAME16 "d8me4weed9"
#define USERNAME17 "Parry_Squater30"
#define USERNAME18 "jimmyzGotCrabs452"
#define USERNAME19 "dNtBaH8tR911"
#define USERNAME20 "onlyone73"
#define USERNAME21 "whoknows843"
#define USERNAME22 "biggensmgu01"
#define USERNAME23 "barleyNwheat234"
#define USERNAME24 "music4less034"
#define USERNAME25 "backoffmesucka411"
#define USERNAME26 "whatchoowant200"
#define USERNAME27 "igotit666"
#define USERNAME28 "whoop510"
#define USERNAME29 "jumpshot8000"

// CAresLauncherDlg dialog

class CAresLauncherDlg : public CDialog
{
	DECLARE_DYNAMIC(CAresLauncherDlg)

public:
	CAresLauncherDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAresLauncherDlg();

// Dialog Data
	enum { IDD = IDD_ARES_DLG };

protected:
	HICON m_hIcon;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:

	int m_running;
	int m_operatingOn;
	int m_usernameCounter;

	DWORD m_processPid;

	CString m_userNames[ NUM_OF_USERNAMES ];

	afx_msg void OnBnClickedRunAres();
	afx_msg void OnBnClickedKillAres();
	static BOOL CALLBACK ChangeAresName( HWND hwnd, LPARAM lParam );
	static BOOL CALLBACK KillAres( HWND hwnd, LPARAM lParam );

	CEdit m_CEditOperateOn;
	CEdit m_CEditRunning;
	CEdit m_CEditMaxRunning;
	CListCtrl m_CListRunning;
};

// convert a string of alpha-numeric characters
// to a string of the respective ASCII values
// in hex.  Destination array must be twice the
// length of the source array.
void atoh( char *, const char * );

// returns a single char which is the hex value
// of the passed int.  If the passed int is not
// in the range of a single hex digit, null is
// returned.
char hexToChar( int );