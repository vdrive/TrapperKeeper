// DCMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCMasterDlg.h"
#include "DCMasterDll.h"
#define SUB_COLLECTOR					0
#define SUB_NETWORK						1
#define SUB_NUM_PROJECTS				2
#define SUB_PING_TIME					3
//#define SUB_CTIME						4
#define SUB_SUPPLY_RETRIEVAL_STARTED	4
#define SUB_SUPPLY_RETRIEVAL_ENDED		5
#define SUB_SUPPLY_RETRIEVAL_PROGRESS	6
#define WM_RESTART_COMPUTER				WM_USER_MENU+1

// CDCMasterDlg dialog

IMPLEMENT_DYNAMIC(CDCMasterDlg, CDialog)
CDCMasterDlg::CDCMasterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDCMasterDlg::IDD, pParent)
{
}

CDCMasterDlg::~CDCMasterDlg()
{
}

void CDCMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLLECTOR_LIST, m_collector_list);
}


BEGIN_MESSAGE_MAP(CDCMasterDlg, CDialog)
	ON_BN_CLICKED(IDC_RESET_BUTTON, OnBnClickedResetButton)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_COLLECTOR_LIST, OnLvnColumnclickCollectorList)
	ON_BN_CLICKED(IDC_BROWSE_PROJECTS_BUTTON, OnBnClickedBrowseProjectsButton)
	ON_NOTIFY(NM_RCLICK, IDC_COLLECTOR_LIST, OnNMRclickCollectorList)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Load_Balancing_Divider_Spin, OnDeltaposLoadBalancingDividerSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Singles_Load_Balancing_Spin, OnDeltaposSinglesLoadBalancingSpin)
	ON_BN_CLICKED(IDC_RESET_SPLITTED_GNUTELLA_BUTTON, OnBnClickedResetSplittedGnutellaButton)
	ON_BN_CLICKED(IDC_RESET_GIFT_SPOOFER_BUTTON, OnBnClickedResetGiftSpooferButton)
	ON_BN_CLICKED(IDC_RESET_GIFT_SWARMER_BUTTON, OnBnClickedResetGiftSwarmerButton)
	ON_BN_CLICKED(IDC_RESET_WINMX_SPOOFER_BUTTON, OnBnClickedResetWinmxSpooferButton)
	ON_BN_CLICKED(IDC_RESET_WINMX_SWARMER_BUTTON, OnBnClickedResetWinmxSwarmerButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Piolet_Load_Balancing_Divider_Spin, OnDeltaposPioletLoadBalancingDividerSpin)
	ON_BN_CLICKED(IDC_RESET_SPLITTED_PIOLET_BUTTON, OnBnClickedResetSplittedPioletButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_BearShare_Load_Balancing_Divider_Spin, OnDeltaposBearshareLoadBalancingDividerSpin)
	ON_BN_CLICKED(IDC_RESET_BEARSHARE_BUTTON, OnBnClickedResetBearshareButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Gift_Swarmer_Load_Balancing_Divider_Spin, OnDeltaposGiftSwarmerLoadBalancingDividerSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Gift_Spoofer_Load_Balancing_Divider_Spin, OnDeltaposGiftSpooferLoadBalancingDividerSpin)
END_MESSAGE_MAP()


// CDCMasterDlg message handlers

//
//
//
void CDCMasterDlg::OnBnClickedResetButton()
{
	p_parent->Reset();
}

//
//
//
void CDCMasterDlg::InitParent(DCMasterDll* parent)
{
	p_parent = parent;
}

//
//
//
void CDCMasterDlg::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==5)
		UpdateReponseStatus();
	else
		p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CDCMasterDlg::UpdateReponseStatus()
{
	for(UINT i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		/*
		char time[32];
		UINT ctime=0;
		m_collector_list.GetItemText(i,SUB_CTIME,time,sizeof(time));
		sscanf(time,"%u",&ctime);
		CTime response = (CTime)ctime;
		//CTime response = (CTime)m_collector_list.GetItemData(i);
		CTimeSpan ts = CTime::GetCurrentTime() - response;
		*/
		CString ip = m_collector_list.GetItemText(i,SUB_COLLECTOR);
		CString network = m_collector_list.GetItemText(i,SUB_NETWORK);
		CTimeSpan ts = CTime::GetCurrentTime() - p_parent->GetLastResponseTime(ip,network);
		CString timestring = ts.Format("%H:%M:%S");
		m_collector_list.SetItemText(i,SUB_PING_TIME, timestring);
	}
}

//
//
//
void CDCMasterDlg::UpdateCollectorStatus(DCStatus& collector, CString network)
{
	bool found = false;
	UINT i=0;
	for(i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		if( (strcmp(collector.m_ip, m_collector_list.GetItemText(i,SUB_COLLECTOR))==0) &&
			(strcmp(network,m_collector_list.GetItemText(i,SUB_NETWORK))==0) )

		{
			found = true;
			char num[32];
			sprintf(num, "%d", collector.v_projects.size());
			m_collector_list.SetItemText(i, SUB_NUM_PROJECTS, num);
			m_collector_list.SetItemText(i, SUB_NETWORK, network);
			//m_collector_list.SetItemData(i, collector.m_last_init_response_time.GetTime());
			/*
			sprintf(num,"%u",collector.m_last_init_response_time.GetTime());
			m_collector_list.SetItemText(i, SUB_CTIME,num);
			*/
			break;
		}
	}
	if(!found)
	{
		int index = m_collector_list.InsertItem(SUB_COLLECTOR,collector.m_ip);
		//m_collector_list.SetItemData(index, collector.m_last_init_response_time.GetTime());
		m_collector_list.SetItemData(index, m_collector_list.GetItemCount());
		m_collector_list.SetItemText(index, SUB_NETWORK, network);
		char num[32];
		sprintf(num, "%d", collector.v_projects.size());
		m_collector_list.SetItemText(index, SUB_NUM_PROJECTS, num);
		/*
		sprintf(num,"%u",collector.m_last_init_response_time.GetTime());
		m_collector_list.SetItemText(index, SUB_CTIME,num);
		*/
	}
}

//
//
//
void CDCMasterDlg::InitRunningSinceWindowText()
{
	// Update the time that the values were last cleared
	CTime time;
	time=CTime::GetCurrentTime();
	CString time_str = time.Format("PLEASE DON'T RESTART DCMASTER UNLESS IT HAS TO BE!!! Running since %A %m/%d/%Y at %H:%M");

	char msg[1024];
	char date[32];
	sprintf(date,__DATE__);
	char m[16];
	unsigned int d;
	unsigned int y;
	sscanf(date,"%s %u %u",&m,&d,&y);
	sprintf(msg,"[ Built on %s %02u, %u at %s ]",m,d,y,__TIME__);

	time_str += (" - ");
	time_str += msg;
	SetWindowText(time_str);
}
//
//
//
BOOL CDCMasterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_collector_list.SetExtendedStyle(m_collector_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	CRect rect;
	m_collector_list.GetWindowRect(&rect);
	int nWidth = rect.Width();

	m_collector_list.InsertColumn(SUB_COLLECTOR,"Collector",LVCFMT_LEFT, nWidth * 15/100);
	m_collector_list.InsertColumn(SUB_NUM_PROJECTS,"Projects",LVCFMT_LEFT, nWidth * 10/100);
	m_collector_list.InsertColumn(SUB_NETWORK, "Network", LVCFMT_LEFT, nWidth* 15/100);
	m_collector_list.InsertColumn(SUB_PING_TIME,"Last Response Time",LVCFMT_LEFT, nWidth *15/100);
	//m_collector_list.InsertColumn(SUB_CTIME,"CTime",LVCFMT_LEFT, nWidth *5/20);
	m_collector_list.InsertColumn(SUB_SUPPLY_RETRIEVAL_STARTED,"Gift Supply Started",LVCFMT_LEFT, nWidth *15/100);
	m_collector_list.InsertColumn(SUB_SUPPLY_RETRIEVAL_ENDED,"Gift Supply Ended",LVCFMT_LEFT, nWidth *15/100);
	m_collector_list.InsertColumn(SUB_SUPPLY_RETRIEVAL_PROGRESS,"Gift Supply Progress",LVCFMT_LEFT, nWidth *15/100);


	m_DlgResizer.InitResizer(this);
	m_DlgResizer.SizeItem(IDC_COLLECTOR_LIST, CDlgResizer::DownAndRight);
	m_DlgResizer.ListCtrlItem(IDC_COLLECTOR_LIST);
	m_DlgResizer.MoveItem(IDC_RESET_BUTTON,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_BROWSE_PROJECTS_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_RESET_SPLITTED_GNUTELLA_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_TOTAL_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_ACTIVE_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_GIFT_DC_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_GIFT_SPOOFING_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_GIFT_SWARMING_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_GNUTELLA_PROTECTION_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Singles_Projects_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Gnutella_Load_Balancing_Divider_STATIC,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Gnutella_Singles_Load_Balancing_Divider_STATIC,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Load_Balancing_Divider_Static,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Singles_Load_Balancing_Divider_Static,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Load_Balancing_Divider_Spin,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Singles_Load_Balancing_Spin,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_WINMX_SPOOFING_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_WINMX_SWARMING_PROJECTS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_RESET_WINMX_SPOOFER_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_RESET_WINMX_SWARMER_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_RESET_GIFT_SPOOFER_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_RESET_GIFT_SWARMER_BUTTON,CDlgResizer::DownAndRight);	
	m_DlgResizer.MoveItem(IDC_PIOLET_DIVIDER_STATIC,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Piolet_Load_Balancing_Divider_Static,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Piolet_Load_Balancing_Divider_Spin,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_RESET_SPLITTED_PIOLET_BUTTON,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_BEARSHARE_DIVIDER_STATIC,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_BearShare_Load_Balancing_Divider_Static,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_BearShare_Load_Balancing_Divider_Spin,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_RESET_BEARSHARE_BUTTON,CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Gift_Spoofer_Load_Balancing_Divider_STATIC,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_Gift_Swarmer_Load_Balancing_Divider_STATIC,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_GiftSpoofer_Load_Balancing_Divider_Static,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_GiftSwarmer_Load_Balancing_Divider_Static,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_Gift_Spoofer_Load_Balancing_Divider_Spin,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_Gift_Swarmer_Load_Balancing_Divider_Spin,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_BearShare_Projects_STATIC,CDlgResizer::Down);	
	m_DlgResizer.Done();

	InitRunningSinceWindowText();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CDCMasterDlg::ClearProjectsNumber(char* network)
{
	UINT i=0;
	for(i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		if(strcmp(network, m_collector_list.GetItemText(i,SUB_NETWORK))==0)
		{
			m_collector_list.SetItemText(i, SUB_NUM_PROJECTS, "0");
		}
	}
}



//
//
//
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{
	// lParamSort is the list control pointer and column
	// lParam1 and lParam2 is the index

	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl *list=(CListCtrl *)params[0];
	int sub_item=(int)params[1];

	bool found1=false;
	bool found2=false;
	CString cstring1,cstring2;

	HWND hList = list->GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam1;
	int index1 = ListView_FindItem(hList, -1, &lvfi);
	lvfi.lParam = lParam2;
	int index2 = ListView_FindItem(hList, -1, &lvfi);
	cstring1=list->GetItemText(index1,sub_item);
	cstring2=list->GetItemText(index2,sub_item);

	// Make then both lowercase
	cstring1.MakeLower();
	cstring2.MakeLower();
	// Put empty strings at the bottom


	switch(sub_item)
	{
		case SUB_SUPPLY_RETRIEVAL_STARTED:
		case SUB_SUPPLY_RETRIEVAL_ENDED:
		case SUB_SUPPLY_RETRIEVAL_PROGRESS:
		case SUB_NETWORK:
		case SUB_PING_TIME:
		{
			// Put empty strings at the bottom
			if(cstring1.IsEmpty())
			{
				return 1;
			}
			else if(cstring2.IsEmpty())
			{
				return -1;
			}
			else
			{
				return strcmp(cstring1,cstring2);
			}
		}
		case SUB_COLLECTOR:
		{
			unsigned int ip_1=0xFFFFFFFF;	// very big
			unsigned int ip_2=0xFFFFFFFF;	// very big
			unsigned int ip1,ip2,ip3,ip4;
			sscanf(cstring1,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
			ip_1=(ip1<<24)+(ip2<<16)+(ip3<<8)+(ip4<<0);	// big-endian
			sscanf(cstring2,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
			ip_2=(ip1<<24)+(ip2<<16)+(ip3<<8)+(ip4<<0);	// big-endian

			if(ip_1<ip_2)
			{
				return -1;
			}
			else if(ip_1>ip_2)
			{
				return 1;
			}
			
			return 0;
		}
		case SUB_NUM_PROJECTS:
		{
			float val1=0x00FFFFFF;	// very big
			float val2=0x00FFFFFF;	// very big
			sscanf(cstring1,"%f",&val1);
			sscanf(cstring2,"%f",&val2);

			if(val1<val2)
			{
				return -1;
			}
			else if(val1>val2)
			{
				return 1;
			}

			return 0;
		}
		default:
		{
			return 0;
		}
	}
}

//
//
//
void CDCMasterDlg::OnLvnColumnclickCollectorList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_collector_list;
	params[1]=pNMLV->iSubItem;
	m_collector_list.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::ReceivedGiftSupplyStarted(const char* ip)
{
	UINT i=0;
	for(i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		if(strcmp(ip, m_collector_list.GetItemText(i,SUB_COLLECTOR))==0)
		{
			CString time=CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_STARTED, time);
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_ENDED, "");
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_PROGRESS, "0%");
			break;
		}
	}
}

//
//
//
void CDCMasterDlg::ReceivedGiftSupplyEnded(const char* ip)
{
	UINT i=0;
	for(i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		if(strcmp(ip, m_collector_list.GetItemText(i,SUB_COLLECTOR))==0)
		{
			CString time=CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_ENDED, time);
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_PROGRESS, "100%");
			break;
		}
	}
}

//
//
//
void CDCMasterDlg::ReceivedGiftSupplyProgress(const char* ip, int progress)
{
	UINT i=0;
	for(i=0;i<(UINT)m_collector_list.GetItemCount();i++)
	{
		if(strcmp(ip, m_collector_list.GetItemText(i,SUB_COLLECTOR))==0)
		{
			char num[32];
			sprintf(num, "%d%%", progress);
			m_collector_list.SetItemText(i, SUB_SUPPLY_RETRIEVAL_PROGRESS, num);
			break;
		}
	}
}

//
//
//
void CDCMasterDlg::OnBnClickedBrowseProjectsButton()
{
	p_parent->OnBnClickedBrowseProjectsButton();
}

//
//
//
void CDCMasterDlg::OnNMRclickCollectorList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_collector_list.GetSelectedCount()>0)
	{
		POINT point;
		GetCursorPos(&point);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING,WM_RESTART_COMPUTER,"Restart Computer");
		menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this,0);
	}
	*pResult = 0;	*pResult = 0;
}

//
//
//
BOOL CDCMasterDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Check for right-click popup menu items
	if(wParam>WM_USER_MENU)
	{
		switch(wParam)
		{

			case WM_RESTART_COMPUTER:
			{
				OnRestartComputer();
				break;
			}
		}
	}
	
	return CDialog::OnCommand(wParam,lParam);
}

//
//
//
void CDCMasterDlg::OnRestartComputer()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_collector_list.GetSelectedCount()>0)
	{
		POSITION pos = m_collector_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_collector_list.GetNextSelectedItem(pos);
				rack_name = m_collector_list.GetItemText(nItem,SUB_COLLECTOR);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRestart(selected_racks);
	}
}

//
//
//
void CDCMasterDlg::OnDeltaposLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterGnutellaLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::OnDeltaposSinglesLoadBalancingSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterGnutellaSinglesLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::AlterGnutellaLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterGnutellaLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_Load_Balancing_Divider_Static)->SetWindowText(buf);

}

//
//
//
void CDCMasterDlg::AlterGnutellaSinglesLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterGnutellaSinglesLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_Singles_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::SetDividers(int normal, int singles)
{
	char buf[16];
	sprintf(buf,"%u",singles);
	GetDlgItem(IDC_Singles_Load_Balancing_Divider_Static)->SetWindowText(buf);
	sprintf(buf,"%u",normal);
	GetDlgItem(IDC_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::SetPioletDivider(int normal)
{
	char buf[16];
	sprintf(buf,"%u",normal);
	GetDlgItem(IDC_Piolet_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::SetBearShareDivider(int normal)
{
	char buf[16];
	sprintf(buf,"%u",normal);
	GetDlgItem(IDC_BearShare_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::SetGiftSwarmerDivider(int normal)
{
	char buf[16];
	sprintf(buf,"%u",normal);
	GetDlgItem(IDC_GiftSwarmer_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::SetGiftSpooferDivider(int normal)
{
	char buf[16];
	sprintf(buf,"%u",normal);
	GetDlgItem(IDC_GiftSpoofer_Load_Balancing_Divider_Static)->SetWindowText(buf);
}

//
//
//
void CDCMasterDlg::OnBnClickedResetSplittedGnutellaButton()
{
	p_parent->ResetSplittedGnutellaProjects();
}

void CDCMasterDlg::OnBnClickedResetGiftSpooferButton()
{
	p_parent->ResetGiftSpooferProjects();
}

void CDCMasterDlg::OnBnClickedResetGiftSwarmerButton()
{
	p_parent->ResetGiftSwarmerProjects();
}

void CDCMasterDlg::OnBnClickedResetWinmxSpooferButton()
{
	p_parent->ResetWinmxSpooferProjects();
}

void CDCMasterDlg::OnBnClickedResetWinmxSwarmerButton()
{
	p_parent->ResetWinmxSwarmerProjects();
}

void CDCMasterDlg::OnDeltaposPioletLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterPioletLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::AlterPioletLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterPioletLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_Piolet_Load_Balancing_Divider_Static)->SetWindowText(buf);

}

//
//
//
void CDCMasterDlg::AlterBearShareLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterBearShareLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_BearShare_Load_Balancing_Divider_Static)->SetWindowText(buf);

}

//
//
//
void CDCMasterDlg::OnBnClickedResetSplittedPioletButton()
{
	p_parent->ResetSplittedPioletProjects();
}

//
//
//
void CDCMasterDlg::OnDeltaposBearshareLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterBearShareLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::OnBnClickedResetBearshareButton()
{
	p_parent->ResetBearShareProjects();
}

//
//
//
void CDCMasterDlg::OnDeltaposGiftSwarmerLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterGiftSwarmerLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::OnDeltaposGiftSpooferLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterGiftSpooferLoadBalancingDivider((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CDCMasterDlg::AlterGiftSwarmerLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterGiftSwarmerLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_GiftSwarmer_Load_Balancing_Divider_Static)->SetWindowText(buf);

}

//
//
//
void CDCMasterDlg::AlterGiftSpooferLoadBalancingDivider(int value)
{
	int new_value = p_parent->AlterGiftSpooferLoadBalancingDivider(value);
	char buf[16];
	sprintf(buf,"%u",new_value);
	GetDlgItem(IDC_GiftSpoofer_Load_Balancing_Divider_Static)->SetWindowText(buf);

}