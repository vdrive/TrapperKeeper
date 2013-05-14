#pragma once

#include "resource.h"

// CMediaMakerDlg dialog
class MediaManager;
class CMediaMakerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMediaMakerDlg)

public:
	CMediaMakerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaMakerDlg();

	void DisplayInfo(char * info);
	void InitParent(MediaManager * parent);

// Dialog Data
	enum { IDD = IDD_MEDIA_DLG };
	CListCtrl m_list_control;

	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	MediaManager * p_parent;


};
