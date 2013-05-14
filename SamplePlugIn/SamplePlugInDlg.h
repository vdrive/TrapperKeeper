#pragma once
#include "Resource.h"

// CSamplePlugInDlg dialog

class CSamplePlugInDlg : public CDialog
{
	DECLARE_DYNAMIC(CSamplePlugInDlg)

public:
	CSamplePlugInDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSamplePlugInDlg();
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
