//////////////////////////////////////////////////////////////////////////////
// Author : Jerry Trinh
// Date   : March | 03 | 03
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "resource.h"
#include "afxcmn.h"
#include <vector>
#include "Rack.h"

#define SUB_WDC_NAME	0
#define SUB_WDC_STATUS	1
using namespace std;

// CWatchDogControllerSourceDlg dialog
class WatchDogControllerSourceDll;
class CWatchDogControllerSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CWatchDogControllerSourceDlg)

public:
	CWatchDogControllerSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWatchDogControllerSourceDlg();
	CListCtrl m_list_control;

// Dialog Data
	enum { IDD = IDD_WDCS_DLG };

protected:
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	// member variable	
	int m_count;
	CTime m_time;
	CString m_running_since;
	WatchDogControllerSourceDll *p_parent;
public:
	// Routines:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void InitParent(WatchDogControllerSourceDll * parent);
	virtual BOOL OnInitDialog(void);
	afx_msg void OnTimer(UINT nIDEvent);
	void AddDestIP(char* name);
	void AddNewDestIP(char* new_name);
	void ReturnStatus(char * ip, int result);
	void ConnectingStatus();
	void DeleteAllRacks(void);
	void TotalUpRacks(int);
	void TotalDownRacks(int);
	void TimeToPing(vector<Rack> &v_racks);
	void TimeFromPong(vector<Rack> &v_racks, char *new_ip);
	void ResetTitle(void);
	void RunningSince();
	void TotalWatchRacks();
	void UpdateList(void);
	void SetSortArrow(const int i_sort_column, const BOOL b_sort_ascending);
	void Sort(int i_column, bool b_ascending);
	afx_msg void OnLvnColumnclickListControl(NMHDR *pNMHDR, LRESULT *pResult);
};
