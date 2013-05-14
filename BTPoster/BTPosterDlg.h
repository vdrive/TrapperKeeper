#pragma once
#include "Resource.h"

// BTPosterDlg dialog

class BTPosterDlg : public CDialog
{
	DECLARE_DYNAMIC(BTPosterDlg)

public:
	BTPosterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTPosterDlg();

// Dialog Data
	enum { IDD = IDD_BTPOSTERDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedOk();
};
