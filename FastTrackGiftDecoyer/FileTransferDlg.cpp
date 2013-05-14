// FileTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FastTrackGiftDlg.h"
#include "FileTransferDlg.h"
#include "LogFile.h"


// CFileTransferDlg dialog

IMPLEMENT_DYNAMIC(CFileTransferDlg, CDialog)
CFileTransferDlg::CFileTransferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileTransferDlg::IDD, pParent)
{
}

CFileTransferDlg::~CFileTransferDlg()
{
}

void CFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_List, m_list);
}


BEGIN_MESSAGE_MAP(CFileTransferDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_Disable_List, OnDisableList)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFileTransferDlg message handlers
void CFileTransferDlg::OnPaint() 
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
void CFileTransferDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CFileTransferDlg::InitParent(CFastTrackGiftDlg *parent)
{
	p_parent = parent;
}

//
//
//
void CFileTransferDlg::ReportStatus(vector<NoiseModuleThreadStatusData> &status)
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

	sprintf(buf,"Total Connections : %s",FormatWithCommas(m_total_num_received_connections).c_str());
	GetDlgItem(IDC_Total_Num_Connections)->SetWindowText(buf);

	// Update the virtual list control if it is enabled
	if(m_list.IsWindowEnabled())
	{
		m_list.SetItemCountEx(m_list.GetItemCount(),LVSICF_NOSCROLL);
	}

	// Write out logfile
	//WriteToLogfile();
}


//
//
//
void CFileTransferDlg::WriteToLogfile()
{
	// Write out logfile, if it is a new hour and minute
	CTime time=CTime::GetCurrentTime();
	if((time.GetHour()!=m_previous_logging_hour) || (time.GetMinute()!=m_previous_logging_minute))
	{
		char self[256];
		memset(self,0,sizeof(self));
		unsigned long len=sizeof(self);
		GetComputerName(self,&len);
		Logfile logfile(self);
		
		char buf[4096];
		memset(buf,0,sizeof(buf));

		int num_connections=m_list.GetItemCount();

		double bps=p_parent->GetBandwidthInBytes()*8;

		sprintf(buf,"%02u:%02u:%02u - NumGood: %u NumBogus: %u NumTotal: %u CurrentConnections: %u Bandwidth: %.2f KB/sec - %.2f %%\n",
			time.GetHour(),time.GetMinute(),time.GetSecond(),
			m_num_good_connections-m_previous_logging_num_good_connections,
			m_num_bogus_connections-m_previous_logging_num_bogus_connections,
			m_total_num_received_connections-m_previous_logging_total_num_received_connections,
			num_connections,((double)p_parent->GetBandwidthInBytes())/1024,(100*bps)/p_parent->GetBandwdithInBitsPerSecond());

		// Store values for next time
		m_previous_logging_hour=time.GetHour();
		m_previous_logging_minute=time.GetMinute();
		m_previous_logging_num_good_connections=m_num_good_connections;
		m_previous_logging_num_bogus_connections=m_num_bogus_connections;
		m_previous_logging_total_num_received_connections=m_total_num_received_connections;
				
		logfile.Log(buf);
	}
}

//
//
//
string CFileTransferDlg::FormatWithCommas(unsigned int num)
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
BOOL CFileTransferDlg::OnInitDialog()
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
	m_list.InsertColumn(SUB_FILENAME,"File",LVCFMT_LEFT,250);
	m_list.InsertColumn(SUB_RANGE,"Range",LVCFMT_LEFT,140);
	m_list.InsertColumn(SUB_TOTAL_SIZE,"Total Size",LVCFMT_LEFT,100);
	m_list.InsertColumn(SUB_SENT,"Sent",LVCFMT_LEFT,125);
	m_list.InsertColumn(SUB_ELAPSED,"Elapsed",LVCFMT_LEFT,70);
	m_list.InsertColumn(SUB_VERSION,"Version",LVCFMT_LEFT,300);

	m_num_good_connections=0;
	m_num_bogus_connections=0;
	m_total_num_received_connections=0;

	m_previous_logging_hour=24;	// not a real hour...real hours are 0-23
	m_previous_logging_minute=60;	// not a real minute...real minutes are 0-59
	m_previous_logging_total_num_received_connections=0;
	m_previous_logging_num_good_connections=0;
	m_previous_logging_num_bogus_connections=0;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CFileTransferDlg::OnDisableList() 
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