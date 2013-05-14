// MovieMakerDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "MovieMaker.h"
#include "MovieMakerDlg.h"


// CMovieMakerDlg dialog

IMPLEMENT_DYNAMIC(CMovieMakerDlg, CDialog)
CMovieMakerDlg::CMovieMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieMakerDlg::IDD, pParent)
{
}

CMovieMakerDlg::~CMovieMakerDlg()
{
}

void CMovieMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMovieMakerDlg, CDialog)
END_MESSAGE_MAP()


// CMovieMakerDlg message handlers
