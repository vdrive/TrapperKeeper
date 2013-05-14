#pragma once
#include "resource.h"
#include "afxwin.h"

// CWatchDogControllerDestinationDlg dialog
class WatchDogControllerDestinationDll;
class CWatchDogControllerDestinationDlg : public CDialog
{
	DECLARE_DYNAMIC(CWatchDogControllerDestinationDlg)

public:
	CWatchDogControllerDestinationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWatchDogControllerDestinationDlg();

// Dialog Data
	enum { IDD = IDD_WDCD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void InitParent(WatchDogControllerDestinationDll * parent);
private:	
	WatchDogControllerDestinationDll * p_parent;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	void ReturnMessage(char *ip);
	BOOL OnInitDialog(void);	
	afx_msg void OnLbnSelchangeList();
	CListBox m_list_box;
};
