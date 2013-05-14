// SwarmerSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SwarmersourceDll.h"
#include "SwarmerSourceDlg.h"


// CSwarmerSourceDlg dialog

IMPLEMENT_DYNAMIC(CSwarmerSourceDlg, CDialog)
CSwarmerSourceDlg::CSwarmerSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSwarmerSourceDlg::IDD, pParent)
{
}

CSwarmerSourceDlg::~CSwarmerSourceDlg()
{
}

void CSwarmerSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCELIST, m_list_control);
}


BEGIN_MESSAGE_MAP(CSwarmerSourceDlg, CDialog)
	ON_BN_CLICKED(IDC_DATA_BUTTON,OnBnClickedGetData)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MAP_BUTTON, OnBnClickedMapButton)
END_MESSAGE_MAP()


void CSwarmerSourceDlg::InitParent(SwarmerSourceDll* parent)
{
	p_parent = parent;
}

BOOL CSwarmerSourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	return true;
}

void CSwarmerSourceDlg::OnTimer(UINT nIDEvent)
{


}

void CSwarmerSourceDlg::DisplayInfo(char * info)
{
	CFile logfile;
	if (logfile.Open("newswarmerlogfile.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone)== TRUE)
	{
		if ((int)logfile.GetLength() > 10000000)
		{
			logfile.Close();
			remove("newswarmerlogfile.txt");
			logfile.Open("newswarmerlogfile.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone);
		}
		logfile.SeekToEnd();
		logfile.Write(info, (unsigned int)strlen(info));
		logfile.Write("\r\n",2);
		logfile.Close();
	}


	int index = m_list_control.GetItemCount();
	m_list_control.InsertItem(index,info);
}

void CSwarmerSourceDlg::OnBnClickedGetData()
{
	p_parent->ProcessData();
}


// CSwarmerSourceDlg message handlers

void CSwarmerSourceDlg::OnBnClickedMapButton()
{
	// TODO: Add your control notification handler code here
	p_parent->GenerateMapFromFiles();
}
