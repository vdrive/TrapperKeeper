#pragma once

#include "resource.h"
// MediaSwarmerDlg dialog

class MediaSwarmer;
class MediaSwarmerDlg : public CDialog
{
	DECLARE_DYNAMIC(MediaSwarmerDlg)

public:
	MediaSwarmerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MediaSwarmerDlg();

	void InitParent(MediaSwarmer * parent);

	afx_msg void OnTimer(UINT nIDEvent);

// Dialog Data
	enum { IDD = IDD_MEDIASWARMER_DEST_DLG };
	CListCtrl m_list_control;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	MediaSwarmer * p_parent;
public:
	afx_msg void OnDestroy();
};
