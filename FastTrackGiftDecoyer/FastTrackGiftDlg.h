#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "afxcmn.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include "ProjectKeywords.h"
#include "FileTransferDlg.h"
#include "NoiseModuleThreadStatusData.h"

#define WM_FILE_SHARING_MANAGER_THREAD_PROGRESS					WM_USER+14
#define	WM_FILE_SHARING_MANAGER_UPDATE_SHARED_FILES_DONE		WM_USER+15
#define WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA				WM_USER+16

// CFastTrackGiftDlg dialog
class FastTrackGiftDll;
class CFastTrackGiftDlg : public CDialog
{
	DECLARE_DYNAMIC(CFastTrackGiftDlg)

public:
	CFastTrackGiftDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFastTrackGiftDlg();
	void Log(const char* log);
	virtual BOOL OnInitDialog();
	void InitParent(FastTrackGiftDll* parent);
	afx_msg void OnBnClickedSearchButton();
	afx_msg void OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg LRESULT InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerUpdateSharedFilesDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT RebuildSharingDBStarted(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT RebuildSharingDBEnded(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerThreadProgress(WPARAM wparam,LPARAM lparam);

	void ReportConnectionStatus(ConnectionModuleStatusData &status);//,vector<ConnectionModuleStatusData> *all_mod_status);
	void ModuleCountHasChanged(int count);
	void UpdateVendorCounts(vector<VendorCount> &vendor_counts);
	int GetModCount();
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);

// Dialog Data
	enum { IDD = IDD_FASTTRACK_GIFT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListBox m_log_list;
	FastTrackGiftDll* p_parent;
	CListCtrl m_module_connection_list;
	CListCtrl m_supernode_vendor_list;
	CFileTransferDlg m_ft_dlg;
	BOOL m_vendor_counts_enabled;

	void AlterModuleCounts(int dmin,int dmax);
	void InitRunningSinceWindowText();
	void AlterSupplyIntervalMultiplier(int dmulti);

public:
	afx_msg void OnBnClickedFileTransferButton();
	afx_msg void OnBnClickedVendorCountsCheck();
	afx_msg void OnBnClickedReconnectStatic();
	void UpdateSpoofSize(UINT spoof_size, UINT temp_spoof_size, UINT spoof_index, UINT round, UINT hash_size);
	afx_msg void OnBnClickedDisableSharingButton();
	afx_msg void OnBnClickedEnableSharingButton();
	afx_msg void OnBnClickedRescanSharedButton();
};
