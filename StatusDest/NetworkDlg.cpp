// NetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StatusDestDll.h"
#include "NetworkDlg.h"

// CNetworkDlg dialog

IMPLEMENT_DYNAMIC(CNetworkDlg, CDialog)
CNetworkDlg::CNetworkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkDlg::IDD, pParent)
{
	p_export_dlg = NULL;
}

CNetworkDlg::~CNetworkDlg()
{
	for(UINT i=0; i<v_shared_files_dlgs.size(); i++)
	{
		v_shared_files_dlgs[i]->DestroyWindow();
		delete *(v_shared_files_dlgs[i]);
	}
	v_shared_files_dlgs.clear();
}

void CNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_LIST, m_ip_list);
}


BEGIN_MESSAGE_MAP(CNetworkDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_IP_LIST, OnNMRclickIpList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_IP_LIST, OnLvnColumnclickIpList)
	ON_MESSAGE(WM_DONE_EDIT,OnDoneChangingMaxKazaa)
	ON_MESSAGE(WM_SHARED_FILES_DLG_QUIT,OnQuitSharedFilesDlg)
	ON_MESSAGE(WM_DONE_REMOVE_DLL_EDIT,OnDoneRemoveDllEdit)

	ON_NOTIFY(NM_DBLCLK, IDC_IP_LIST, OnNMDblclkIpList)
END_MESSAGE_MAP()


// CNetworkDlg message handlers

BOOL CNetworkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_edit_dlg.m_parent_hwnd = GetSafeHwnd();
	m_edit_dlg.Create(IDD_EDIT_DIALOG,this);

	m_remove_dll_edit_dlg.m_parent_hwnd = GetSafeHwnd();
	m_remove_dll_edit_dlg.Create(IDD_REMOVE_DLL_EDIT_DIALOG,this);

	m_ip_list.SetExtendedStyle(m_ip_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	CRect rect;
	m_ip_list.GetWindowRect(&rect);
	
	int nWidth = rect.Width();

	m_ip_list.InsertColumn(SUB_RACK_IP,"Host",LVCFMT_LEFT, nWidth * 6/113);
	m_ip_list.InsertColumn(SUB_COMPUTER_NAME,"Name",LVCFMT_LEFT, nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_MAC_ADDRESS,"MAC Address",LVCFMT_LEFT, nWidth * 9/113);
	m_ip_list.InsertColumn(SUB_RACK_VERSION,"Version",LVCFMT_CENTER,nWidth * 4/113);
	m_ip_list.InsertColumn(SUB_RACK_PROC,"CPU Usage",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_BANDWIDTH,"Bandwidth",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_AVG_BANDWIDTH, "Avg. Bandwidth",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_UPLOAD_BANDWIDTH, "Upload Bandwidth",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_DOWNLOAD_BANDWIDTH, "Download Bandwidth",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_KAZAA_COUNT,"# of KaZaA",LVCFMT_CENTER,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_KAZAA_UPLOADS,"KaZaA Uploads",LVCFMT_LEFT,nWidth * 5/113);
	m_ip_list.InsertColumn(SUB_RACK_KAZAA_MEMS,"KaZaA Memory Usages (MB)",LVCFMT_LEFT,nWidth * 4/113);
	m_ip_list.InsertColumn(SUB_RACK_FILES_SHARED,"Files Shared",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_MOVIES_SHARED,"Movies Shared",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_MUSIC_SHARED,"Music Shared",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_SWARMER_SHARED,"Swamer Shared",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_TK_MEMORY,"TK Memory",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_COMMITTED_MEMORY,"Committed Memory",LVCFMT_CENTER,nWidth *5/113);
	m_ip_list.InsertColumn(SUB_RACK_OS,"OS Version",LVCFMT_CENTER,nWidth *4/113);
	m_ip_list.InsertColumn(SUB_RACK_CPU_INFO,"CPU Type",LVCFMT_CENTER,nWidth *4/113);
	m_ip_list.InsertColumn(SUB_RACK_HARDDRIVE_SPACE_LEFT,"HDD Free Space",LVCFMT_CENTER,nWidth *4/113);
	m_ip_list.InsertColumn(SUB_RACK_TOTAL_HARDDRIVE_SPACE,"HDD Total Space",LVCFMT_CENTER,nWidth *4/113);
	m_ip_list.InsertColumn(SUB_RACK_TOTAL_MEMORY,"Memory Installed",LVCFMT_CENTER,nWidth *4/113);
	
	

	SetTimer(1,30*1000,NULL); //check to see if any computers are down
	SetTimer(2,5*60*1000,NULL); //get computer info
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CNetworkDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(IsWindowVisible())
	{
		
		CRect main_rect,list_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_IP_LIST)->GetWindowRect(&list_rect);
		
		int border=list_rect.left-main_rect.left;

		list_rect.bottom=main_rect.bottom-border;
		list_rect.right=main_rect.right-border;
		ScreenToClient(&list_rect);
		GetDlgItem(IDC_IP_LIST)->MoveWindow(&list_rect);
		
		CRect rect;
		m_ip_list.GetWindowRect(&rect);
		int nWidth = rect.Width();

		m_ip_list.SetColumnWidth(SUB_RACK_IP, nWidth * 6/113);
		m_ip_list.SetColumnWidth(SUB_COMPUTER_NAME, nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_MAC_ADDRESS, nWidth * 9/113);
		m_ip_list.SetColumnWidth(SUB_RACK_VERSION,nWidth * 4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_PROC,nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_BANDWIDTH,nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_AVG_BANDWIDTH,nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_UPLOAD_BANDWIDTH,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_DOWNLOAD_BANDWIDTH,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_KAZAA_COUNT,nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_KAZAA_UPLOADS,nWidth * 5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_KAZAA_MEMS,nWidth * 4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_FILES_SHARED,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_MOVIES_SHARED,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_MUSIC_SHARED,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_SWARMER_SHARED,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_TK_MEMORY,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_COMMITTED_MEMORY,nWidth *5/113);
		m_ip_list.SetColumnWidth(SUB_RACK_OS,nWidth *4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_CPU_INFO,nWidth *4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_HARDDRIVE_SPACE_LEFT,nWidth *4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_TOTAL_HARDDRIVE_SPACE,nWidth *4/113);
		m_ip_list.SetColumnWidth(SUB_RACK_TOTAL_MEMORY,nWidth *4/113);
	}
}

//
//
//
void CNetworkDlg::InitParent(StatusDestDll* parent)
{
	p_parent = parent;
}

//
//
//
void CNetworkDlg::UpdateIPList(vector<string> &ips)
{
	m_ip_list.DeleteAllItems();
	for(UINT i=0; i<ips.size(); i++)
	{
		int index = m_ip_list.InsertItem(m_ip_list.GetItemCount(), ips[i].c_str());
		int ip_int = 0;
		int ip1,ip2,ip3,ip4;
		sscanf(ips[i].c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
		ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
		m_ip_list.SetItemData(index, ip_int);

		p_parent->SendGenericMessage(ips[i].c_str(),StatusHeader::System_Info_Request); //send a system info request
	}
}

//
//
//
void CNetworkDlg::UpdateStatus(CString ip, StatusData* status, double avg_bandwidth)
{
	int index = -1;
	for(int i=0; i<m_ip_list.GetItemCount();i++)
	{
		CString cstring;
		cstring=m_ip_list.GetItemText(i,SUB_RACK_IP);
		if(strcmp(cstring,ip)==0)
		{
			index = i;
			break;
		}
		
	}
	if(index == -1)
		return;

	char buf[1024];
	sprintf(buf,"%u.%u.%u.%u", ((status->m_version)>>24)&0xFF,((status->m_version)>>16)&0xFF,((status->m_version)>>8)&0xFF,
		((status->m_version)>>0)&0xFF );
	m_ip_list.SetItemText(index,SUB_RACK_VERSION,buf);

	sprintf(buf,"%u %%",status->m_percent_processor_usage);
	m_ip_list.SetItemText(index,SUB_RACK_PROC,buf);

	sprintf(buf,"%.2f Mb/s",(((double)status->m_total_used_bandwidth)/((double)(1024*1024))));	// convert from bits/sec to Mbits/s
	m_ip_list.SetItemText(index,SUB_RACK_BANDWIDTH,buf);

	sprintf(buf,"%.2f Mb/s",(((double)status->m_sent_per_sec)/((double)(1024*1024))));	// convert from bits/sec to Mbits/s
	m_ip_list.SetItemText(index,SUB_RACK_UPLOAD_BANDWIDTH,buf);

	sprintf(buf,"%.2f Mb/s",(((double)status->m_received_per_sec)/((double)(1024*1024))));	// convert from bits/sec to Mbits/s
	m_ip_list.SetItemText(index,SUB_RACK_DOWNLOAD_BANDWIDTH,buf);

	sprintf(buf,"%.2f Mb/s",avg_bandwidth);
	m_ip_list.SetItemText(index,SUB_RACK_AVG_BANDWIDTH,buf);

	sprintf(buf,"%u/%u",status->m_kazaa_count,status->m_max_kazaa);
	m_ip_list.SetItemText(index,SUB_RACK_KAZAA_COUNT,buf);

	sprintf(buf,"%u", (status->m_movies_shared + status->m_music_shared + status->m_swarmer_shared) );
	m_ip_list.SetItemText(index,SUB_RACK_FILES_SHARED,buf);

	sprintf(buf,"%u",status->m_movies_shared);
	m_ip_list.SetItemText(index,SUB_RACK_MOVIES_SHARED,buf);

	sprintf(buf,"%u",status->m_music_shared);
	m_ip_list.SetItemText(index,SUB_RACK_MUSIC_SHARED,buf);
	
	sprintf(buf,"%u",status->m_swarmer_shared);
	m_ip_list.SetItemText(index,SUB_RACK_SWARMER_SHARED,buf);

	sprintf(buf,"%u MB", (UINT)((double)status->m_trapper_keeper_mem_usage/(double)(1024*1024)));
	m_ip_list.SetItemText(index,SUB_RACK_TK_MEMORY,buf);

	sprintf(buf,"%u MB",(UINT)((double)status->m_committed_memory/(double)(1024*1024)));
	m_ip_list.SetItemText(index,SUB_RACK_COMMITTED_MEMORY,buf);

	sprintf(buf,"%.2f GB",status->m_free_bytes);
	m_ip_list.SetItemText(index,SUB_RACK_HARDDRIVE_SPACE_LEFT,buf);

	memset(buf,0,sizeof(buf));
	for(UINT i=0;i<status->v_kazaa_mems.size();i++)
	{
		sprintf(&buf[strlen(buf)],"%u ", status->v_kazaa_mems[i]/(1024*1024));
	}
	m_ip_list.SetItemText(index,SUB_RACK_KAZAA_MEMS,buf);
}

//
//
//
void CNetworkDlg::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1)
	{
		CheckComputerStatus(); //see if any computers are down
	}
	else if(nIDEvent==2)
	{
		for(int i=0; i<m_ip_list.GetItemCount();i++)
		{
			p_parent->SendGenericMessage(m_ip_list.GetItemText(i,SUB_RACK_IP),StatusHeader::System_Info_Request); //send a system info request
		}
	}

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CNetworkDlg::CheckComputerStatus()
{
	CString cstring;
	GetWindowText(cstring);
	string network = cstring;
	for(int i=0;i<m_ip_list.GetItemCount();i++)
	{
		cstring = m_ip_list.GetItemText(i,SUB_RACK_IP);
		string ip = cstring;
		bool is_up = p_parent->IsComputerUp(network, ip);
		if(!is_up)
		{
			m_ip_list.SetItemText(i,SUB_RACK_VERSION,"");
			m_ip_list.SetItemText(i,SUB_RACK_PROC,"");
			m_ip_list.SetItemText(i,SUB_RACK_BANDWIDTH,"");
			m_ip_list.SetItemText(i,SUB_RACK_AVG_BANDWIDTH,"");
			m_ip_list.SetItemText(i,SUB_RACK_UPLOAD_BANDWIDTH,"");
			m_ip_list.SetItemText(i,SUB_RACK_DOWNLOAD_BANDWIDTH,"");
			m_ip_list.SetItemText(i,SUB_RACK_TK_MEMORY,"");
			m_ip_list.SetItemText(i,SUB_RACK_COMMITTED_MEMORY,"");
			m_ip_list.SetItemText(i,SUB_RACK_KAZAA_UPLOADS,"");
			m_ip_list.SetItemText(i,SUB_RACK_KAZAA_COUNT,"");
			m_ip_list.SetItemText(i,SUB_RACK_KAZAA_MEMS,"");
			m_ip_list.SetItemText(i,SUB_RACK_FILES_SHARED,"");
			m_ip_list.SetItemText(i,SUB_RACK_MOVIES_SHARED,"");
			m_ip_list.SetItemText(i,SUB_RACK_MUSIC_SHARED,"");
			m_ip_list.SetItemText(i,SUB_RACK_SWARMER_SHARED,"");
			m_ip_list.SetItemText(i,SUB_RACK_HARDDRIVE_SPACE_LEFT,"");

		}
	}
}

void CNetworkDlg::OnNMRclickIpList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_ip_list.GetSelectedCount()>0)
	{
		POINT point;
		GetCursorPos(&point);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING,WM_REMOTE_CONTROL,"Remote Control");
		menu.AppendMenu(MF_STRING,WM_STOP_KAZAA,"Stop Launching and Kill All Kazaa");
		//menu.AppendMenu(MF_STRING,WM_RESUME_KAZAA,"Resume Launching Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESTART_KAZAA,"Restart All Kazaa");
		menu.AppendMenu(MF_STRING,WM_CHANGE_MAX_KAZAA,"Change Maximum Number of Kazaa");
		menu.AppendMenu(MF_STRING,WM_RESTART_COMPUTER,"Restart Computer");
		menu.AppendMenu(MF_STRING,WM_REQUEST_KAZAA_UPLOADS,"Get Number of Kazaas' Uploads");
		menu.AppendMenu(MF_STRING,WM_VIEW_SHARED_FILES,"View Shared Files");
		menu.AppendMenu(MF_STRING,WM_EMERGENCY_RESTART,"Emergency Restart");
		menu.AppendMenu(MF_STRING,WM_EXPORT_INFO,"Export Rack Info to a File");
		menu.AppendMenu(MF_STRING,WM_REMOVE_DLL,"Remove File");
		menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this,0);
	}
	*pResult = 0;
}


//
//
//
BOOL CNetworkDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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
			case WM_REMOTE_CONTROL:
			{
				OnRemoteControl();
				break;
			}
			case WM_REQUEST_KAZAA_UPLOADS:
			{
				OnRequestKazaaUploads();
				break;
			}
			case WM_VIEW_SHARED_FILES:
			{
				OnViewSharedFiles();
				break;
			}
			case WM_EMERGENCY_RESTART:
			{
				if(MessageBox("Are you sure!?", "Seriously!!",MB_YESNO|MB_ICONQUESTION)==IDYES)
				OnEmergencyRestart();
				break;
			}
			case WM_EXPORT_INFO:
			{
				OnShowExportRackInfoDlg();
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
void CNetworkDlg::OnStopAndKillKazaa()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnStopAndKillKazaa(selected_racks);
	}
}

//
//
//
void CNetworkDlg::OnResumeKazaa()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnResumeKazaa(selected_racks);
	}
}

//
//
//
void CNetworkDlg::OnRestartKazaa()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRestartKazaa(selected_racks);
	}
}

//
//
//
void CNetworkDlg::OnRestartComputer()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRestartComputer(selected_racks);
	}
}

//
//
//
void CNetworkDlg::OnRequestKazaaUploads()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				m_ip_list.SetItemText(nItem, SUB_RACK_KAZAA_UPLOADS, "");
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRequestKazaaUploads(selected_racks);
	}
}

//
//
//
void CNetworkDlg::OnChangeMaxKazaa()
{
	m_edit_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
LRESULT CNetworkDlg::OnDoneChangingMaxKazaa(WPARAM wparam,LPARAM lparam)
{
	UINT max = (UINT)wparam;
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
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

	switch(sub_item)
	{
		case SUB_RACK_VERSION:
		case SUB_RACK_KAZAA_UPLOADS:
		case SUB_RACK_KAZAA_MEMS:
		case SUB_RACK_OS:
		case SUB_RACK_CPU_INFO:
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
		case SUB_RACK_IP:
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
		case SUB_RACK_PROC:
		case SUB_RACK_BANDWIDTH:
		case SUB_RACK_UPLOAD_BANDWIDTH:
		case SUB_RACK_DOWNLOAD_BANDWIDTH:
		case SUB_RACK_KAZAA_COUNT:
		case SUB_RACK_FILES_SHARED:
		case SUB_RACK_AVG_BANDWIDTH:
		case SUB_RACK_HARDDRIVE_SPACE_LEFT:
		case SUB_RACK_TOTAL_HARDDRIVE_SPACE:
		case SUB_RACK_TOTAL_MEMORY:
		case SUB_RACK_MOVIES_SHARED:
		case SUB_RACK_MUSIC_SHARED:
		case SUB_RACK_SWARMER_SHARED:
		case SUB_RACK_TK_MEMORY:
		case SUB_RACK_COMMITTED_MEMORY:
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
void CNetworkDlg::OnLvnColumnclickIpList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_ip_list;
	params[1]=pNMLV->iSubItem;
	m_ip_list.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}

//
//
//
void CNetworkDlg::UpdateSystemInfoStatus(char* ip, SystemInfoData* sys_info)
{
	int index = -1;
	for(int i=0; i<m_ip_list.GetItemCount();i++)
	{
		CString cstring;
		cstring=m_ip_list.GetItemText(i,SUB_RACK_IP);
		if(strcmp(cstring,ip)==0)
		{
			index = i;
			break;
		}
		
	}
	if(index == -1)
		return;

	char buf[1024];
	//cpu type
	m_ip_list.SetItemText(index,SUB_RACK_CPU_INFO,sys_info->m_cpu_info);
	
	//harddisk size
	sprintf(buf,"%.2f GB",sys_info->m_total_harddisk_space);
	m_ip_list.SetItemText(index,SUB_RACK_TOTAL_HARDDRIVE_SPACE,buf);

	//memory installed
	sprintf(buf,"%d MB",(sys_info->m_total_physical_memory / (1024*1024)));
	m_ip_list.SetItemText(index,SUB_RACK_TOTAL_MEMORY,buf);

	//OS version
	CString OS = "";

	switch (sys_info->m_os_info.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:

		// Test for the product.

		if ( sys_info->m_os_info.dwMajorVersion <= 4 )
			OS += "Microsoft Windows NT ";

		if ( sys_info->m_os_info.dwMajorVersion == 5 && sys_info->m_os_info.dwMinorVersion == 0 )
			OS += "Microsoft Windows 2000 ";

		if ( sys_info->m_os_info.dwMajorVersion == 5 && sys_info->m_os_info.dwMinorVersion == 1 )
			OS += "WinXP ";

		// Test for product type.

		if( sys_info->b_OsVersionInfoEx)
		{
			if ( sys_info->m_os_info.wProductType == VER_NT_WORKSTATION )
			{
				if( sys_info->m_os_info.wSuiteMask & VER_SUITE_PERSONAL )
					OS +=  "Personal ";
				else
					OS +=  "Pro ";
			}

			else if ( sys_info->m_os_info.wProductType == VER_NT_SERVER )
			{
				if( sys_info->m_os_info.wSuiteMask & VER_SUITE_DATACENTER )
					OS +=  "DataCenter Server ";
				else if( sys_info->m_os_info.wSuiteMask & VER_SUITE_ENTERPRISE )
					OS +=  "Advanced Server ";
				else
					OS +=  "Server ";
			}
		}
		else
		{
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );
			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				OS +=  "Professional ";
			if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				OS +=  "Server ";
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				OS +=  "Advanced Server ";
		}

		// Display version, service pack (if any), and build number.

		if ( sys_info->m_os_info.dwMajorVersion <= 4 )
		{
			char version[128];
			sprintf(version, "version %d.%d %s (Build %d)\n",
				sys_info->m_os_info.dwMajorVersion,
				sys_info->m_os_info.dwMinorVersion,
				sys_info->m_os_info.szCSDVersion,
				sys_info->m_os_info.dwBuildNumber & 0xFFFF);
			OS += version;
		}
		else
		{ 
			char version[128];
			sprintf(version,"%s (Build %d)",
				sys_info->m_os_info.szCSDVersion,
				sys_info->m_os_info.dwBuildNumber & 0xFFFF);
			OS += version;
		}
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		if (sys_info->m_os_info.dwMajorVersion == 4 && sys_info->m_os_info.dwMinorVersion == 0)
		{
			OS += "Microsoft Windows 95 ";
			if ( sys_info->m_os_info.szCSDVersion[1] == 'C' || sys_info->m_os_info.szCSDVersion[1] == 'B' )
				OS += "OSR2 ";
		} 

		if (sys_info->m_os_info.dwMajorVersion == 4 && sys_info->m_os_info.dwMinorVersion == 10)
		{
			OS += "Microsoft Windows 98 ";
			if ( sys_info->m_os_info.szCSDVersion[1] == 'A' )
				OS += "SE ";
		} 

		if (sys_info->m_os_info.dwMajorVersion == 4 && sys_info->m_os_info.dwMinorVersion == 90)
		{
			OS += "Microsoft Windows Me ";
		} 
		break;

	case VER_PLATFORM_WIN32s:

		OS += "Microsoft Win32s ";
		break;
	}

	CString mac_address, computer_name;
	//check for valid string
	if(strstr(sys_info->m_mac_address,"\0")!=NULL)
		mac_address = sys_info->m_mac_address;
	if(strstr(sys_info->m_computer_name,"\0")!=NULL)
		computer_name = sys_info->m_computer_name;

	m_ip_list.SetItemText(index,SUB_RACK_OS,OS);
	m_ip_list.SetItemText(index, SUB_MAC_ADDRESS,mac_address);
	m_ip_list.SetItemText(index, SUB_COMPUTER_NAME, computer_name);
}

//
//
//
void CNetworkDlg::UpdateKazaaUploadsStatus(char* from, int uploads)
{
	int index = -1;
	for(int i=0; i<m_ip_list.GetItemCount();i++)
	{
		CString cstring;
		cstring=m_ip_list.GetItemText(i,SUB_RACK_IP);
		if(strcmp(cstring,from)==0)
		{
			index = i;
			break;
		}
		
	}
	if(index == -1)
		return;

	char buf[1024];
	//cpu type
	m_ip_list.GetItemText(index, SUB_RACK_KAZAA_UPLOADS, buf, sizeof(buf));
	char str_uploads[32];
	memset(&str_uploads, 0, sizeof(str_uploads));
	itoa(uploads, str_uploads, 10);
	strcat(buf, " ");
	strcat(buf, str_uploads);
	m_ip_list.SetItemText(index,SUB_RACK_KAZAA_UPLOADS,buf);
}

//
//
//
void CNetworkDlg::OnRemoteControl(void)
{
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_NETWORK);
				
				char command[256];
				sprintf(command, "mstsc /v:%s /f", rack_name);
				PROCESS_INFORMATION pi;
				STARTUPINFOA si = {sizeof si};
				int bret = CreateProcess(NULL,command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
				if(bret != 0)
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
			}
		}
	}

}

//
//
//
void CNetworkDlg::OnViewSharedFiles(void)
{
	CString rack_name;
	CString network;
	GetWindowText(network);
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				CString title = "Shared Files on ";
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_NETWORK);
				title += rack_name;
				title += " (";
				title += network;
				title += ")";
				bool found = false;
				for(UINT i=0; i<v_shared_files_dlgs.size(); i++)
				{
					if(v_shared_files_dlgs[i]->m_ip.CompareNoCase(rack_name)==0)
					{
						found = true;
						v_shared_files_dlgs[i]->ShowWindow(SW_NORMAL);
						v_shared_files_dlgs[i]->BringWindowToTop();
						break;
					}
				}
				if(!found)
				{
					CSharedFilesDlg* dlg = new CSharedFilesDlg();
					dlg->Create(IDD_SHARED_FILES_DIALOG,CWnd::GetDesktopWindow());
					dlg->m_parent_hwnd = GetSafeHwnd();
					dlg->m_ip = rack_name;
					dlg->SetWindowText(title);
					dlg->ShowWindow(SW_NORMAL);
					v_shared_files_dlgs.push_back(dlg);
				}
				p_parent->SendGenericMessage(rack_name, StatusHeader::Request_Shared_Files);
			}
		}
	}
}

//
//
//
LRESULT CNetworkDlg::OnQuitSharedFilesDlg(WPARAM wparam, LPARAM lparam)
{
	vector<CSharedFilesDlg*>::iterator iter = v_shared_files_dlgs.begin();
	while(iter != v_shared_files_dlgs.end())
	{
		if( (*iter)->GetSafeHwnd() == (HWND)wparam)
		{
			(*iter)->DestroyWindow();
			delete (*iter);
			v_shared_files_dlgs.erase(iter);
			break;
		}
		iter++;
	}
	return 0;
}

//
//
//
void CNetworkDlg::ReceivedSharedFilesReply(char* from, FileInfos& reply)
{
	for(UINT i=0; i<v_shared_files_dlgs.size(); i++)
	{
		if(v_shared_files_dlgs[i]->m_ip.CompareNoCase(from)==0)
		{
			v_shared_files_dlgs[i]->ReceivedSharedFilesReply(reply);
			break;
		}
	}
}

//
//
//
void CNetworkDlg::OnEmergencyRestart()
{
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnEmergencyRestart(selected_racks);
	}
}

//
//
//
void CNetworkDlg::KillExportDlg(void)
{
	if(p_export_dlg != NULL)
	{
		p_export_dlg->DestroyWindow();
		delete p_export_dlg;
		p_export_dlg = NULL;
	}
}

//
//
//
void CNetworkDlg::OnExportRackInfo()
{
	if(p_export_dlg != NULL)
	{
//		int checked_count = 0;
		bool mac = p_export_dlg->m_mac_address_checked;
//		if(mac)
//			checked_count++;
		bool ip = p_export_dlg->m_ip_checked;
//		if(ip)
//			checked_count++;
		bool cpu = p_export_dlg->m_cpu_checked;
//		if(cpu)
//			checked_count++;
		bool memory = p_export_dlg->m_memory_checked;
//		if(memory)
//			checked_count++;
		bool hdd = p_export_dlg->m_hdd_checked;
//		if(hdd)
//			checked_count++;
		bool os = p_export_dlg->m_os_checked;
//		if(os)
//			checked_count++;
		bool computer_name = p_export_dlg->m_computer_name;
//		if(computer_name)
//			checked_count++;
		bool network = p_export_dlg->m_network;
//		if(network)
//			checked_count++;

		//Exporting to file
		CString filename;
		GetWindowText(filename);
		filename += ".txt";
		CStdioFile file;
		if(file.Open(filename, CFile::modeWrite|CFile::modeCreate|CFile::typeText)==TRUE)
		{
			for(int i=0; i<m_ip_list.GetItemCount(); i++)
			{
//				int checked_wrote = checked_count;
				if(ip)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_RACK_IP));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(computer_name)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_COMPUTER_NAME));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(mac)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_MAC_ADDRESS));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(network)
				{
					CString nw;
					GetWindowText(nw);
					file.WriteString(nw);
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(cpu)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_RACK_CPU_INFO));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(memory)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_RACK_TOTAL_MEMORY));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(hdd)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_RACK_TOTAL_HARDDRIVE_SPACE));
//					checked_wrote--;
//					if(checked_wrote > 0)
						file.WriteString("\t");
				}
				if(os)
				{
					file.WriteString(m_ip_list.GetItemText(i, SUB_RACK_OS));
					file.WriteString("\t");
				}
				file.WriteString("\n");
			}
			file.Close();
			char msg[128];
			sprintf(msg, "Exported to the file \"%s\"", filename);
			MessageBox(msg, "Done", MB_OK|MB_ICONINFORMATION);
		}
		p_export_dlg->DestroyWindow();
		delete p_export_dlg;
		p_export_dlg = NULL;
	}
}

//
//
//
void CNetworkDlg::OnShowExportRackInfoDlg()
{
	if(p_export_dlg == NULL)
	{
		p_export_dlg = new CExportRackInfoDlg(this);
		p_export_dlg->InitParent(this);
		p_export_dlg->Create(IDD_DIALOG1,this);
		p_export_dlg->ShowWindow(SW_NORMAL);
		p_export_dlg->BringWindowToTop();
	}
	else
	{
		p_export_dlg->BringWindowToTop();
	}
}

//
//
//
void CNetworkDlg::OnRemoveDll()
{
	m_remove_dll_edit_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
LRESULT CNetworkDlg::OnDoneRemoveDllEdit(WPARAM wparam,LPARAM lparam)
{
	const char * filename = (const char*)wparam;
	vector<CString> selected_racks;
	CString rack_name;
	if(m_ip_list.GetSelectedCount()>0)
	{
		POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_ip_list.GetNextSelectedItem(pos);
				rack_name = m_ip_list.GetItemText(nItem,SUB_RACK_IP);
				selected_racks.push_back(rack_name);
			}
		}
		p_parent->OnRemoveDll(selected_racks, filename);
	}
	return 0;
}

//
//
//
void CNetworkDlg::OnNMDblclkIpList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString rack_name;
	POSITION pos = m_ip_list.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int nItem = m_ip_list.GetNextSelectedItem(pos);
		rack_name = m_ip_list.GetItemText(nItem,SUB_NETWORK);
		
		char command[256];
		sprintf(command, "mstsc /v:%s /f", rack_name);
		PROCESS_INFORMATION pi;
		STARTUPINFOA si = {sizeof si};
		int bret = CreateProcess(NULL,command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
		if(bret != 0)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}
	*pResult = 0;
}
