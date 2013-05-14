#pragma once
#include "Resource.h"
#include "afxcmn.h"
#include "EditDlg.h"
#include "RemoveDllEditDlg.h"

#define SUB_NETWORK				0
#define SUB_COMPUTERS			1
//#define SUB_COMPUTERS_UP		2
#define SUB_PROC				2
#define SUB_BANDWIDTH			3
#define SUB_UPLOAD_BANDWIDTH	4
#define SUB_DOWNLOAD_BANDWIDTH	5
#define SUB_AVG_FILES_SHARED	6
#define SUB_IP_RANGE			7

// CStatusDestDlg dialog
class StatusDestDll;
class CStatusDestDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusDestDlg)

public:
	CStatusDestDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(StatusDestDll* parent);
	virtual ~CStatusDestDlg();
	CListCtrl m_network_list;

// Dialog Data
	enum { IDD = IDD_STATUS_DEST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	StatusDestDll* p_parent;
	CEditDlg m_edit_dlg;
	CRemoveDllEditDlg m_remove_dll_edit_dlg;

	void OnStopAndKillKazaa();
	void OnResumeKazaa();
	void OnRestartKazaa();
	void OnChangeMaxKazaa();
	void OnRestartComputer();
	void OnEmergencyRestart();
	void OnRemoveDll();

	LRESULT OnDoneChangingMaxKazaa(WPARAM wparam,LPARAM lparam);
	LRESULT OnDoneRemoveDllEdit(WPARAM wparam,LPARAM lparam);
	//LRESULT OnInitStatusDestThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT OnUpdateNetworkSummary(WPARAM wparam,LPARAM lparam);



public:
	void UpdateSummary();
	void UpdateStatus(string network, UINT total_computers, UINT total_computers_up, UINT bandwidth, 
			UINT cpu_usage, UINT upload_bandwidth, UINT download_bandwidth, UINT files_shared);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	void AddNetwork(string& network);
	void AddIPRange(CString network, CString ip_range);
	afx_msg void OnNMDblclkNetworkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickNetworkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickNetworkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRebootButton();
	afx_msg void OnLvnItemchangedNetworkList(NMHDR *pNMHDR, LRESULT *pResult);
};
