#pragma once
#include "resource.h"

// CNewSourceDialog dialog

class CNewSourceDialog : public CDialog
{
	DECLARE_DYNAMIC(CNewSourceDialog)

public:
	CNewSourceDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewSourceDialog();

// Dialog Data
	enum { IDD = IDD_NEWSOURCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_source_name;
};
