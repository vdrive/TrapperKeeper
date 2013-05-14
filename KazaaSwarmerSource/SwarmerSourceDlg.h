#pragma once

#include "resource.h"

// SwarmerSourceDlg dialog
class SwarmerSource;
class SwarmerSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(SwarmerSourceDlg)

public:
	SwarmerSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SwarmerSourceDlg();

	void InitParent(SwarmerSource* parent);

	afx_msg void OnTimer(UINT nIDEvent);
	void DisplayInfo(char * info);

// Dialog Data
	enum { IDD = IDD_KAZAASWARMER_DLG  };
	CListCtrl m_list_control;

	afx_msg void OnBnClickedGetList();
	afx_msg void OnBnClickedSynch();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	SwarmerSource * p_parent;
};


