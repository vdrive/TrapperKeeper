// SwarmerDownloaderDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "KazaaSwarmerDownloader.h"
#include "SwarmerDownloaderDlg.h"
#include "SwarmerDownloader.h"



// SwarmerDownloaderDlg dialog

IMPLEMENT_DYNAMIC(SwarmerDownloaderDlg, CDialog)
SwarmerDownloaderDlg::SwarmerDownloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SwarmerDownloaderDlg::IDD, pParent)
{
}

SwarmerDownloaderDlg::~SwarmerDownloaderDlg()
{
}

void SwarmerDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_Downloader_list, m_list_control);
}


BEGIN_MESSAGE_MAP(SwarmerDownloaderDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void SwarmerDownloaderDlg::InitParent(SwarmerDownloader * parent)
{
	p_parent = parent;
}

BOOL SwarmerDownloaderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	return true;
}

void SwarmerDownloaderDlg::OnTimer(UINT nIDEvent)
{

	switch (nIDEvent)
	{
		case (1):
			{
				p_parent->TimerFired();
				DisplayInfo("Timer Fired");

				// Debug
				//KillTimer(1);
			}

	}
}

void SwarmerDownloaderDlg::DisplayInfo(char * info)
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


// SwarmerDownloaderDlg message handlers
