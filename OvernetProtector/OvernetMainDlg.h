#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "ConnectionModuleStatusData.h"
#include "afxcmn.h"

// COvernetMainDlg dialog
class OvernetProtectorDll;
class COvernetMainDlg : public CDialog
{
	DECLARE_DYNAMIC(COvernetMainDlg)

public:
	COvernetMainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COvernetMainDlg();
	void InitParent(OvernetProtectorDll* parent);
	void Log(const char *buf);
	void StatusReady(ConnectionModuleStatusData& status);
	void SpoofEntriesRetrievalStarted();
	void RetrievingProject(const char* status,int percent);
	void DoneRetrievingProject();


// Dialog Data
	enum { IDD = IDD_Overnet_Main_DIALOG };

private:
	OvernetProtectorDll* p_parent;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
protected:
	CListBox m_log_list;
	UINT m_num_results;
public:
	virtual BOOL OnInitDialog();
protected:
	CProgressCtrl m_supply_entry_progress;
};
