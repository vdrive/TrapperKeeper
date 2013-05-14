#pragma once

#include "resource.h"

// SwarmerSourceDlg dialog

class SwarmerSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(SwarmerSourceDlg)

public:
	SwarmerSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SwarmerSourceDlg();

	void OnTimer(UINT nIDEvent);

// Dialog Data
	enum { IDD = IDD_MEDIASWARMER_SOURCE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()
};
