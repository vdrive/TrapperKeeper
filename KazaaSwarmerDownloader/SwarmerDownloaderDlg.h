#pragma once
#include "resource.h"

// SwarmerDownloaderDlg dialog

class SwarmerDownloader;
class SwarmerDownloaderDlg : public CDialog
{
	DECLARE_DYNAMIC(SwarmerDownloaderDlg)

public:
	SwarmerDownloaderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SwarmerDownloaderDlg();

	void InitParent(SwarmerDownloader * parent);


	afx_msg void OnTimer(UINT nIDEvent);
	void DisplayInfo(char * info);
	CListCtrl m_list_control;
	// Dialog Data
	enum { IDD = IDD_DOWNLOADER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	SwarmerDownloader * p_parent;
};
