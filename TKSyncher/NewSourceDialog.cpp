// NewSourceDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NewSourceDialog.h"


// CNewSourceDialog dialog

IMPLEMENT_DYNAMIC(CNewSourceDialog, CDialog)
CNewSourceDialog::CNewSourceDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNewSourceDialog::IDD, pParent)
	, m_source_name(_T(""))
{
}

CNewSourceDialog::~CNewSourceDialog()
{
}

void CNewSourceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SOURCE_NAME, m_source_name);
}


BEGIN_MESSAGE_MAP(CNewSourceDialog, CDialog)
END_MESSAGE_MAP()


// CNewSourceDialog message handlers
