#pragma once
#include "Resource.h"

// CBTinterdictorDlg dialog

class CBTinterdictorDlg : public CDialog
{
	DECLARE_DYNAMIC(CBTinterdictorDlg)

public:
	CBTinterdictorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBTinterdictorDlg();
	void OnCancel();

// Dialog Data
	enum { IDD = IDD_SamplePlusIn_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedKillUsAll();
};
