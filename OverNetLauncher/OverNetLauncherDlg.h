#pragma once
#include "resource.h"
#include "OvernetClient.h"

// COverNetLauncherDlg dialog

class COverNetLauncherDlg : public CDialog
{
	DECLARE_DYNAMIC(COverNetLauncherDlg)

public:
	COverNetLauncherDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COverNetLauncherDlg();

	// Dialog Data
	enum { IDD = IDD_OVERNET_LAUNCHER_DLG };

	BOOL OnInitDialog();

/****************************************************************************************************************/
/*/
/*/
	//
	// My added functions:
	//
	void InitClients();
	void KillClients();
	void SendConsoleString(HWND hwnd, LPTSTR pCmdString);

	static BOOL CALLBACK enumwndfn(HWND hWnd, LPARAM lParam);
	BOOL enumwndfn(HWND hwnd);

	//
	// My added (member) variables:
	//
	int m_num_launched;
	vector<OvernetClient> v_clients;

/*/
/*/
/****************************************************************************************************************/

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
};
