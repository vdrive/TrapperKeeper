#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "VendorCount.h"
#include "afxcmn.h"
#include "ProjectKeywords.h"

#define WM_SUPPLY_PROCESS_PROJECT								WM_USER+3
#define WM_DEMAND_PROCESS_PROJECT								WM_USER+4
#define WM_PROCESST_MANAGER_DONE								WM_USER+5
#define WM_INIT_PROCESS_MANAGER_THREAD_DATA						WM_USER+6
#define WM_INIT_DB_MANAGER_THREAD_DATA							WM_USER+7
#define WM_READY_TO_WRITE_DATA_TO_DB							WM_USER+8
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+9
#define WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE					WM_USER+10

// CPioletDCDlg dialog

class PioletDCDll;
class CPioletDCDlg : public CDialog
{
	DECLARE_DYNAMIC(CPioletDCDlg)

public:
	CPioletDCDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(PioletDCDll* parent);
	virtual ~CPioletDCDlg();

	void Log(char *buf);

	void IncrementCounters(unsigned int ping,unsigned int pong,unsigned int query,unsigned int query_hit,unsigned int unknown,unsigned int error_pong);
	void InitRunningSinceWindowText();
	void ReportVendorCounts(vector<VendorCount>* vc);
	void KeywordsUpdated(vector<ProjectKeywords> &keywords);
	void UpdateDataProcessProgressBar(int demand, int supply);

	unsigned int m_ping_count;
	unsigned int m_pong_count;
	unsigned int m_query_count;
	unsigned int m_query_hit_count;
	unsigned int m_unknown;
	unsigned int m_error_pong;

	// Dialog Data
	enum { IDD = IDD_PIOLET_DC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg LRESULT InitDBManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT DemandProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyProcessProject(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_status_list;

private:
	PioletDCDll* p_parent;
	UINT m_maximum_hosts;
	UINT m_maximum_hosts_cache;

public:
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();

private:
	void ReadMaximumHostsDataFromRegistry();
	void WriteMaximumHostsDataToRegistry();

public:
	afx_msg void OnBnClickedButton1();
private:
	CListCtrl m_vendor_list;
public:
	CTreeCtrl m_project_keyword_tree;
	CProgressCtrl m_demand_progress;
	CProgressCtrl m_supply_progress;
	afx_msg void OnBnClickedDataProcessButton();
};
