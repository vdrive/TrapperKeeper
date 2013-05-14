#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "ConnectionModuleStatusData.h"
#include "VirtualProjectStatusListCtrl.h"
#include "ProjectKeywords.h"
#include "SupplyProject.h"
#include "VendorCount.h"
#include "DlgResizer.h"

//#define WM_SUPPLY_MANAGER_THREAD_DATA_READY		WM_USER+1
//#define WM_DIST_MANAGER_THREAD_DATA_READY		WM_USER+2
//#define WM_INIT_LOGFILE_MANAGER_THREAD_DATA		WM_USER+3
//#define WM_INIT_REVERSE_DNS_MANAGER_THREAD_DATA	WM_USER+4
//#define WM_REVERSE_DNS_RESULTS					WM_USER+5
//#define WM_READY_TO_WRITE_LOGFILE_DATA_TO_FILE	WM_USER+6
#define WM_INIT_SUPPLY_MANAGER_THREAD_DATA						WM_USER+2
#define WM_SUPPLY_PROCESS_PROJECT								WM_USER+3
#define WM_DEMAND_PROCESS_PROJECT								WM_USER+4
#define WM_PROCESST_MANAGER_DONE								WM_USER+5
#define WM_INIT_PROCESS_MANAGER_THREAD_DATA						WM_USER+6
#define WM_INIT_DB_MANAGER_THREAD_DATA							WM_USER+7
#define WM_READY_TO_WRITE_DATA_TO_DB							WM_USER+8
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+9
#define WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE					WM_USER+10
#define WM_SUPPLY_MANAGER_THREAD_PROJECT						WM_USER+11
#define WM_SUPPLY_MANAGER_THREAD_PROGRESS						WM_USER+12
#define WM_UPDATE_SUPPLY_MANAGER_SUPPLY_PROJECTS				WM_USER+13
#define WM_SUPPLY_MANAGER_RETRIEVING_SUPPLY_ABORTED				WM_USER+14

// CGnutellaProtectorDlg dialog
class GnutellaProtectorDll;
class CGnutellaProtectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CGnutellaProtectorDlg)

public:
	CGnutellaProtectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGnutellaProtectorDlg();
	void InitParent(GnutellaProtectorDll* parent);
	void ReportConnectionStatus(ConnectionModuleStatusData &status,vector<ConnectionModuleStatusData> *all_mod_status);
	void KeywordsUpdated(vector<ProjectKeywords> &keywords);
//	void SupplyUpdated(vector<SupplyProject> &supply_projects);
	void ClearProjectStatus(string &project);
	void UpdateLogfileCacheSize(unsigned int size);
	void AlterModuleCounts(int dmin,int dmax);
	void ModuleCountHasChanged(int count);
	void UpdateVendorCounts(vector<VendorCount> &vendor_counts, int compression_on_counts);
	void UpdateDataProcessProgressBar(int demand);
	void SpoofEntriesRetrievalStarted();

//	afx_msg LRESULT SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	afx_msg LRESULT InitLogfileManagerThreadData(WPARAM wparam,LPARAM lparam);
//	afx_msg LRESULT ReadyToWriteLogfileDataToFile(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitDBManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT DemandProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadProgress(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadAborted(WPARAM wparam,LPARAM lparam);

// Dialog Data
	enum { IDD = IDD_GNUTELLA_PROTECTOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	// Private Data Members
	GnutellaProtectorDll* p_parent;
	vector<int> v_searcher_socket_query_counts;
	CDlgResizer m_DlgResizer;

	// Private Member Functions
	void InitRunningSinceWindowText();
	void UpdateSupernodeVendorList(vector<ConnectionModuleStatusData> *status);

//	void UpdateSearcherQueryCounts(ConnectionModuleStatusData &status);
public:
	CListCtrl m_module_connection_list;
	CTreeCtrl m_project_keyword_tree;
	VirtualProjectStatusListCtrl m_project_status_list;
	CListCtrl m_supernode_vendor_list;
	afx_msg void OnBnClickedShowLogWindow();
	afx_msg void OnBnClickedResetProjectStatusList();
	afx_msg void OnBnClickedDataProcessButton();
	CProgressCtrl m_spoof_progress;
	CProgressCtrl m_supply_progress;
	afx_msg void OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedRetrieveButton();
private:
	CString m_add_host;
public:
	afx_msg void OnBnClickedAddHostButton();
private:
	BOOL m_vendor_counts_enabled;
public:
	afx_msg void OnBnClickedVendorCountsCheck();
};
