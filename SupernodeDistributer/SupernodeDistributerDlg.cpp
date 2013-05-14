// SupernodeDistributerDlg.cpp : implementation file
// CSupernodeDistributerDlg dialog

#include "stdafx.h"
#include "SupernodeDistributerDlg.h"
#include "HttpSocket.h"
#include "SupernodeDistributerDll.h"


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


	// Make then both lowercase
	cstring1.MakeLower();
	cstring2.MakeLower();

	switch(sub_item)
	{
		case SUB_IP:
		{
			char ip[32];
			memset(&ip,0,sizeof(ip));
			sprintf(ip,"%u.%u.%u.%u",(lParam1>>0)&0xFF,(lParam1>>8)&0xFF,
				(lParam1>>16)&0xFF,(lParam1>>24)&0xFF);
			cstring1 = ip;
			sprintf(ip,"%u.%u.%u.%u",(lParam2>>0)&0xFF,(lParam2>>8)&0xFF,
				(lParam2>>16)&0xFF,(lParam2>>24)&0xFF);
			cstring2 = ip;
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
			break;
		}
/*		case SUB_PORT:
		{
			HWND hList = list->GetSafeHwnd();
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=list->GetItemText(index1,sub_item);
			cstring2=list->GetItemText(index2,sub_item);
			float val1=0x00FFFFFF;	// very big
			float val2=0x00FFFFFF;	// very big
			sscanf(cstring1,"%d",&val1);
			sscanf(cstring2,"%d",&val2);

			if(val1<val2)
			{
				return -1;
			}
			else if(val1>val2)
			{
				return 1;
			}

			return 0;
			break;
		}
		case SUB_STATUS:
		case SUB_USERNAME:
		case SUB_RACK:
		{
			HWND hList = list->GetSafeHwnd();
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=list->GetItemText(index1,sub_item);
			cstring2=list->GetItemText(index2,sub_item);

			cstring1.MakeLower();
			cstring2.MakeLower();
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
			break;

		}
		case SUB_UP_TIME:
		case SUB_DOWN_TIME:
		{
			HWND hList = list->GetSafeHwnd();
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=list->GetItemText(index1,sub_item);
			cstring2=list->GetItemText(index2,sub_item);

			cstring1.MakeLower();
			cstring2.MakeLower();
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
				return (-strcmp(cstring1,cstring2));
			}
			break;

		}
*/		default:
		{
			return 0;
		}
	}
}

IMPLEMENT_DYNAMIC(CSupernodeDistributerDlg, CPropertyPage)
CSupernodeDistributerDlg::CSupernodeDistributerDlg()
	: CPropertyPage(CSupernodeDistributerDlg::IDD)
	, m_launching_interval(0)
	, m_share_folder(_T(""))
	, m_reset_supernodes_hours(0)
{
	m_launching_interval = 10; //10 sec
	m_num_supernode_alive = 0;
	m_enable_launch = true;
	m_disable_probing = false;
}

CSupernodeDistributerDlg::~CSupernodeDistributerDlg()
{
}

void CSupernodeDistributerDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_LIST, m_ip_list);
	DDX_Text(pDX, IDC_LAUNCHING_INTERVAL, m_launching_interval);
	DDX_Text(pDX, IDC_SHARE_FOLDER_EDIT, m_share_folder);
	DDX_Control(pDX, IDC_SHARE_FOLDER_LIST, m_share_folder_list);
	DDX_Text(pDX, IDC_HOURS_EDIT, m_reset_supernodes_hours);
	DDV_MinMaxInt(pDX, m_reset_supernodes_hours, 0, 9999);
}


BEGIN_MESSAGE_MAP(CSupernodeDistributerDlg, CPropertyPage)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnBnClickedStopButton)
	ON_BN_CLICKED(IDC_DISABLE_BUTTON, OnBnClickedDisableButton)
	ON_BN_CLICKED(IDC_SAVE_IP_BUTTON, OnBnClickedSaveIpButton)
	ON_BN_CLICKED(IDC_LOAD_IP_BUTTON2, OnBnClickedLoadIpButton2)
	ON_BN_CLICKED(IDC_SAVE_USERNAME_BUTTON, OnBnClickedSaveUsernameButton)
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_IP_LIST, OnLvnColumnclickIpList)
//	ON_STN_CLICKED(IDC_PROBING_INDEX, OnStnClickedProbingIndex)
ON_BN_CLICKED(IDC_ADD_FOLDER_BUTTON, OnBnClickedAddFolderButton)
ON_BN_CLICKED(IDC_REMOVE_BUTTON, OnBnClickedRemoveButton)
ON_BN_CLICKED(IDC_APPLY_BUTTON, OnBnClickedApplyButton)
END_MESSAGE_MAP()


// CSupernodeDistributerDlg message handlers

/*
void CSupernodeDistributerDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	ResizePage();
}
*/
BOOL CSupernodeDistributerDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CRect main_rect,list_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_IP_LIST)->GetWindowRect(&list_rect);
	m_border=list_rect.left-main_rect.left;
	m_bottom=main_rect.bottom - list_rect.bottom;

	m_ip_list.SetExtendedStyle(m_ip_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ip_list.InsertColumn(SUB_IP,"Supernode's IP",LVCFMT_LEFT,120);
	m_ip_list.InsertColumn(SUB_PORT,"Port",LVCFMT_LEFT,70);
	//m_ip_list.InsertColumn(SUB_STATUS,"Status",LVCFMT_CENTER,100);
	m_ip_list.InsertColumn(SUB_USERNAME,"Kazaa Username",LVCFMT_CENTER,150);
	m_ip_list.InsertColumn(SUB_UP_TIME,"Up Since",LVCFMT_CENTER,150);
	//m_ip_list.InsertColumn(SUB_DOWN_TIME,"Down Time",LVCFMT_CENTER,150);
	m_ip_list.InsertColumn(SUB_RACK,"Rack",LVCFMT_CENTER,120);
	UpdateData(FALSE);

	m_DlgResizer.InitResizer(this);
	m_DlgResizer.SizeItem(IDC_IP_LIST, CDlgResizer::DownAndRight);
	m_DlgResizer.ListCtrlItem(IDC_IP_LIST);
	m_DlgResizer.MoveItem(IDC_SHARE_FOLDER_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveSizeItem(IDC_SHARE_FOLDER_LIST,CDlgResizer::Down,CDlgResizer::Right);
	m_DlgResizer.MoveItem(IDC_SHARE_FOLDER_EDIT,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_ADD_FOLDER_BUTTON,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_REMOVE_BUTTON,CDlgResizer::Down);		
	
	m_DlgResizer.MoveItem(IDC_RESET_SUPERNODES_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_HOURS_EDIT,CDlgResizer::Down);	
	m_DlgResizer.MoveItem(IDC_HOURS_STATIC,CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_APPLY_BUTTON,CDlgResizer::Down);	
	m_DlgResizer.Done();

	ReadFolderListFromRegistry();
	ReadSupernodesResetHoursFromRegistry();

	if(m_reset_supernodes_hours > 0)
		SetTimer(8,m_reset_supernodes_hours*3600*1000,NULL);


	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CSupernodeDistributerDlg::InitParent(SupernodeDistributerDll* parent)
{
	p_parent = parent;
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedStopButton()
{
	m_enable_launch = !m_enable_launch;
	if(m_enable_launch)
	{
		SetDlgItemText(IDC_STOP_BUTTON,"Stop Gathering");
		SetTimer(1,10*1000,0);
		SetTimer(2,m_launching_interval*1000,0);
	}
	else
	{
		KillTimer(1);
		KillTimer(2);
		SetDlgItemText(IDC_STOP_BUTTON,"Start Gathering");
	}	
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedDisableButton()
{
	m_disable_probing = !m_disable_probing;
	if(m_disable_probing == true)
	{
		p_parent->m_probing_index = 0;
		KillTimer(4);
		SetDlgItemText(IDC_DISABLE_BUTTON,"Enable Probing");
	}
	else
	{
		SetTimer(3, 11*1000, 0); //probing for the first time
		SetTimer(4, 30*1000, 0); //check for idle sockets
		SetDlgItemText(IDC_DISABLE_BUTTON,"Disable Probing");
	}
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedSaveIpButton()
{
	p_parent->SaveSupernodeList();	
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedLoadIpButton2()
{
	p_parent->LoadSupernodeList();	
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedSaveUsernameButton()
{
	p_parent->SaveUserNames();
}

//
//
//
void CSupernodeDistributerDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CPropertyPage::OnTimer(nIDEvent);
}

//
//
//
void CSupernodeDistributerDlg::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	UpdateData(TRUE);
/*
	char interval[16];
	GetDlgItemText(IDC_LAUNCHING_INTERVAL,interval, sizeof(interval));
	m_launching_interval = atoi(interval);
*/	KillTimer(2);
	SetTimer(2,m_launching_interval*1000,0);
	*pResult = 0;
}

//
//
//
void CSupernodeDistributerDlg::ReportStatus(HttpSocket* socket, char* ip,int& port,int& error,string& rack_name,CTime& up_time, CTime& down_time,char *username,char *supernode_ip,char *supernode_port)
{
	int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	int ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);

	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = ip_int;
	int index = ListView_FindItem(hList, -1, &lvfi);
/*	char rackname[16+1];
	if(rack_ip==0)
		strcpy(rackname,"0");
	else
	{
		sprintf(rackname,"%u.%u.%u.%u",(rack_ip>>0)&0xFF,(rack_ip>>8)&0xFF,
			(rack_ip>>16)&0xFF,(rack_ip>>24)&0xFF);
	}
*/
	if(index >= 0)
		m_ip_list.SetItemText(index,SUB_RACK,rack_name.c_str());

	if(error==0)
	{
		/*
		if(index >= 0)
		{
			m_ip_list.SetItemText(index,SUB_STATUS,"Complete");
			m_num_supernode_alive++;
		}
		
		// Check to see if this dude is a supernode
		if((strlen(supernode_ip)==0) && (strlen(supernode_port)==0))
		{
			if(index >= 0)
			{
		*/		m_ip_list.SetItemText(index,SUB_USERNAME,username);
				//CTimeSpan up_time_span = CTime::GetCurrentTime() - up_time;
				//CString cs_uptime = up_time_span.Format("%D days - %H:%M:%S");
				CString cs_uptime = up_time.Format("%Y-%m-%d %H:%M:%S");
				m_ip_list.SetItemText(index,SUB_UP_TIME,cs_uptime); 
//				m_ip_list.SetItemText(index,SUB_DOWN_TIME,"");
		/*	}
		}
		else //add it's supernode to the big list
		{
			if(index >= 0)
				m_ip_list.DeleteItem(index);
			p_parent->RemoveSuperNode(ip,port); //remove the old supernode from the list
			p_parent->AddSuperNode(supernode_ip, supernode_port); //add it to the list
		}
		*/

	}
	/*
	else //supernode down
	{
		if(index >= 0)
		{
			CTimeSpan down_time_span = CTime::GetCurrentTime() - down_time;
			CString cs_downtime = down_time_span.Format("%D days - %H:%M:%S");

			m_ip_list.SetItemText(index,SUB_STATUS,"Error");
			m_ip_list.SetItemText(index,SUB_USERNAME,"");
			m_ip_list.SetItemText(index,SUB_UP_TIME,"");
			m_ip_list.SetItemText(index,SUB_DOWN_TIME,cs_downtime);
		}
		if(strcmp(rack_name.c_str(), "NULL") != 0)
		{
			if(index >= 0)
				m_ip_list.SetItemText(index,SUB_RACK,"0");
			p_parent->SuperNodeIsDown(rack_name, ip_int, port);
		}
	}
	*/

	char num[128];
	sprintf(num,"Number of Supernodes Alive: %u",m_num_supernode_alive);
	SetDlgItemText(IDC_NUM_SUPERNODES_ALIVE, num);
	sprintf(num,"Number of Supernodes: %u",p_parent->GetSuperNodeListSize());
	SetDlgItemText(IDC_NUM_SUPERNODES, num);

	// Probe the next item
	//if(m_disable_probing==false)
	//	p_parent->ProbeNextItem();
}

//
//
//
void CSupernodeDistributerDlg::AddSuperNode(char* ip, char* port)
{
	int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	int ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);

	UINT index = m_ip_list.InsertItem(m_ip_list.GetItemCount(),ip);
	m_ip_list.SetItemData(index, ip_int);
	m_ip_list.SetItemText(index,SUB_PORT,port);
}

//
//
//
void CSupernodeDistributerDlg::AddSuperNode(IPAddress& ip_address, char* kazaa_user, char* rack_name)
{
	int index =0;
	char ip[16];
	char port[8];
	sprintf(ip,"%u.%u.%u.%u",(ip_address.m_ip>>0)&0xFF,(ip_address.m_ip>>8)&0xFF,
		(ip_address.m_ip>>16)&0xFF,(ip_address.m_ip>>24)&0xFF);
	sprintf(port,"%d",ip_address.m_port);
	index = m_ip_list.InsertItem(m_ip_list.GetItemCount(),ip);
	m_ip_list.SetItemData(index, ip_address.m_ip);
	m_ip_list.SetItemText(index,SUB_PORT,port);
	m_ip_list.SetItemText(index,SUB_USERNAME,kazaa_user);
	m_ip_list.SetItemText(index,SUB_RACK,rack_name);
	CString cs_uptime = ip_address.m_up_time.Format("%Y-%m-%d %H:%M:%S");
	m_ip_list.SetItemText(index,SUB_UP_TIME,cs_uptime); 

	char num[128];
	sprintf(num,"Number of Supernodes: %u",p_parent->GetSuperNodeListSize());
	SetDlgItemText(IDC_NUM_SUPERNODES, num);
}
//
//
//
void CSupernodeDistributerDlg::SetConnectingStatus(int ip)
{
/*
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = ip;
	int index = ListView_FindItem(hList, -1, &lvfi);
	CString status = m_ip_list.GetItemText(index,SUB_STATUS);
	if(status == "Complete")
		m_num_supernode_alive--;
	m_ip_list.SetItemText(index,SUB_STATUS,"Connecting");
*/
}

//
//
//
void CSupernodeDistributerDlg::UpdateProbingIndexStatus(UINT& probing_index, int queue_size)
{
	char num[128];
	sprintf(num,"Connections: %d",probing_index);
	SetDlgItemText(IDC_PROBING_INDEX, num);
	sprintf(num,"Queue Size: %d",queue_size);
	SetDlgItemText(IDC_QUEUE_SIZE, num);

}

//
//
//
void CSupernodeDistributerDlg::RefreshSupernodeList(vector<IPAddress>& ip_list)
{
	int index =0;
	for(unsigned int i=0; i<ip_list.size();i++)
	{
		char ip[16];
		char port[8];
		sprintf(ip,"%u.%u.%u.%u",(ip_list[i].m_ip>>0)&0xFF,(ip_list[i].m_ip>>8)&0xFF,
			(ip_list[i].m_ip>>16)&0xFF,(ip_list[i].m_ip>>24)&0xFF);
		sprintf(port,"%d",ip_list[i].m_port);
		index = m_ip_list.InsertItem(m_ip_list.GetItemCount(),ip);
		m_ip_list.SetItemData(index, ip_list[i].m_ip);
		m_ip_list.SetItemText(index,SUB_PORT,port);
	}
	char num[128];
	sprintf(num,"Number of Supernodes: %u",p_parent->GetSuperNodeListSize());
	SetDlgItemText(IDC_NUM_SUPERNODES, num);
}

//
//
//
void CSupernodeDistributerDlg::RefreshSupernodeList(IPAddress& ip_address)
{
	int index =0;
	char ip[16];
	char port[8];
	sprintf(ip,"%u.%u.%u.%u",(ip_address.m_ip>>0)&0xFF,(ip_address.m_ip>>8)&0xFF,
		(ip_address.m_ip>>16)&0xFF,(ip_address.m_ip>>24)&0xFF);
	sprintf(port,"%d",ip_address.m_port);
	index = m_ip_list.InsertItem(m_ip_list.GetItemCount(),ip);
	m_ip_list.SetItemData(index, ip_address.m_ip);
	m_ip_list.SetItemText(index,SUB_PORT,port);

	char num[128];
	sprintf(num,"Number of Supernodes: %u",p_parent->GetSuperNodeListSize());
	SetDlgItemText(IDC_NUM_SUPERNODES, num);
}
//
//
//
void CSupernodeDistributerDlg::RemoveSuperNodes(vector<IPAddress>& remove_list)
{
	int index = 0;
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	for(unsigned int i=0; i<remove_list.size();i++)
	{
		lvfi.lParam = remove_list[i].m_ip;
		index = ListView_FindItem(hList, -1, &lvfi);
		if(index >= 0)
			m_ip_list.DeleteItem(index);
	}
}

//
//
//
void CSupernodeDistributerDlg::RemoveSuperNodes(vector<IPAndPort>& remove_list)
{
	int index = 0;
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	for(unsigned int i=0; i<remove_list.size();i++)
	{
		lvfi.lParam = remove_list[i].m_ip;
		index = ListView_FindItem(hList, -1, &lvfi);
		if(index >= 0)
			m_ip_list.DeleteItem(index);
	}
}

//
//
//
void CSupernodeDistributerDlg::RemoveSuperNode(IPAndPort& remove_list)
{
	int index = 0;
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = remove_list.m_ip;
	index = ListView_FindItem(hList, -1, &lvfi);
	if(index >= 0)
		m_ip_list.DeleteItem(index);
}
//
//
//
void CSupernodeDistributerDlg::OnLvnColumnclickIpList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_ip_list;
	params[1]=pNMLV->iSubItem;

	// Sort the items depending on which column was clicked
	m_ip_list.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}

//
//
//
void CSupernodeDistributerDlg::RefreshSuperNodeListWithRack(char* rack_name, IPAndPort& new_ip)
{
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	int index = -1;
	lvfi.lParam = new_ip.m_ip;
	index = ListView_FindItem(hList, -1, &lvfi);
	if(index >= 0)
	{
/*		char rackname[16+1];
		if(strlen(rack_name)==0)
			strcpy(rackname,"0");
		else
		{
			sprintf(rackname,"%u.%u.%u.%u",(rack_ip>>0)&0xFF,(rack_ip>>8)&0xFF,
				(rack_ip>>16)&0xFF,(rack_ip>>24)&0xFF);
		}
*/		m_ip_list.SetItemText(index,SUB_RACK,rack_name);
	}
}

//
//
//
void CSupernodeDistributerDlg::RefreshSuperNodeListWithRack(char* rack_name, int ip)
{
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	int index = -1;
	lvfi.lParam = ip;
	index = ListView_FindItem(hList, -1, &lvfi);
	if(index >= 0)
	{
		m_ip_list.SetItemText(index,SUB_RACK,rack_name);
	}
}

//
//
//
void CSupernodeDistributerDlg::RefreshSuperNodeListWithRack(char* rack_name, vector<IPAndPort>& new_ips)
{
	HWND hList = m_ip_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	int index = -1;
	for(unsigned int i=0; i<new_ips.size(); i++)
	{
		lvfi.lParam = new_ips[i].m_ip;
		index = ListView_FindItem(hList, -1, &lvfi);
		if(index >= 0)
		{
/*			char rackname[16+1];
			if(rack_ip==0)
				strcpy(rackname,"0");
			else
			{
				sprintf(rackname,"%u.%u.%u.%u",(rack_ip>>0)&0xFF,(rack_ip>>8)&0xFF,
					(rack_ip>>16)&0xFF,(rack_ip>>24)&0xFF);
			}
			*/
			m_ip_list.SetItemText(index,SUB_RACK,rack_name);
		}
	}
}

//
//
//
/*
void CSupernodeDistributerDlg::ResizePage(void)
{
	if(IsWindowVisible())
	{
		CRect main_rect,list_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_IP_LIST)->GetWindowRect(&list_rect);

		list_rect.bottom=main_rect.bottom-m_border-m_bottom;
		list_rect.right=main_rect.right-m_border;

		ScreenToClient(&list_rect);

		GetDlgItem(IDC_IP_LIST)->MoveWindow(&list_rect);

		GetDlgItem(IDC_SHARE_FOLDER_STATIC)->GetWindowRect(&list_rect);
		list_rect.top += main_rect.bottom-m_bottom;
		GetDlgItem(IDC_SHARE_FOLDER_STATIC)->MoveWindow(&list_rect);
	}
}

//
//
//
BOOL CSupernodeDistributerDlg::OnSetActive()
{
	CRect main_rect,list_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_IP_LIST)->GetWindowRect(&list_rect);
	list_rect.bottom=main_rect.bottom-m_border-m_bottom;
	list_rect.right=main_rect.right-m_border;
	ScreenToClient(&list_rect);
	GetDlgItem(IDC_IP_LIST)->MoveWindow(&list_rect);

	return CPropertyPage::OnSetActive();
}
*/
//void CSupernodeDistributerDlg::OnStnClickedProbingIndex()
//{
//	// TODO: Add your control notification handler code here
//}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedAddFolderButton()
{
	UpdateData();
	if(m_share_folder.GetLength()!=0)
	{
		m_share_folder_list.InsertString(m_share_folder_list.GetCount(),m_share_folder);
		SaveFolderListToRegistry();
		p_parent->SendSharedFoldersToAllRacks(GetSharedFolderList());
	}
}

//
//
//
vector<CString> CSupernodeDistributerDlg::GetSharedFolderList(void)
{
	vector<CString> folders;
	for(int i=0; i<m_share_folder_list.GetCount();i++)
	{
		CString folder;
		m_share_folder_list.GetText(i, folder);
		folders.push_back(folder);
	}
	return folders;
}

//
//
//
void CSupernodeDistributerDlg::ReadFolderListFromRegistry(void)
{
	vector<CString> folders;
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\Trapper Keeper\\KazaaController\\SharedFolders";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			folders.push_back(szName);
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			cbData=sizeof(int);
			index++;			
		}
	}
	else
	{
		folders.push_back("C:\\Fasttrack Shared");
	}
	RegCloseKey(hKey);
	for(UINT i=0; i<folders.size();i++)
	{
		m_share_folder_list.InsertString(m_share_folder_list.GetCount(),folders[i]);
	}	
}


//
//
//
void CSupernodeDistributerDlg::SaveFolderListToRegistry(void)
{
	vector<CString> folders = GetSharedFolderList();
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\Trapper Keeper\\KazaaController\\SharedFolders";
	RegDeleteKey(HKEY_CURRENT_USER,subkey);
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	for(UINT i=0; i<folders.size();i++)
	{
		DWORD cbData=0;
		long l = RegSetValueEx(hKey,folders[i],0,REG_NONE,NULL,cbData);
	}
	RegCloseKey(hKey);
}

//
//
//
void CSupernodeDistributerDlg::ReadSupernodesResetHoursFromRegistry(void)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\Trapper Keeper\\KazaaController\\ResetSupernodesEvery";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		int val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Hours")==0)
			{
				m_reset_supernodes_hours=val;
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}
	else
	{
		m_reset_supernodes_hours = 0;
	}
	RegCloseKey(hKey);
	UpdateData(FALSE);
}


//
//
//
void CSupernodeDistributerDlg::SaveSupernodesResetHoursToRegistry(void)
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\Trapper Keeper\\KazaaController\\ResetSupernodesEvery";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Hours",0,REG_DWORD,(unsigned char *)&m_reset_supernodes_hours,cbData);
	RegCloseKey(hKey);
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedRemoveButton()
{
	int index = m_share_folder_list.GetCurSel();
	if(index == LB_ERR)
		return;	
	m_share_folder_list.DeleteString(index);
	SaveFolderListToRegistry();
	p_parent->SendSharedFoldersToAllRacks(GetSharedFolderList());
}

//
//
//
void CSupernodeDistributerDlg::OnBnClickedApplyButton()
{
	UpdateData(TRUE);
	SaveSupernodesResetHoursToRegistry();
	KillTimer(8); //reset supernodes hours
	if(m_reset_supernodes_hours>0)
		SetTimer(8, m_reset_supernodes_hours*3600*1000,NULL);
}
