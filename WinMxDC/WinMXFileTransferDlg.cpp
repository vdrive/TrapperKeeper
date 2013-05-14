// WinMXFileTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinMxDcDlg.h"
#include "WinMXFileTransferDlg.h"


// CWinMXFileTransferDlg dialog

IMPLEMENT_DYNAMIC(CWinMXFileTransferDlg, CDialog)
CWinMXFileTransferDlg::CWinMXFileTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinMXFileTransferDlg::IDD, pParent)
{
}

CWinMXFileTransferDlg::~CWinMXFileTransferDlg()
{
}

void CWinMXFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_List, m_list);
}


BEGIN_MESSAGE_MAP(CWinMXFileTransferDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_Disable_List, OnDisableList)
	ON_WM_TIMER()
//	ON_WM_DESTROY()
//ON_WM_DESTROY()
//ON_WM_CLOSE()
END_MESSAGE_MAP()


// CWinMXFileTransferDlg message handlers
void CWinMXFileTransferDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		//dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//
//
//
void CWinMXFileTransferDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CWinMXFileTransferDlg::InitParent(CWinMxDcDlg *parent)
{
	p_parent = parent;
}

//
//
//
void CWinMXFileTransferDlg::ReportStatus(vector<NoiseModuleThreadStatusData> &status)
{
	UINT i;
	unsigned int num_connections=0;
	for(i=0;i<status.size();i++)
	{
		m_num_good_connections+=status[i].m_num_good_connections;
		m_num_bogus_connections+=status[i].m_num_bogus_connections;
	}

	// Tell the list
//	if(m_list.IsWindowEnabled())
//	{
		m_list.ReportStatus(status);
//	}

	// Counters
	char buf[4096];
	sprintf(buf,"Mods : %u   /   Connections : %s",p_parent->GetModCount(),FormatWithCommas(m_list.GetItemCount()).c_str());
	GetDlgItem(IDC_Num_Current_Connections)->SetWindowText(buf);

	sprintf(buf,"Good Connections : %s",FormatWithCommas(m_num_good_connections).c_str());
	GetDlgItem(IDC_Num_Good_Connections)->SetWindowText(buf);

	sprintf(buf,"Bogus Connections : %s",FormatWithCommas(m_num_bogus_connections).c_str());
	GetDlgItem(IDC_Num_Bogus_Connections)->SetWindowText(buf);

	sprintf(buf,"Total Connections : %s",FormatWithCommas(m_num_good_connections+m_num_bogus_connections).c_str());
	GetDlgItem(IDC_Total_Num_Connections)->SetWindowText(buf);

	// Update the virtual list control if it is enabled
	if(m_list.IsWindowEnabled())
	{
		m_list.SetItemCountEx(m_list.GetItemCount(),LVSICF_NOSCROLL);
	}
}

//
//
//
string CWinMXFileTransferDlg::FormatWithCommas(unsigned int num)
{
	string ret;

	char buf[32];
	char buf2[sizeof(buf)*2];
	memset(buf,0,sizeof(buf));
	memset(buf2,0,sizeof(buf2));

	_itoa(num,buf,10);

	char *ptr=&buf[strlen(buf)-1];	// last char
	char *ptr2=&buf2[sizeof(buf2)-2];	// last char
	int i=0;
	while(1)
	{
		if(i>0)
		{
			if(i%3==0)
			{
				*ptr2=',';
				ptr2--;
			}
		}

		*ptr2=*ptr;

		if(ptr==buf)
		{
			break;
		}

		ptr--;
		ptr2--;
		i++;
	}
	*ptr2=*ptr;
	
	ret=ptr2;

	return ret;
}

//
//
//
BOOL CWinMXFileTransferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

/*	char msg[1024];
	char date[32];
	sprintf(date,__DATE__);
	char m[16];
	unsigned int d;
	unsigned int y;
	sscanf(date,"%s %u %u",&m,&d,&y);
	sprintf(msg,"[ Built on %s %02u, %u at %s ]",m,d,y,__TIME__);
	GetDlgItem(IDC_About_Static)->SetWindowText(msg);
*/
	m_list.InsertColumn(SUB_REMOTE_IP,"Remote IP",LVCFMT_LEFT,100);
	m_list.InsertColumn(SUB_MODULE,"Mod",LVCFMT_LEFT,40);
	m_list.InsertColumn(SUB_SOCKET,"Sock",LVCFMT_LEFT,40);
	m_list.InsertColumn(SUB_FILENAME,"Filename",LVCFMT_LEFT,450);
	m_list.InsertColumn(SUB_RANGE,"Range",LVCFMT_LEFT,100);
	m_list.InsertColumn(SUB_TOTAL_SIZE,"Total Size",LVCFMT_LEFT,100);
	m_list.InsertColumn(SUB_SENT,"Sent",LVCFMT_LEFT,125);
	m_list.InsertColumn(SUB_ELAPSED,"Elapsed",LVCFMT_LEFT,70);

	m_num_good_connections=0;
	m_num_bogus_connections=0;

	m_previous_logging_hour=24;	// not a real hour...real hours are 0-23
	m_previous_logging_minute=60;	// not a real minute...real minutes are 0-59
	m_previous_logging_total_num_received_connections=0;
	m_previous_logging_num_good_connections=0;
	m_previous_logging_num_bogus_connections=0;

	m_DlgResizer.InitResizer(this);	
	m_DlgResizer.SizeItem(IDC_List, CDlgResizer::DownAndRight);
	m_DlgResizer.ListCtrlItem(IDC_List);
	m_DlgResizer.MoveItem(IDC_Num_Good_Connections,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Num_Bogus_Connections,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Total_Num_Connections,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Num_Current_Connections,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Disable_List,   CDlgResizer::Down);
	m_DlgResizer.DialogIsMinSize();
	m_DlgResizer.Done();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CWinMXFileTransferDlg::OnDisableList() 
{
	CButton *button=(CButton *)GetDlgItem(IDC_Disable_List);
	if(button->GetCheck()==1)
	{
		m_list.DeleteAllItems();
		m_list.SetItemCountEx(m_list.GetItemCount(),LVSICF_NOSCROLL);
		m_list.EnableWindow(FALSE);
	}
	else
	{
		m_list.EnableWindow(TRUE);
	}
}

//
//
//
void CWinMXFileTransferDlg::OnUnInitialize()
{
	m_DlgResizer.OnDestroy();
}