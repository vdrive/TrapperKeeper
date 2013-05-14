#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "Resource.h"
#include "VendorCount.h"
#include "ProjectKeywords.h"

#define WM_SUPPLY_MANAGER_THREAD_PROJECT						WM_USER+1
#define WM_SUPPLY_MANAGER_THREAD_PROGRESS						WM_USER+2
#define WM_SPOOF_PROCESS_PROJECT								WM_USER+3
#define WM_INIT_SUPPLY_MANAGER_THREAD_DATA						WM_USER+4
#define WM_PROCESST_MANAGER_DONE								WM_USER+5
#define WM_INIT_PROCESS_MANAGER_THREAD_DATA						WM_USER+6
#define WM_INIT_DB_MANAGER_THREAD_DATA							WM_USER+7
#define WM_READY_TO_WRITE_DATA_TO_DB							WM_USER+8
#define WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA		WM_USER+9
#define WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE					WM_USER+10
#define WM_UPDATE_SUPPLY_MANAGER_SUPPLY_PROJECTS				WM_USER+11

class PioletSpooferDll;

// CPioletSpooferDlg dialog

class CPioletSpooferDlg : public CDialog
{
	DECLARE_DYNAMIC(CPioletSpooferDlg)

public:
	CPioletSpooferDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(PioletSpooferDll* parent);
	virtual ~CPioletSpooferDlg();
	
	void Log(const char *text,COLORREF color=0xFFFFFFFF,bool bold=false,bool italic=false);

	void IncrementCounters(unsigned int fordwarded_query,unsigned int query,unsigned int query_matched,unsigned int spoof_sent,unsigned int unknown,unsigned int error_pong);
	void InitRunningSinceWindowText();
	void ReportVendorCounts(vector<VendorCount>* vc);
	void KeywordsUpdated(vector<ProjectKeywords> &keywords);
	void UpdateDataProcessProgressBar(int spoof);
	void SpoofEntriesRetrievalStarted();


	unsigned int m_forwarded_query_count;
	unsigned int m_qurey_matched_count;
	unsigned int m_query_count;
	unsigned int m_spoof_sent_count;
	unsigned int m_unknown;
	unsigned int m_error_pong;

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT InitDBManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SpoofProcessProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadProject(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SupplyManagerThreadProgress(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_project_keyword_tree;
	CProgressCtrl m_spoof_progress;
	CListCtrl m_vendor_list;
	int m_maximum_hosts;
	int m_maximum_hosts_cache;
	afx_msg void OnBnClickedDataProcessButton();
	afx_msg void OnBnClickedButton1();

private:
	void WriteMaximumHostsDataToRegistry();
	void ReadMaximumHostsDataFromRegistry();

	bool m_newline;
	PioletSpooferDll* p_parent;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	CProgressCtrl m_spoof_entries_progress;
	CRichEditCtrl m_status_list;
	afx_msg void OnBnClickedResetPoisonerButton();
};
