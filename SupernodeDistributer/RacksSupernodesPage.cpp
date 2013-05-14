// RacksSupernodesPage.cpp : implementation file
//

#include "stdafx.h"
#include "RacksSupernodesPage.h"
#include "SupernodeDistributerDll.h"

// CRacksSupernodesPage dialog
IMPLEMENT_DYNAMIC(CRacksSupernodesPage, CPropertyPage)
CRacksSupernodesPage::CRacksSupernodesPage()
	: CPropertyPage(CRacksSupernodesPage::IDD)
{
}

CRacksSupernodesPage::~CRacksSupernodesPage()
{
}

void CRacksSupernodesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RACK_LIST, m_rack_list);
	DDX_Control(pDX, IDC_SUPERNODE_LIST, m_supernode_list);
}


BEGIN_MESSAGE_MAP(CRacksSupernodesPage, CPropertyPage)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_RACK_LIST, OnNMClickRackList)
//	ON_MESSAGE(WM_DONE_EDITING, OnDoneEditing)
	ON_NOTIFY(NM_RCLICK, IDC_RACK_LIST, OnNMRclickRackList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_RACK_LIST, OnLvnKeydownRackList)
END_MESSAGE_MAP()


// CRacksSupernodesPage message handlers

BOOL CRacksSupernodesPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	//ShowWindow(SW_NORMAL);

	m_rack_list.m_parent_hwnd = GetSafeHwnd();

	CRect main_rect,list_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_RACK_LIST)->GetWindowRect(&list_rect);
	m_border=list_rect.left-main_rect.left;
	m_bottom=main_rect.bottom - list_rect.bottom;

	m_rack_list.SetExtendedStyle(m_rack_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_rack_list.InsertColumn(SUB_RACK_NAME,"Rack Name",LVCFMT_LEFT,100);
//	m_rack_list.InsertColumn(SUB_NUM_KAZAA,"Number of Kazaa",LVCFMT_LEFT,100);
	
	m_supernode_list.SetExtendedStyle(m_supernode_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_supernode_list.InsertColumn(0,"Supernode's IP",LVCFMT_LEFT,200);
	m_supernode_list.InsertColumn(1,"Port",LVCFMT_LEFT,50);
	m_supernode_list.InsertColumn(2,"Up Time",LVCFMT_LEFT,150);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CRacksSupernodesPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	ResizePage();
}

//
//
//
void CRacksSupernodesPage::InitParent(SupernodeDistributerDll* parent)
{
	p_parent = parent;
}

//
//
//
void CRacksSupernodesPage::ResizePage(void)
{
	if(IsWindowVisible())
	{
		CRect main_rect,list1_rect,list2_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_RACK_LIST)->GetWindowRect(&list1_rect);
		GetDlgItem(IDC_SUPERNODE_LIST)->GetWindowRect(&list2_rect);

		list1_rect.bottom=main_rect.bottom-m_border-m_bottom;
		list1_rect.right=((main_rect.right-main_rect.left)/3)+list1_rect.left-m_border;

		list2_rect.bottom=main_rect.bottom-m_border-m_bottom;
		list2_rect.left=list1_rect.right+m_border;
		list2_rect.right=main_rect.right-m_border;

		ScreenToClient(&list1_rect);
		ScreenToClient(&list2_rect);

		GetDlgItem(IDC_RACK_LIST)->MoveWindow(&list1_rect);
		GetDlgItem(IDC_SUPERNODE_LIST)->MoveWindow(&list2_rect);
	}
}

//
//
//
BOOL CRacksSupernodesPage::OnSetActive()
{
	CRect main_rect,list1_rect,list2_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_RACK_LIST)->GetWindowRect(&list1_rect);
	GetDlgItem(IDC_SUPERNODE_LIST)->GetWindowRect(&list2_rect);
	list1_rect.bottom=main_rect.bottom-m_border-m_bottom;
	list1_rect.right=((main_rect.right-main_rect.left)/3)+list1_rect.left-m_border;
	list2_rect.bottom=main_rect.bottom-m_border-m_bottom;
	list2_rect.left=list1_rect.right+m_border;
	list2_rect.right=main_rect.right-m_border;
	ScreenToClient(&list1_rect);
	ScreenToClient(&list2_rect);
	GetDlgItem(IDC_RACK_LIST)->MoveWindow(&list1_rect);
	GetDlgItem(IDC_SUPERNODE_LIST)->MoveWindow(&list2_rect);

	return CPropertyPage::OnSetActive();
}

//
//
//
void CRacksSupernodesPage::OnNMClickRackList(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	//check if the user select any item
	UINT selected = m_rack_list.GetSelectedCount();
	UINT kazaa_running = 0;
	if(selected > 0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		int index = m_rack_list.GetNextSelectedItem(pos);
		string rack_name = m_rack_list.GetItemText(index, 0);
		vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

		//fill up the supernodes list
		m_supernode_list.DeleteAllItems();
		for(unsigned int i=0; i<supernodes.size(); i++)
		{
			char ip[16+1];
			char port[8];
			itoa(supernodes[i].m_port, port, 10);
			GetIPStringFromInterger(supernodes[i].m_ip, ip);
			int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
			m_supernode_list.SetItemText(inserted, 1, port);
			CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
			m_supernode_list.SetItemText(inserted, 2, up_time);
		}
	}
	
	char msg[32];
	sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
	GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
	sprintf(msg, "Number of Kazaa: %u", kazaa_running);
	GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
	*pResult = 0;
}

//
//
//
void CRacksSupernodesPage::GetIPStringFromInterger(int ip_int, char* ip)
{
	sprintf(ip, "%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
}

//
//
//
void CRacksSupernodesPage::AddRack(RackSuperNodes& rack)
{
	int index = m_rack_list.InsertItem(m_rack_list.GetItemCount(),rack.m_rack_name.c_str());
/*
	char num[4];
	sprintf(num, "%d", rack.m_num_kazaa);
	m_rack_list.SetItemText(index, SUB_NUM_KAZAA, num);
*/
	char msg[32];
	sprintf(msg, "Number of Racks: %d", m_rack_list.GetItemCount());
	GetDlgItem(IDC_RACKS_NUM)->SetWindowText(msg);

}

//
//
//
//done editing kazaa number field
/*
LRESULT CRacksSupernodesPage::OnDoneEditing(WPARAM wparam,LPARAM lparam)
{
	char numbers[8];
	char rack_name[32];
	int num;
	m_rack_list.GetItemText((int)wparam,(int)lparam,numbers,8);
	m_rack_list.GetItemText((int)wparam,SUB_RACK_NAME,rack_name,32);
	num = atoi(numbers);
	p_parent->OnEditKazaaNumber(rack_name, num);
	return 0;
}
*/
//
//
//
void CRacksSupernodesPage::OnNMRclickRackList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_rack_list.GetSelectedCount()>0)
	{
		POINT point;
		GetCursorPos(&point);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING,WM_STOP_KAZAA,"Stop Launching and Kill All Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESUME_KAZAA,"Resume Launching Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESTART_KAZAA,"Restart All Kazaa");
		menu.AppendMenu(MF_STRING,WM_REFRESH_KAZAA_LIST,"Retrieve Remote Supernodes");
		menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this,0);
	}
	*pResult = 0;
}

//
//
//
BOOL CRacksSupernodesPage::OnCommand(WPARAM wParam, LPARAM lParam)
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
			case WM_REFRESH_KAZAA_LIST:
			{
				OnGetRemoteSupernodes();
				break;
			}
		}
	}
	
	return CDialog::OnCommand(wParam,lParam);
	return CPropertyPage::OnCommand(wParam, lParam);
}

//
//
//
void CRacksSupernodesPage::OnStopAndKillKazaa()
{
	//erase current supernode list on the dialog
	m_supernode_list.DeleteAllItems();
	vector<CString> selected_racks;
	CString rack_name;
	if(m_rack_list.GetSelectedCount()>0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_rack_list.GetNextSelectedItem(pos);
				rack_name = m_rack_list.GetItemText(nItem,SUB_RACK_NAME);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnStopAndKillKazaa(selected_racks);
	}
}

//
//
//
void CRacksSupernodesPage::OnResumeKazaa()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_rack_list.GetSelectedCount()>0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_rack_list.GetNextSelectedItem(pos);
				rack_name = m_rack_list.GetItemText(nItem,SUB_RACK_NAME);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnResumeKazaa(selected_racks);
	}
}

//
//
//
void CRacksSupernodesPage::OnRestartKazaa()
{
	//erase current supernode list on the dialog
	m_supernode_list.DeleteAllItems();
	vector<CString> selected_racks;
	CString rack_name;
	if(m_rack_list.GetSelectedCount()>0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_rack_list.GetNextSelectedItem(pos);
				rack_name = m_rack_list.GetItemText(nItem,SUB_RACK_NAME);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRestartKazaa(selected_racks);
	}
}

//
//
//
void CRacksSupernodesPage::OnGetRemoteSupernodes()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_rack_list.GetSelectedCount()>0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_rack_list.GetNextSelectedItem(pos);
				rack_name = m_rack_list.GetItemText(nItem,SUB_RACK_NAME);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnGetRemoteSupernodes(selected_racks);
	}
}
/*
//
//
//
void CRacksSupernodesPage::UpdateRackMaxKazaaNum(RackSuperNodes& rack)
{
	int count = m_rack_list.GetItemCount();
	for(int i=0; i<count; i++)
	{
		CString rack_name = m_rack_list.GetItemText(i, SUB_RACK_NAME);
		if(strcmp(rack.m_rack_name.c_str(), rack_name)==0)
		{
			char num[8];
			sprintf(num, "%d", rack.m_num_kazaa);
			m_rack_list.SetItemText(i, SUB_NUM_KAZAA, num);
			break;
		}
	}
}
*/
//
//
//
void CRacksSupernodesPage::OnLvnKeydownRackList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	switch(pLVKeyDow->wVKey)
	{
		case VK_UP:
		{
			//check if the user select any item
			UINT selected = m_rack_list.GetSelectedCount();
			UINT kazaa_running = 0;
			if(selected > 0)
			{
				POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
				int index = m_rack_list.GetNextSelectedItem(pos);
				index--;
				if(index < 0)
					index = 0;
				string rack_name = m_rack_list.GetItemText(index, 0);
				vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

				//fill up the supernodes list
				m_supernode_list.DeleteAllItems();
				for(unsigned int i=0; i<supernodes.size(); i++)
				{
					char ip[16+1];
					char port[8];
					itoa(supernodes[i].m_port, port, 10);
					GetIPStringFromInterger(supernodes[i].m_ip, ip);
					int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
					m_supernode_list.SetItemText(inserted, 1, port);
					CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
					m_supernode_list.SetItemText(inserted, 2, up_time);
				}
			}
			
			char msg[32];
			sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
			GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
			sprintf(msg, "Number of Kazaa: %u", kazaa_running);
			GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
			break;
		}
		case VK_DOWN:
		{
			//check if the user select any item
			UINT selected = m_rack_list.GetSelectedCount();
			UINT kazaa_running = 0;
			if(selected > 0)
			{
				POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
				int index = m_rack_list.GetNextSelectedItem(pos);
				index++;
				if(index >= m_rack_list.GetItemCount())
					index = m_rack_list.GetItemCount()-1;
				string rack_name = m_rack_list.GetItemText(index, 0);
				vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

				//fill up the supernodes list
				m_supernode_list.DeleteAllItems();
				for(unsigned int i=0; i<supernodes.size(); i++)
				{
					char ip[16+1];
					char port[8];
					itoa(supernodes[i].m_port, port, 10);
					GetIPStringFromInterger(supernodes[i].m_ip, ip);
					int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
					m_supernode_list.SetItemText(inserted, 1, port);
					CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
					m_supernode_list.SetItemText(inserted, 2, up_time);
				}
			}
			
			char msg[32];
			sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
			GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
			sprintf(msg, "Number of Kazaa: %u", kazaa_running);
			GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
			break;
		}
	}
	*pResult = 0;
}
