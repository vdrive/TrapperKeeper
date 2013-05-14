// ExportRackInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExportRackInfoDlg.h"
#include "NetworkDlg.h"


// CExportRackInfoDlg dialog

IMPLEMENT_DYNAMIC(CExportRackInfoDlg, CDialog)
CExportRackInfoDlg::CExportRackInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportRackInfoDlg::IDD, pParent)
	, m_mac_address_checked(FALSE)
	, m_ip_checked(FALSE)
	, m_cpu_checked(FALSE)
	, m_memory_checked(FALSE)
	, m_hdd_checked(FALSE)
	, m_os_checked(FALSE)
	, m_computer_name(FALSE)
	, m_network(FALSE)
{
}

CExportRackInfoDlg::~CExportRackInfoDlg()
{
}

void CExportRackInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK2, m_mac_address_checked);
	DDX_Check(pDX, IDC_CHECK1, m_ip_checked);
	DDX_Check(pDX, IDC_CHECK5, m_cpu_checked);
	DDX_Check(pDX, IDC_CHECK3, m_memory_checked);
	DDX_Check(pDX, IDC_CHECK4, m_hdd_checked);
	DDX_Check(pDX, IDC_CHECK6, m_os_checked);
	DDX_Check(pDX, IDC_CHECK7, m_computer_name);
	DDX_Check(pDX, IDC_CHECK8, m_network);
}


BEGIN_MESSAGE_MAP(CExportRackInfoDlg, CDialog)
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, OnBnClickedExportButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnBnClickedCancelButton)
END_MESSAGE_MAP()


//
//
//
void CExportRackInfoDlg::InitParent(CNetworkDlg* parent)
{
	p_parent = parent;
}

//
//
//
// CExportRackInfoDlg message handlers

void CExportRackInfoDlg::OnBnClickedExportButton()
{
	UpdateData(TRUE);
	p_parent->OnExportRackInfo();
}

//
//
//
void CExportRackInfoDlg::OnBnClickedCancelButton()
{
	p_parent->KillExportDlg();
}

//
//
//
BOOL CExportRackInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_mac_address_checked=true;
	m_ip_checked=true;
	m_cpu_checked=true;
	m_memory_checked=true;
	m_hdd_checked=true;
	m_os_checked=true;
	m_computer_name=true;
	m_network = true;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
