#pragma once
#include "AfxWin.h"


// CAddInfohashDlg dialog

class CBTTrackerDlg;
class CAddInfohashDlg
	: public CDialog
{
	DECLARE_DYNAMIC(CAddInfohashDlg)

public:
	CAddInfohashDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddInfohashDlg();
	void Init(CBTTrackerDlg *pMainDlg);

// Dialog Data
	enum { IDD = IDD_ADDTORRENT_DIALOG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

private:
	CBTTrackerDlg *m_pMainDlg;

	CEdit m_infohash;
	CEdit m_length;
};
