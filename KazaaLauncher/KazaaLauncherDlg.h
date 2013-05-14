#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "KazaaProcess.h"
#include "afxwin.h"

// CKazaaLauncherDlg dialog
#define SUB_PROCESS_ID		0
#define SUB_CONNECTING_SUPERNODE_IP	1
#define SUB_CONNECTED_SUPERNODE_IP	2

class KazaaLauncherDll;
class CKazaaLauncherDlg : public CDialog
{
	DECLARE_DYNAMIC(CKazaaLauncherDlg)

public:
	CKazaaLauncherDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CKazaaLauncherDlg();
	void InitParent(KazaaLauncherDll* parent);
	afx_msg LRESULT GotThreadDoneMessage(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT OnKillKazaaMessage(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT OnLogMessage(WPARAM wparam,LPARAM lparam);

// Dialog Data
	enum { IDD = IDD_KAZAA_LAUNCHER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_supernode_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	int m_border;
	int m_bottom;
	KazaaLauncherDll* p_parent;
public:
	afx_msg void OnBnClickedUpdateButton();
	void UpdateSupernodeList(KazaaProcess& process);
	void RemoveKazaa(DWORD process_id);
	afx_msg void OnTimer(UINT nIDEvent);
	void Log(const char * text);

private:
	CListBox m_log_list;
public:
	afx_msg void OnBnClickedKillAllKazaaButton();
	afx_msg void OnBnClickedMinimizeAllKazaaButton();
	afx_msg void OnBnClickedRestoreButton();
};
