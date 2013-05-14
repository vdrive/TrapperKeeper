#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "afxcmn.h"
#include "ConnectionModuleStatusData.h"
#include "ProjectKeywords.h"
#include "VirtualProjectStatusListCtrl.h"
#include "WinMXFileTransferDlg.h"

#define WM_INIT_DB_MANAGER_SUPPLY_THREAD_DATA					WM_USER+5
#define WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB						WM_USER+6
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+11
#define WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE					WM_USER+12
#define WM_INIT_PROCESS_MANAGER_SUPPLY_THREAD_DATA				WM_USER+13
#define WM_PROCESS_MANAGER_SUPPLY_DONE							WM_USER+14
#define WM_SUPPLY_PROCESS_PROJECT								WM_USER+15
#define WM_INIT_SUPPLY_MANAGER_THREAD_DATA						WM_USER+16
#define WM_SUPPLY_MANAGER_THREAD_PROGRESS					WM_USER+17
#define WM_SUPPLY_MANAGER_THREAD_PROJECT					WM_USER+18
#define WM_SUPPLY_MANAGER_UPDATE_SUPPLY_PROJECTS			WM_USER+19
#define WM_SUPPLY_MANAGER_SUPPLY_RETREIVAL_FAILED			WM_USER+20

// CWinMxDcDlg dialog
class WinMxDcDll;
class CWinMxDcDlg : public CDialog
{
	DECLARE_DYNAMIC(CWinMxDcDlg)

public:
	CWinMxDcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWinMxDcDlg();
	void Log(const char* log);
	void InitParent(WinMxDcDll* parent);
	afx_msg LRESULT InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyanagerThreadProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadProgress(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerSupplyRetrievalFailed(WPARAM wparam,LPARAM lparam);

	void ReportConnectionStatus(ConnectionModuleStatusData &status);//,vector<ConnectionModuleStatusData> *all_mod_status);
	void ModuleCountHasChanged(int count);
	void KeywordsUpdated(vector<ProjectKeywords>* keywords);
	void ClearProjectStatus(string &project);
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void AutoPause(bool pause);
	void UpdateDataProcessProgressBar(int supply);
	void SpoofEntriesRetrievalStarted();
	void UpdateSpoofSize(UINT spoof_size,UINT spoof_index, UINT round, UINT hash_size);
	int GetModCount();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);
	void OnUnInitialize();

// Dialog Data
	enum { IDD = IDD_WINMX_DC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedReconnectStatic();
	afx_msg void OnBnClickedResetProjectStatusList();
	afx_msg void OnBnClickedRestartSearchingButton();
	afx_msg void OnBnClickedProcessButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSupplyIntervalMultiplierSpin(NMHDR *pNMHDR, LRESULT *pResult);

private:
	CListBox m_log_list;
	WinMxDcDll* p_parent;
	CListCtrl m_module_connection_list;
	CTreeCtrl m_project_keyword_tree;
	CListCtrl m_supernode_vendor_list;
	VirtualProjectStatusListCtrl m_project_status_list;
	bool m_pause_searching;
	CProgressCtrl m_supply_progress;
	int m_auto_pause_threshold;
	int m_auto_resume_threshold;

	void AlterModuleCounts(int dmin,int dmax);
	void InitRunningSinceWindowText();
	void AlterSupplyIntervalMultiplier(int dmulti);
public:
	afx_msg void OnBnClickedApplyAutoPauseThresholdButton();
private:
	CProgressCtrl m_supply_entry_progress;
	CWinMXFileTransferDlg m_ft_dlg;
public:
	afx_msg void OnBnClickedFileTransferButton();
};
