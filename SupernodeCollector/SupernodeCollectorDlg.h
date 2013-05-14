#pragma once
#include "afxwin.h"
#include "Resource.h"


// CSupernodeCollectorDlg dialog
class SupernodeCollectorDll;
class CSupernodeCollectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CSupernodeCollectorDlg)

public:
	CSupernodeCollectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSupernodeCollectorDlg();
	void InitParent(SupernodeCollectorDll* parent);
	void UpdateConnectionStatus(int connecting, int idle, int timeout, int queue_connecting);
	void UpdateSupernodesStatus(UINT supernodes, UINT queue_size);
	void Log(CString msg);



// Dialog Data
	enum { IDD = IDD_SC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_log_list;

private:
	SupernodeCollectorDll* p_parent;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedResetButton();
};
