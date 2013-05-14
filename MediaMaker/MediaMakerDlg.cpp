// MediaMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaMakerDlg.h"
#include "MediaManager.h"


// CMediaMakerDlg dialog

IMPLEMENT_DYNAMIC(CMediaMakerDlg, CDialog)
CMediaMakerDlg::CMediaMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaMakerDlg::IDD, pParent)
{
}

CMediaMakerDlg::~CMediaMakerDlg()
{
}

void CMediaMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIA_LIST, m_list_control);
}


BEGIN_MESSAGE_MAP(CMediaMakerDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMediaMakerDlg message handlers
BOOL CMediaMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	return true;
}


void CMediaMakerDlg::InitParent(MediaManager * parent)
{
	p_parent = parent;
}

// Display whatever info you receive
void CMediaMakerDlg::DisplayInfo(char * info)
{
	int index = m_list_control.GetItemCount();
	m_list_control.InsertItem(index,info);
}
void CMediaMakerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
}

void CMediaMakerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();

}


void CMediaMakerDlg::OnTimer(UINT nIDEvent)
{

	switch (nIDEvent)
	{
		case (1):
			{
				p_parent->TimerFired();


			}

	}
}