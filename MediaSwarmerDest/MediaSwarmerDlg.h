#pragma once

#include "resource.h"
// MediaSwarmerDlg dialog

class MediaSwarmerDlg : public CDialog
{
	DECLARE_DYNAMIC(MediaSwarmerDlg)

public:
	MediaSwarmerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MediaSwarmerDlg();

// Dialog Data
	enum { IDD = IDD_MEDIASWARMER_DEST_DLG };
	CListCtrl m_list_control;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
