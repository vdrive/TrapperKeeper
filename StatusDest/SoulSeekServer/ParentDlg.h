#pragma once
#include "Resource.h"

// ParentDlg dialog

class CParentDlg : public CDialog
{
	DECLARE_DYNAMIC(CParentDlg)

public:
	CParentDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParentDlg();

// Dialog Data
	enum { IDD = IDD_PARENT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
