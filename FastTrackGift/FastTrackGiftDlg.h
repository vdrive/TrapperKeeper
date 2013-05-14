#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "afxcmn.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include "ProjectKeywords.h"
#include "VirtualProjectStatusListCtrl.h"
#include "FileTransferDlg.h"
#include "NoiseModuleThreadStatusData.h"

#define WM_INIT_DB_MANAGER_SUPPLY_THREAD_DATA					WM_USER+5
#define WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB						WM_USER+6
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+11
#define WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE					WM_USER+12
#define WM_INIT_PROCESS_MANAGER_SUPPLY_THREAD_DATA				WM_USER+13
#define WM_PROCESS_MANAGER_SUPPLY_DONE							WM_USER+14
#define WM_SUPPLY_PROCESS_PROJECT								WM_USER+15
#define WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA				WM_USER+16
#define WM_FILE_SHARING_MANAGER_THREAD_PROGRESS					WM_USER+17
#define WM_FILE_SHARING_MANAGER_THREAD_PROJECT					WM_USER+18
#define WM_FILE_SHARING_MANAGER_UPDATE_SUPPLY_PROJECTS			WM_USER+19
#define WM_FILE_SHARING_MANAGER_SUPPLY_RETREIVAL_FAILED			WM_USER+20

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

	afx_msg LRESULT InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT UpdateFileSharingManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerThreadProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerThreadProgress(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT FileSharingManagerSupplyRetrievalFailed(WPARAM wparam,LPARAM lparam);


	void ReportConnectionStatus(ConnectionModuleStatusData &status);//,vector<ConnectionModuleStatusData> *all_mod_status);
	void ModuleCountHasChanged(int count);
	void UpdateVendorCounts(vector<VendorCount> &vendor_counts);
	void KeywordsUpdated(vector<ProjectKeywords>* keywords);
	void ClearProjectStatus(string &project);
	int GetModCount();
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);
	void SpoofEntriesRetrievalStarted();
	void AutoPause(bool pause);

// Dialog Data
	enum { IDD = IDD_FASTTRACK_GIFT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListBox m_log_list;
	FastTrackGiftDll* p_parent;
	CListCtrl m_module_connection_list;
	CTreeCtrl m_project_keyword_tree;
	CListCtrl m_supernode_vendor_list;
	VirtualProjectStatusListCtrl m_project_status_list;
	CFileTransferDlg m_ft_dlg;
	bool m_pause_searching;

	void AlterModuleCounts(int dmin,int dmax);
	void InitRunningSinceWindowText();
	void AlterSupplyIntervalMultiplier(int dmulti);

public:
	afx_msg void OnBnClickedResetProjectStatusList();
	afx_msg void OnDeltaposSupplyIntervalMultiplierSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSaveReportButton();
	afx_msg void OnBnClickedRestartSearchingButton();
	afx_msg void OnBnClickedFileTransferButton();
	afx_msg void OnBnClickedProcessButton();
	void UpdateDataProcessProgressBar(int supply);
	void UpdateSpoofSize(UINT spoof_size, UINT temp_spoof_size, UINT spoof_index, UINT round, UINT hash_size);

private:
	CProgressCtrl m_supply_progress;
	CProgressCtrl m_supply_entry_progress;
	int m_auto_pause_threshold;
	int m_auto_resume_threshold;
public:
	afx_msg void OnBnClickedReconnectStatic();
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedApplyAutoPauseThresholdButton();
private:
	BOOL m_vendor_counts_enabled;
public:
	afx_msg void OnBnClickedVendorCountsCheck();
};
