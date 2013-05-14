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

	void OnTimer(UINT nIDEvent);

// Dialog Data
	enum { IDD = IDD_MEDIASWARMER_DLG  };
	CListCtrl m_list_control;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	SwarmerSource * p_parent;
};