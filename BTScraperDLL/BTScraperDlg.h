#pragma once

#include "resource.h"
#include "afxcmn.h"

#define MAIN_MESSAGE 1
#define RESEND_MESSAGE 2
#define RECHECK_MESSAGE 3

// BTScraperDlg dialog
class BTScraperMod;

class BTScraperDlg : public CDialog
{
	DECLARE_DYNAMIC(BTScraperDlg)

public:
	BTScraperDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTScraperDlg();

	void Log(const char *buf, int type=MAIN_MESSAGE);

	void InitParent(BTScraperMod *parent);

	// Dialog Data
	enum { IDD = IDD_BTSCRAPERDLG };

	void OnTimer(UINT nIDEvent);

	bool SendReliableData(char * dest, void* send_data, UINT data_length) ;
	CListCtrl  m_main_list_ctrl;
	CListCtrl  m_resend_list_ctrl;
	CListCtrl  m_recheck_list_ctrl;

protected:
	BTScraperMod * p_dll;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnLvnItemchangedList4(NMHDR *pNMHDR, LRESULT *pResult);
};
