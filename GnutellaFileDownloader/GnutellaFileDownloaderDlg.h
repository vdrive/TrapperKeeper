#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "DownloadingSocket.h"


// GnutellaFileDownloaderDlg dialog

class GnutellaFileDownloaderDlg : public CDialog
{
	DECLARE_DYNAMIC(GnutellaFileDownloaderDlg)

public:
	GnutellaFileDownloaderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~GnutellaFileDownloaderDlg();
	void Status(char *status);
	void Status(CString status);

// Dialog Data
	enum { IDD = m_dialog };

protected:
	DownloadingSocket *dl;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CListCtrl m_log;
};
