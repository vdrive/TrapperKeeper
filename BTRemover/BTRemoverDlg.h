#pragma once
#include "resource.h"
//#include "BTRemoverDll.h"

// BTRemoverDlg dialog
class BTRemoverDll;

class BTRemoverDlg : public CDialog
{
	DECLARE_DYNAMIC(BTRemoverDlg)

public:
	BTRemoverDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTRemoverDlg();

// Dialog Data
	enum { IDD = IDD_BTREMOVERDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();

	BTRemoverDll *p_dll;
public:
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	void Log(const char * buf);
	void SetDll(BTRemoverDll*dll);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOk2();

	CListCtrl  m_main_list_ctrl;
};
