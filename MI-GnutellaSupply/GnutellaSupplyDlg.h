#pragma once
#include "Resource.h"
#include "afxcmn.h"
#include "ConnectionModuleStatusData.h"
#include "VirtualProjectStatusListCtrl.h"
#include "ProjectKeywords.h"
//#include "SupplyProject.h"
#include "VendorCount.h"
#include "DlgResizer.h"

//#define WM_SUPPLY_MANAGER_THREAD_DATA_READY		WM_USER+1
//#define WM_DIST_MANAGER_THREAD_DATA_READY		WM_USER+2
//#define WM_INIT_LOGFILE_MANAGER_THREAD_DATA		WM_USER+3
//#define WM_INIT_REVERSE_DNS_MANAGER_THREAD_DATA	WM_USER+4
//#define WM_REVERSE_DNS_RESULTS					WM_USER+5
//#define WM_READY_TO_WRITE_LOGFILE_DATA_TO_FILE	WM_USER+6
#define WM_REVERSE_DNS_PROCESS_PROJECT							WM_USER+2
#define WM_SUPPLY_PROCESS_PROJECT								WM_USER+3
#define WM_DEMAND_PROCESS_PROJECT								WM_USER+4
#define WM_PROCESS_MANAGER_DEMAND_DONE							WM_USER+5
#define WM_INIT_PROCESS_MANAGER_DEMAND_THREAD_DATA				WM_USER+6
#define WM_INIT_DB_MANAGER_DEMAND_THREAD_DATA					WM_USER+7
#define WM_READY_TO_WRITE_DATA_TO_DEMAND_DB						WM_USER+8
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+9
#define WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE					WM_USER+10
#define WM_INIT_PROCESS_MANAGER_SUPPLY_THREAD_DATA				WM_USER+11
#define WM_PROCESS_MANAGER_SUPPLY_DONE							WM_USER+12
#define WM_INIT_PROCESS_MANAGER_REVERSE_DNS_THREAD_DATA			WM_USER+13
#define WM_PROCESS_MANAGER_REVERSE_DNS_DONE						WM_USER+14
#define WM_INIT_DB_MANAGER_SUPPLY_THREAD_DATA					WM_USER+15
#define WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB						WM_USER+16
#define WM_PROCESS_MANAGER_DB_MAINTENANCE_DELETED_HASHES		WM_USER+17
#define WM_PROCESS_MANAGER_DB_MAINTENANCE_INSERTED_HASHES		WM_USER+18
#define WM_MAINTENANCE_PROCESS_PROJECT							WM_USER+19
// CGnutellaSupplyDlg dialog

class GnutellaSupplyDll;
class CGnutellaSupplyDlg : public CDialog
{
	DECLARE_DYNAMIC(CGnutellaSupplyDlg)

public:
	CGnutellaSupplyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGnutellaSupplyDlg();
	void InitParent(GnutellaSupplyDll* parent);
	void ReportConnectionStatus(ConnectionModuleStatusData &status,vector<ConnectionModuleStatusData> *all_mod_status);
	void KeywordsUpdated(vector<ProjectKeywords> &keywords);
//	void SupplyUpdated(vector<SupplyProject> &supply_projects);
	void ClearProjectStatus(string &project);
	void UpdateLogfileCacheSize(unsigned int size);
	void AlterModuleCounts(int dmin,int dmax);
	void AlterSupplyIntervalMultiplier(int dmulti);
	void ModuleCountHasChanged(int count);
	void UpdateVendorCounts(vector<VendorCount> &vendor_counts);
	void UpdateDataMaintenanceProgressBar(int maintain);
	void UpdateDataProcessProgressBar(int demand, int supply, int dns);

//	afx_msg LRESULT SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	afx_msg LRESULT InitLogfileManagerThreadData(WPARAM wparam,LPARAM lparam);
//	afx_msg LRESULT ReadyToWriteLogfileDataToFile(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitDBManagerDemandThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToDemandDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerDemandThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerDemandDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT DemandProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT MaintenanceProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerReverseDNSThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerReverseDNSDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReverseDNSProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDeletedHashes(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceInsertedHashes(WPARAM wparam,LPARAM lparam);


	// Dialog Data
	enum { IDD = IDD_GnutellaSupply_Dialog };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	// Private Data Members
	GnutellaSupplyDll* p_parent;
	vector<int> v_searcher_socket_query_counts;
	CDlgResizer m_DlgResizer;

	// Private Member Functions
	void InitRunningSinceWindowText();
	void UpdateSupernodeVendorList(vector<ConnectionModuleStatusData> *status);

//	void UpdateSearcherQueryCounts(ConnectionModuleStatusData &status);

public:
	CListCtrl m_module_connection_list;
	CTreeCtrl m_project_keyword_tree;
//	CTreeCtrl m_project_supply_tree;
	VirtualProjectStatusListCtrl m_project_status_list;
	CListCtrl m_supernode_vendor_list;
	afx_msg void OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedShowLogWindow();
	afx_msg void OnBnClickedResetProjectStatusList();
//	afx_msg void OnBnClickedProjectSupplyTreeDetailsCheck();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	CProgressCtrl m_demand_progress;
	CProgressCtrl m_supply_progress;
	afx_msg void OnBnClickedDataProcessButton();
	afx_msg void OnDeltaposSupplyIntervalMultiplierSpin(NMHDR *pNMHDR, LRESULT *pResult);
	CProgressCtrl m_reverse_dns_progress;
private:
	BOOL m_vendor_counts_enabled;
public:
	afx_msg void OnBnClickedVendorCountsCheck();
protected:
	CProgressCtrl m_maintenance_progress;
};
