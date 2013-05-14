#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "RackStatusDlg.h"

// CMainDlg dialog

class SupernodeControllerDll;
class CMainDlg : public CDialog
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainDlg();
	void InitParent(SupernodeControllerDll* parent);

// Dialog Data
	enum { IDD = IDD_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListBox m_log_list;
	SupernodeControllerDll* p_parent;

	void RefreshControllerMessagesStatus();

public:
	afx_msg void OnTimer(UINT nIDEvent);
	void Log(const char * text);
	void UpdateStatus(int total_collectors, int up_collectors, int supernodes_collected,int supernodes_consumed, int num_launchers,
		int launcher_supernodes);
	afx_msg void OnBnClickedSendInitButton();
	afx_msg void OnBnClickedRackStatusButton();

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedIsSortedButton();

public:
	CRackStatusDlg m_rack_status_dlg;
	UINT m_remote_supernode_list_count,m_check_new_supernode_count,m_request_new_supernode_count,m_remove_supernode_count;
	UINT m_supernode_taken,m_duplicated_supernode, m_supernode_removed;
	afx_msg void OnBnClickedResetButton();
	afx_msg void OnBnClickedSaveSupernodesButton();
	afx_msg void OnBnClickedResetSharedFoldersButton();
};
