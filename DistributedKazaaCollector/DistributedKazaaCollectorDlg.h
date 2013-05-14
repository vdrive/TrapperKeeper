#pragma once
#include "resource.h"
#include "afxwin.h"

// CDistributedKazaaCollectorDlg dialog

class DistributedKazaaCollectorDll;

class CDistributedKazaaCollectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CDistributedKazaaCollectorDlg)

public:
	CDistributedKazaaCollectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDistributedKazaaCollectorDlg();

	
	void Log(const char *buf);
	void CheckForErrorBoxes();

	void InitParent(DistributedKazaaCollectorDll *parent);
	DistributedKazaaCollectorDll *p_dll;

// Dialog Data
	enum { IDD = IDD_DIALOG };
	CListCtrl m_project_list;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnGo();
	CListBox m_list_box;
	afx_msg void OnBnClickedClear();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedOnTopCheck();
	CEdit m_search_more_interval;

	UINT m_worker_iter;
	CTime m_last_update;
};
