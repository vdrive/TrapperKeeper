// BTTorrentGeneratorDBInterfaceDlg.h : header file
//

#pragma once
#include "TabSelection.h"
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"


// CBTTorrentGeneratorDBInterfaceDlg dialog
class CBTTorrentGeneratorDBInterfaceDlg
	: public CDialog
{
	// Construction
public:
	CBTTorrentGeneratorDBInterfaceDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	enum { IDD = IDD_MAINFRAME_DLG };

protected:
	void AutoSizeAdjust(void);

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

protected:
	CTabSelection m_mainTab;
};
