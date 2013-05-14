#pragma once
#include "resource.h"
//#include "BTSearcherDll.h"

// BTSearcherDlg dialog
class BTSearcherDll;

class BTSearcherDlg : public CDialog
{
	DECLARE_DYNAMIC(BTSearcherDlg)

public:
	BTSearcherDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTSearcherDlg();

// Dialog Data
	enum { IDD = IDD_BTSEARCHERDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();

	BTSearcherDll *p_dll;
public:
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	void Log(const char * buf);
	void SetDll(BTSearcherDll*dll);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk2();

	CListCtrl  m_main_list_ctrl;
};
