// TKLauncherDlg.h : header file
//

#pragma once


// CTKLauncherDlg dialog
class CTKLauncherDlg : public CDialog
{
// Construction
public:
	CTKLauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TKLAUNCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
protected:
	virtual void OnCancel();

private:
	bool m_closing;

	void LaunchTrapperKeeper();
	void CrashLog();
	bool FindProcess(const char* process);
	void KillProcess(const char* process);

};
