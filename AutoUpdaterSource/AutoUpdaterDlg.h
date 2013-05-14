#pragma once


// CAutoUpdaterDlg dialog

class CAutoUpdaterDlg : public CDialog
{
	DECLARE_DYNAMIC(CAutoUpdaterDlg)

public:
	CAutoUpdaterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAutoUpdaterDlg();

// Dialog Data
	enum { IDD = IDD_AutoUpdaterDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
