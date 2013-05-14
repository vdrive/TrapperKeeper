#include "Resource.h"
#include "GnutellaFileTransferDlg.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include "NoiseModuleThreadStatusData.h"
#pragma once

#define SUB_MOD								0
#define SUB_CONNECTED_CONNECTING_IDLE		1
#define SUB_PING_PONG_PUSH_QUERY_QUERYHIT	2

#define SUB_SNV_NUM		0
#define SUB_SNV_VENDOR	1

#define WM_FILE_SHARING_MANAGER_THREAD_PROGRESS					WM_USER+14
#define	WM_FILE_SHARING_MANAGER_UPDATE_SHARED_FILES_DONE		WM_USER+15
#define WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA				WM_USER+16


// CGnutellaDecoyerDlg dialog
class GnutellaDecoyerDll;
class CGnutellaDecoyerDlg : public CDialog
{
	DECLARE_DYNAMIC(CGnutellaDecoyerDlg)

public:
	CGnutellaDecoyerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGnutellaDecoyerDlg();

// Dialog Data
	enum { IDD = IDD_GNUTELLA_DECOYER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedReconnectStatic();
	afx_msg void OnBnClickedFileTransferButton();
	afx_msg void OnBnClickedDisableSharingButton();
	afx_msg void OnBnClickedEnableSharingButton();
	afx_msg void OnBnClickedRescanSharedButton();
	afx_msg void OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedVendorCountsCheck();
	afx_msg LRESULT InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerUpdateSharedFilesDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT RebuildSharingDBStarted(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT RebuildSharingDBEnded(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerThreadProgress(WPARAM wparam,LPARAM lparam);
	virtual BOOL OnInitDialog();
	void InitParent(GnutellaDecoyerDll* parent);
	void Log(const char* log);
	void ModuleCountHasChanged(int count);
	void ReportConnectionStatus(ConnectionModuleStatusData &status);
	void UpdateVendorCounts(vector<VendorCount> &vendor_counts);
	void UpdateSpoofSize(UINT spoof_size, UINT temp_spoof_size, UINT hash_size);
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);
	int GetModCount();

	afx_msg void OnTimer(UINT nIDEvent);

private:
	void AlterModuleCounts(int dmin,int dmax);
	void InitRunningSinceWindowText();

	GnutellaDecoyerDll* p_parent;
	CGnutellaFileTransferDlg m_ft_dlg;
	CListBox m_log_list;
	CListCtrl m_module_connection_list;
	CListCtrl m_supernode_vendor_list;
	BOOL m_vendor_counts_enabled;
};
