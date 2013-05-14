// StatusDestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StatusDestDlg.h"
#include "StatusDestDll.h"

#define WM_STOP_KAZAA			WM_USER_MENU+1
#define WM_RESUME_KAZAA			WM_USER_MENU+2
#define WM_RESTART_KAZAA		WM_USER_MENU+3
#define WM_CHANGE_MAX_KAZAA		WM_USER_MENU+4
#define WM_RESTART_COMPUTER		WM_USER_MENU+5
#define WM_EMERGENCY_RESTART	WM_USER_MENU+9
#define WM_OPEN_ALL_NETWORK_DLG	WM_USER_MENU+10

// CStatusDestDlg dialog

IMPLEMENT_DYNAMIC(CStatusDestDlg, CDialog)
CStatusDestDlg::CStatusDestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusDestDlg::IDD, pParent)
{
}

CStatusDestDlg::~CStatusDestDlg()
{
}

void CStatusDestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Network_List, m_network_list);
}


BEGIN_MESSAGE_MAP(CStatusDestDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDC_Network_List, OnNMDblclkNetworkList)
	ON_NOTIFY(NM_RCLICK, IDC_Network_List, OnNMRclickNetworkList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_Network_List, OnLvnColumnclickNetworkList)
	ON_MESSAGE(WM_DONE_EDIT,OnDoneChangingMaxKazaa)
	ON_MESSAGE(WM_DONE_REMOVE_DLL_EDIT,OnDoneRemoveDllEdit)
	//ON_MESSAGE(WM_INIT_STATUS_DEST_THREAD_DATA,OnInitStatusDestThreadData)
	ON_MESSAGE(WM_UPDATE_NETWORK_SUMMARY,OnUpdateNetworkSummary)


	ON_BN_CLICKED(IDC_REBOOT_BUTTON, OnBnClickedRebootButton)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_Network_List, OnLvnItemchangedNetworkList)
END_MESSAGE_MAP()


// CStatusDestDlg message handlers

void CStatusDestDlg::InitParent(StatusDestDll* parent)
{
	p_parent = parent;
}

BOOL CStatusDestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//this->MoveWindow(0,0);

	m_edit_dlg.m_parent_hwnd = GetSafeHwnd();
	m_edit_dlg.Create(IDD_EDIT_DIALOG,this);

	m_remove_dll_edit_dlg.m_parent_hwnd = GetSafeHwnd();
	m_remove_dll_edit_dlg.Create(IDD_REMOVE_DLL_EDIT_DIALOG,this);

	m_network_list.SetExtendedStyle(m_network_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	
	CRect rect;
	m_network_list.GetWindowRect(&rect);
	int nWidth = rect.Width()-5;
	m_network_list.InsertColumn(SUB_NETWORK,"Network",LVCFMT_LEFT,nWidth*55/500);
	m_network_list.InsertColumn(SUB_COMPUTERS,"Total Racks Up",LVCFMT_CENTER,nWidth*45/500);
	//m_network_list.InsertColumn(SUB_COMPUTERS_UP,"Total Racks Alive",LVCFMT_CENTER,nWidth*80/500);
	m_network_list.InsertColumn(SUB_PROC,"CPU Usage",LVCFMT_CENTER,nWidth*35/500);
	m_network_list.InsertColumn(SUB_BANDWIDTH,"Bandwidth Usage",LVCFMT_CENTER,nWidth*65/500);
	m_network_list.InsertColumn(SUB_UPLOAD_BANDWIDTH,"Upload Bandwidth",LVCFMT_CENTER,nWidth*65/500);
	m_network_list.InsertColumn(SUB_DOWNLOAD_BANDWIDTH,"Download Bandwidth",LVCFMT_CENTER,nWidth*65/500);
	m_network_list.InsertColumn(SUB_AVG_FILES_SHARED,"Avg Files Shared",LVCFMT_CENTER,nWidth*65/500);
	m_network_list.InsertColumn(SUB_IP_RANGE, "IP Range", LVCFMT_LEFT,nWidth*105/500);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CStatusDestDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(IsWindowVisible())
	{
		CRect main_rect,list_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_Network_List)->GetWindowRect(&list_rect);
		
		int border=list_rect.left-main_rect.left;

		list_rect.bottom=main_rect.bottom-border;
		list_rect.right=main_rect.right-border;
		ScreenToClient(&list_rect);
		GetDlgItem(IDC_Network_List)->MoveWindow(&list_rect);
		
		CRect rect;
		m_network_list.GetWindowRect(&rect);
		int nWidth = rect.Width()-5;
		m_network_list.SetColumnWidth(SUB_NETWORK,nWidth*55/500);
		m_network_list.SetColumnWidth(SUB_COMPUTERS,nWidth*45/500);
		//m_network_list.SetColumnWidth(SUB_COMPUTERS_UP,nWidth*100/500);
		m_network_list.SetColumnWidth(SUB_PROC,nWidth*35/500);
		m_network_list.SetColumnWidth(SUB_BANDWIDTH,nWidth*65/500);
		m_network_list.SetColumnWidth(SUB_UPLOAD_BANDWIDTH,nWidth*65/500);
		m_network_list.SetColumnWidth(SUB_DOWNLOAD_BANDWIDTH,nWidth*65/500);
		m_network_list.SetColumnWidth(SUB_AVG_FILES_SHARED,nWidth*65/500);
		m_network_list.SetColumnWidth(SUB_IP_RANGE,nWidth*105/500);
	}
}

//
//
//
void CStatusDestDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CStatusDestDlg::AddNetwork(string& network)
{
	int index = m_network_list.InsertItem(m_network_list.GetItemCount(),network.c_str());
	m_network_list.SetItemData(index, index);
}

//
//
//
void CStatusDestDlg::AddIPRange(CString network, CString ip_range)
{
	for(int i=0; i<m_network_list.GetItemCount(); i++)
	{
		CString this_network = m_network_list.GetItemText(i, SUB_NETWORK);
		if(network.CompareNoCase(this_network)==0)
		{
			m_network_list.SetItemText(i, SUB_IP_RANGE,ip_range);
			break;
		}
	}
}

//
//
//
void CStatusDestDlg::OnNMDblclkNetworkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_network_list.GetSelectedCount() > 0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		int index = m_network_list.GetNextSelectedItem(pos);
		CString network = m_network_list.GetItemText(index, SUB_NETWORK);
		p_parent->OpenNetworkDialog(network);
	}
	*pResult = 0;
}

//
//
//
void CStatusDestDlg::UpdateStatus(string network, UINT total_computers, UINT total_computers_up, UINT bandwidth, UINT cpu_usage,
								  UINT upload_bandwidth, UINT download_bandwidth, UINT files_shared)
{
	char buf[1024];
	for(int i=0; i<m_network_list.GetItemCount(); i++)
	{
		CString cstring;
		cstring=m_network_list.GetItemText(i,SUB_NETWORK);
		if(strcmp(cstring,network.c_str())==0)
		{
			sprintf(buf,"%.2f Mb/s",((double)bandwidth)/(((double)1024)*((double)1024)));
			m_network_list.SetItemText(i,SUB_BANDWIDTH,buf);

			sprintf(buf,"%.2f Mb/s",((double)upload_bandwidth)/(((double)1024)*((double)1024)));
			m_network_list.SetItemText(i,SUB_UPLOAD_BANDWIDTH,buf);

			sprintf(buf,"%.2f Mb/s",((double)download_bandwidth)/(((double)1024)*((double)1024)));
			m_network_list.SetItemText(i,SUB_DOWNLOAD_BANDWIDTH,buf);

			sprintf(buf,"%u %",cpu_usage);
			m_network_list.SetItemText(i,SUB_PROC,buf);

			sprintf(buf,"%u",files_shared);
			m_network_list.SetItemText(i,SUB_AVG_FILES_SHARED,buf);

			sprintf(buf, "%u/%u", total_computers_up,total_computers);
			m_network_list.SetItemText(i,SUB_COMPUTERS,buf);
			
			/*
			sprintf(buf, "%u", total_computers_up);
			m_network_list.SetItemText(i,SUB_COMPUTERS_UP,buf);
			*/

			break;
		}
	}
	UpdateSummary();
}

//
//
//

void CStatusDestDlg::OnNMRclickNetworkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_network_list.GetSelectedCount()>0)
	{
		POINT point;
		GetCursorPos(&point);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING,WM_STOP_KAZAA,"Stop Launching and Kill All Kazaa");
		//menu.AppendMenu(MF_STRING,WM_RESUME_KAZAA,"Resume Launching Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESTART_KAZAA,"Restart All Kazaa");
		menu.AppendMenu(MF_STRING,WM_CHANGE_MAX_KAZAA,"Change Maximum Number of Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESTART_COMPUTER,"Restart Computer");
		menu.AppendMenu(MF_STRING,WM_EMERGENCY_RESTART,"Emergency Restart");
		menu.AppendMenu(MF_STRING,WM_OPEN_ALL_NETWORK_DLG,"Open All Networks Status Windows");
		menu.AppendMenu(MF_STRING,WM_REMOVE_DLL,"Remove File");
		menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this,0);
	}
	*pResult = 0;
}


//
//
//
BOOL CStatusDestDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Check for right-click popup menu items
	if(wParam>WM_USER_MENU)
	{
		switch(wParam)
		{
			case WM_STOP_KAZAA:
			{
				OnStopAndKillKazaa();
				break;
			}
			case WM_RESUME_KAZAA:
			{
				OnResumeKazaa();
				break;
			}
			case WM_RESTART_KAZAA:
			{
				OnRestartKazaa();
				break;
			}
			case WM_CHANGE_MAX_KAZAA:
			{
				OnChangeMaxKazaa();
				break;
			}
			case WM_RESTART_COMPUTER:
			{
				if(MessageBox("Are you sure!?", "Seriously!!",MB_YESNO|MB_ICONQUESTION)==IDYES)
					OnRestartComputer();
				break;
			}
			case WM_EMERGENCY_RESTART:
			{
				if(MessageBox("Are you sure!?", "Seriously!!",MB_YESNO|MB_ICONQUESTION)==IDYES)
					OnEmergencyRestart();
				break;
			}
			case WM_OPEN_ALL_NETWORK_DLG:
			{
				for(int i=0; i<m_network_list.GetItemCount();i++)
				{
					CString network = m_network_list.GetItemText(i, SUB_NETWORK);
					CString ip_range = m_network_list.GetItemText(i, SUB_IP_RANGE);
					if(ip_range.GetLength() > 0)
						p_parent->OpenNetworkDialog(network);
				}
				break;
			}
			case WM_REMOVE_DLL:
			{
				OnRemoveDll();
				break;
			}
		}
	}
	
	return CDialog::OnCommand(wParam,lParam);
}

//
//
//
void CStatusDestDlg::OnStopAndKillKazaa()
{
	vector<CString> selected_networks;
	CString network;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				network = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_networks.push_back(network);
			}
		}
		p_parent->OnStopAndKillKazaa(selected_networks);
	}
}

//
//
//
void CStatusDestDlg::OnResumeKazaa()
{
	vector<CString> selected_networks;
	CString network;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				network = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_networks.push_back(network);
			}
		}
		p_parent->OnResumeKazaa(selected_networks);
	}
}

//
//
//
void CStatusDestDlg::OnRestartKazaa()
{
	vector<CString> selected_networks;
	CString network;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				network = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_networks.push_back(network);
			}
		}
		p_parent->OnRestartKazaa(selected_networks);
	}
}

//
//
//
void CStatusDestDlg::OnRestartComputer()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				rack_name = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRestartComputer(selected_racks);
	}
}

//
//
//
void CStatusDestDlg::OnEmergencyRestart()
{
	vector<CString> selected_networks;
	CString rack_name;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				rack_name = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_networks.push_back(rack_name);
			}
		}
		p_parent->OnEmergencyRestartNetwork(selected_networks);
	}
}

//
//
//
void CStatusDestDlg::OnChangeMaxKazaa()
{
	m_edit_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
void CStatusDestDlg::OnRemoveDll()
{
	m_remove_dll_edit_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
LRESULT CStatusDestDlg::OnDoneChangingMaxKazaa(WPARAM wparam,LPARAM lparam)
{
	UINT max = (UINT)wparam;
	vector<CString> selected_racks;
	CString rack_name;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				rack_name = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnChangeMaxKazaa(selected_racks, max);
	}
	return 0;
}

//
//
//
LRESULT CStatusDestDlg::OnDoneRemoveDllEdit(WPARAM wparam,LPARAM lparam)
{
	const char* filename = (const char*)wparam;
	vector<CString> selected_racks;
	CString rack_name;
	if(m_network_list.GetSelectedCount()>0)
	{
		POSITION pos = m_network_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_network_list.GetNextSelectedItem(pos);
				rack_name = m_network_list.GetItemText(nItem,SUB_NETWORK);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRemoveDllNetwork(selected_racks, filename);
	}
	return 0;
}

//
//
//
int CALLBACK StatusCompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
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

	switch(sub_item)
	{
		case SUB_NETWORK:
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
		case SUB_COMPUTERS:
		//case SUB_COMPUTERS_UP:
		case SUB_PROC:
		case SUB_BANDWIDTH:
		case SUB_UPLOAD_BANDWIDTH:
		case SUB_DOWNLOAD_BANDWIDTH:
		case SUB_AVG_FILES_SHARED:
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
void CStatusDestDlg::OnLvnColumnclickNetworkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_network_list;
	params[1]=pNMLV->iSubItem;
	m_network_list.SortItems(StatusCompareFunc,(DWORD_PTR)params);	*pResult = 0;
}

//
//
//
/*
LRESULT CStatusDestDlg::OnInitStatusDestThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->OnInitStatusDestThreadData(wparam, lparam);
	return 0;
}
*/
//
//
//
LRESULT CStatusDestDlg::OnUpdateNetworkSummary(WPARAM wparam,LPARAM lparam)
{
	p_parent->OnUpdateNetworkSummary(wparam, lparam);
	return 0;
}

//
//
//
void CStatusDestDlg::OnBnClickedRebootButton()
{
	p_parent->RebootAllDownRacks();
}

//
//
//
void CStatusDestDlg::UpdateSummary()
{
	int total_up=0;
	int total_down=0;
	int total=0;

	for(int i=0; i<m_network_list.GetItemCount();i++)
	{
		int up=0;
		int sub_total=0;
		CString str = m_network_list.GetItemText(i, SUB_COMPUTERS);
		sscanf(str, "%d/%d", &up,&sub_total);
		total_up+=up;
		total+=sub_total;
	}
	char msg[128];
	sprintf(msg, "%d", total_up);
	GetDlgItem(IDC_COMPUTERS_UP)->SetWindowText(msg);
	total_down = total-total_up;
	sprintf(msg, "%d", total_down);
	GetDlgItem(IDC_COMPUTERS_DOWN)->SetWindowText(msg);
	sprintf(msg, "%d", total_down+total_up);
	GetDlgItem(IDC_TOTAL_COMPUTERS)->SetWindowText(msg);
}
void CStatusDestDlg::OnLvnItemchangedNetworkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
