// WinMxDcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinMxDcDlg.h"
#include "WinMxDcDll.h"

#define SUB_MOD								0
#define SUB_CONNECTED_CONNECTING_IDLE		1
#define SUB_PING_PONG_PUSH_QUERY_QUERYHIT	2


// CWinMxDcDlg dialog

IMPLEMENT_DYNAMIC(CWinMxDcDlg, CDialog)
CWinMxDcDlg::CWinMxDcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinMxDcDlg::IDD, pParent)
	, m_auto_pause_threshold(0)
	, m_auto_resume_threshold(0)
{
	m_pause_searching=false;
}

CWinMxDcDlg::~CWinMxDcDlg()
{
}

void CWinMxDcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
	DDX_Control(pDX, IDC_Module_Connection_List, m_module_connection_list);
	DDX_Control(pDX, IDC_Project_Keyword_Tree, m_project_keyword_tree);
	DDX_Control(pDX, IDC_Project_Status_List, m_project_status_list);
	DDX_Control(pDX, IDC_SUPPLY_PROGRESS, m_supply_progress);
	DDX_Text(pDX, IDC_auto_pause_threshold_EDIT, m_auto_pause_threshold);
	DDX_Text(pDX, IDC_auto_resume_threshold_EDIT, m_auto_resume_threshold);
	DDV_MinMaxInt(pDX, m_auto_pause_threshold, 0, 100);
	DDV_MinMaxInt(pDX, m_auto_resume_threshold, 0, 100);
	DDX_Control(pDX, IDC_SUPPLY_PROGRESS2, m_supply_entry_progress);
}


BEGIN_MESSAGE_MAP(CWinMxDcDlg, CDialog)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnBnClickedPauseButton)
	ON_BN_CLICKED(IDC_RECONNECT_STATIC, OnBnClickedReconnectStatic)
	ON_BN_CLICKED(IDC_Reset_Project_Status_List, OnBnClickedResetProjectStatusList)
	ON_BN_CLICKED(IDC_RESTART_SEARCHING_BUTTON, OnBnClickedRestartSearchingButton)
	ON_BN_CLICKED(IDC_PROCESS_BUTTON, OnBnClickedProcessButton)
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_Min_Module_Count_Spin, OnDeltaposMinModuleCountSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Max_Module_Count_Spin, OnDeltaposMaxModuleCountSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Supply_Interval_Multiplier_Spin, OnDeltaposSupplyIntervalMultiplierSpin)
	ON_MESSAGE(WM_INIT_DB_MANAGER_SUPPLY_THREAD_DATA,InitDBManagerSupplyThreadData)
	ON_MESSAGE(WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB,ReadyToWriteDataToSupplyDatabase)
	ON_MESSAGE(WM_SUPPLY_PROCESS_PROJECT, SupplyProcessProject)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,InitProcessManagerMaintenanceThreadData)
	ON_MESSAGE(WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE,ProcessManagerMaintenanceDone)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_SUPPLY_THREAD_DATA,InitProcessManagerSupplyThreadData)
	ON_MESSAGE(WM_PROCESS_MANAGER_SUPPLY_DONE, ProcessManagerSupplyDone)
	ON_BN_CLICKED(IDC_APPLY_AUTO_PAUSE_THRESHOLD_BUTTON, OnBnClickedApplyAutoPauseThresholdButton)
	ON_MESSAGE(WM_INIT_SUPPLY_MANAGER_THREAD_DATA, InitSupplyManagerThreadData)
	ON_MESSAGE(WM_SUPPLY_MANAGER_UPDATE_SUPPLY_PROJECTS,UpdateSupplyManagerSupplyProjects)
	ON_MESSAGE(WM_SUPPLY_MANAGER_THREAD_PROGRESS,SupplyManagerThreadProgress)
	ON_MESSAGE(WM_SUPPLY_MANAGER_THREAD_PROJECT,SupplyanagerThreadProject)
	ON_MESSAGE(WM_SUPPLY_MANAGER_SUPPLY_RETREIVAL_FAILED,SupplyManagerSupplyRetrievalFailed)
	ON_BN_CLICKED(IDC_FILE_TRANSFER_BUTTON, OnBnClickedFileTransferButton)
END_MESSAGE_MAP()


// CWinMxDcDlg message handlers
//
//
//
void CWinMxDcDlg::Log(const char* log)
{
	CString msg = (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - ");
	msg += log;
	if(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
	m_log_list.InsertString(0,msg);
}

//
//
//
void CWinMxDcDlg::InitParent(WinMxDcDll* parent)
{
	p_parent = parent;
	char buf[256];
	sprintf(buf,"%u",p_parent->AlterModuleCounts(-1,-1));
	GetDlgItem(IDC_Min_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",p_parent->AlterModuleCounts(1,1));
	GetDlgItem(IDC_Max_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",p_parent->GetSupplyIntervalMultiplier());
	GetDlgItem(IDC_Supply_Interval_Multiplier_Static)->SetWindowText(buf);

}

//
//
//
void CWinMxDcDlg::OnBnClickedPauseButton()
{
	m_pause_searching = !m_pause_searching;
	p_parent->ManualPauseSearching(m_pause_searching);
	CString text;
	if(m_pause_searching)
	{
		text="Resume Searching";
		GetDlgItem(IDC_MANUAL_PAUSE_STATIC)->SetWindowText("Manual Searching State: Pause");
	}
	else
	{
		text="Pause Searching";
		GetDlgItem(IDC_MANUAL_PAUSE_STATIC)->SetWindowText("Manual Searching State: Searching");
	}
	GetDlgItem(IDC_PAUSE_BUTTON)->SetWindowText(text);
}

//
//
//
void CWinMxDcDlg::OnBnClickedReconnectStatic()
{
	p_parent->ReConnectToAllSupernodes();
}

//
//
//
void CWinMxDcDlg::OnBnClickedResetProjectStatusList()
{
	m_project_status_list.DeleteAllItems();
	m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);	// refresh
}

//
//
//
void CWinMxDcDlg::OnBnClickedRestartSearchingButton()
{
	p_parent->RestartSearching();
}

//
//
//
void CWinMxDcDlg::OnBnClickedProcessButton()
{
	p_parent->StartDataProcess();
}

//
//
//
BOOL CWinMxDcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ft_dlg.Create(IDD_File_Transfer_DIALOG,CWnd::GetDesktopWindow());
	m_ft_dlg.InitParent(this);
	m_ft_dlg.ShowWindow(SW_HIDE);

	m_auto_pause_threshold = 50;
	m_auto_resume_threshold = 30;
	UpdateData(FALSE);

	// TODO:  Add extra initialization here
	m_log_list.SetHorizontalExtent(1000);
	InitRunningSinceWindowText();

	CRect rect;
	m_module_connection_list.GetWindowRect(&rect);
	int nWidth = rect.Width();
	m_module_connection_list.InsertColumn(SUB_MOD,"M",LVCFMT_LEFT,nWidth *47/242);
	m_module_connection_list.InsertColumn(SUB_CONNECTED_CONNECTING_IDLE,"+   0   -",LVCFMT_CENTER,nWidth *80/242);
	m_module_connection_list.InsertColumn(SUB_PING_PONG_PUSH_QUERY_QUERYHIT,"Pi Po Pu Qu QH",LVCFMT_CENTER,nWidth *115/242);

	m_project_status_list.GetWindowRect(&rect);
	nWidth = rect.Width();
	m_project_status_list.InsertColumn(SUB_PS_PROJECT,"Project",LVCFMT_LEFT,nWidth *175/500);
	//m_project_status_list.InsertColumn(SUB_PS_WRITTEN,"Written",LVCFMT_LEFT,nWidth *20/500);
	m_project_status_list.InsertColumn(SUB_PS_QUERY_HIT,"QH",LVCFMT_CENTER,nWidth *45/500);
	m_project_status_list.InsertColumn(SUB_PS_DECOY,"Decoys",LVCFMT_CENTER,nWidth *45/500);
	m_project_status_list.InsertColumn(SUB_PS_EFFICIENCY,"Efficiency (%)",LVCFMT_CENTER,nWidth *60/500);
	m_project_status_list.InsertColumn(SUB_PS_TRACK_QUERY_HIT,"QH / Track",LVCFMT_LEFT,nWidth *175/500);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CWinMxDcDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CWinMxDcDlg::OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts((-1)*pNMUpDown->iDelta,0);
	*pResult = 0;
}

//
//
//
void CWinMxDcDlg::OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts(0,(-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CWinMxDcDlg::OnDeltaposSupplyIntervalMultiplierSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterSupplyIntervalMultiplier((-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

//
//
//
void CWinMxDcDlg::AlterModuleCounts(int dmin,int dmax)
{
	p_parent->AlterModuleCounts(dmin,dmax);	

	int min=p_parent->AlterModuleCounts(-1,-1);
	int max=p_parent->AlterModuleCounts(1,1);

	// Update both values
	char buf[256];
	sprintf(buf,"%u",min);
	GetDlgItem(IDC_Min_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",max);
	GetDlgItem(IDC_Max_Modules_Static)->SetWindowText(buf);

	// See if we have more modules than needed displayed
	while(m_module_connection_list.GetItemCount()>max)
	{
		m_module_connection_list.DeleteItem(m_module_connection_list.GetItemCount()-1);
	}
}

//
//
//
void CWinMxDcDlg::InitRunningSinceWindowText()
{
	// Update the time that the values were last cleared
	CTime time;
	time=CTime::GetCurrentTime();
	CString time_str = time.Format("Running since %A %m/%d/%Y at %H:%M");

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
//
//
//
void CWinMxDcDlg::ReportConnectionStatus(ConnectionModuleStatusData &status)//,vector<ConnectionModuleStatusData> *all_mod_status)
{
	UINT i;
	char buf[1024+1];

	// See if this mod is already represented in the list
	int index=-1;
	for(i=0;i<(UINT)m_module_connection_list.GetItemCount();i++)
	{
		if(m_module_connection_list.GetItemData(i)==status.m_mod)
		{
			index=i;
			break;
		}
	}

	// If we didn't find it, then add a new item
	if(index==-1)
	{
		sprintf(buf,"%02u",status.m_mod);
		index=m_module_connection_list.InsertItem(m_module_connection_list.GetItemCount(),buf,0);
		m_module_connection_list.SetItemData(index,status.m_mod);
	}

	sprintf(buf,"%02u %02u %02u",status.m_connected_socket_count,status.m_connecting_socket_count,status.m_idle_socket_count);
	m_module_connection_list.SetItemText(index,SUB_CONNECTED_CONNECTING_IDLE,buf);

	// Compile the ping,pong,push,query,query-hit counts
	unsigned int pi=0,po=0,pu=0,qu=0,qh=0;
	
	pi+=status.m_ping_count;
	po+=status.m_pong_count;
	pu+=status.m_push_count;
	qu+=status.m_query_count;
	qh+=status.m_query_hit_count;

	sprintf(buf,"%u : %u : %u : %u : %u",pi,po,pu,qu,qh);
	m_module_connection_list.SetItemText(index,SUB_PING_PONG_PUSH_QUERY_QUERYHIT,buf);

	bool refresh=false;
	for(i=0;i<status.v_project_status.size();i++)
	{
		if((status.v_project_status[i].m_total_query_hit_count)>0 || (status.v_project_status[i].m_total_fake_query_hit_count>0))
		{
			refresh=true;
			if(m_project_status_list.IncrementCounters(status.v_project_status[i]))
				p_parent->StopSearchingProject(status.v_project_status[i].ProjectName());
		}
	}

	// Update the virtual list control
	if(refresh)
	{
		m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);
	}
}

//
//
//
void CWinMxDcDlg::ModuleCountHasChanged(int count)
{
	// See if we have more modules than needed displayed
	while(m_module_connection_list.GetItemCount()>count)
	{
		m_module_connection_list.DeleteItem(m_module_connection_list.GetItemCount()-1);
	}
}

//
//
//
void CWinMxDcDlg::KeywordsUpdated(vector<ProjectKeywords>* keywords)
{
	UINT i,j,k;
	
	m_project_keyword_tree.DeleteAllItems();

	HTREEITEM item1,item2,item3,item4;
	char buf[1024];

	for(i=0;i<keywords->size();i++)
	{
		// Project name
		item1=m_project_keyword_tree.InsertItem((*keywords)[i].m_project_name.c_str(),0,0,TVI_ROOT);
		sprintf(buf,"Project ID: %u",(*keywords)[i].m_id);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Artist: %s",(*keywords)[i].m_artist_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Album: %s",(*keywords)[i].m_album_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Owner: %s",(*keywords)[i].m_owner.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		item4=m_project_keyword_tree.InsertItem("Viewers",0,0,item1);
		for(j=0;j<(*keywords)[i].m_viewers.v_strings.size();j++)
		{
			m_project_keyword_tree.InsertItem((*keywords)[i].m_viewers.v_strings[j].c_str(),0,0,item4);
		}

		sprintf(buf,"Version: %u",(*keywords)[i].m_version);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);



		// Flags
		item2=m_project_keyword_tree.InsertItem("Flags",0,0,item1);

		sprintf(buf,"Project Active : %u",(*keywords)[i].m_project_active);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Auto Multiplier : %u",(*keywords)[i].m_auto_multiplier_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BearShare Disting : %u",(*keywords)[i].m_bearshare_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"GUID Spoofing : %u",(*keywords)[i].m_guid_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Poisoning : %u",(*keywords)[i].m_poisoning_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Uber Disting: %u",(*keywords)[i].m_uber_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Emule Spoofing: %u",(*keywords)[i].m_emule_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Swarming: %u",(*keywords)[i].m_emule_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Decoying: %u",(*keywords)[i].m_emule_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Supply: %u",(*keywords)[i].m_emule_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"DirectConnect Decoying: %u",(*keywords)[i].m_directconnect_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Spoofing: %u",(*keywords)[i].m_directconnect_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Supply: %u",(*keywords)[i].m_directconnect_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"FastTrack Decoying: %u",(*keywords)[i].m_fasttrack_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Spoofing: %u",(*keywords)[i].m_fasttrack_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Swarming: %u",(*keywords)[i].m_fasttrack_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Supply: %u",(*keywords)[i].m_fasttrack_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Find More: %u",(*keywords)[i].m_find_more);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gnutella Supply: %u",(*keywords)[i].m_gnutella_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Demand: %u",(*keywords)[i].m_gnutella_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Decoying: %u",(*keywords)[i].m_gnutella_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Spoofing: %u",(*keywords)[i].m_gnutella_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Swarming: %u",(*keywords)[i].m_gnutella_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);


		sprintf(buf,"Piolet Spoofing: %u",(*keywords)[i].m_piolet_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Demand: %u",(*keywords)[i].m_piolet_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Supply: %u",(*keywords)[i].m_piolet_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gift Spoofing: %u",(*keywords)[i].m_gift_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Decoys: %u",(*keywords)[i].m_gift_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Supply: %u",(*keywords)[i].m_gift_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"WinMX Supply: %u",(*keywords)[i].m_winmx_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Spoofing: %u",(*keywords)[i].m_winmx_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Swarming: %u",(*keywords)[i].m_winmx_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Decoying: %u",(*keywords)[i].m_winmx_decoy_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		//Kazaa Search Type
		char type[32];
		switch((*keywords)[i].m_search_type)
		{
			case ProjectKeywords::search_type::everything:
			{
				strcpy(type, "Everything");
				break;
			}
			case ProjectKeywords::search_type::audio:
			{
				strcpy(type, "Audio");
				break;
			}
			case ProjectKeywords::search_type::video:
			{
				strcpy(type, "Video");
				break;
			}
			case ProjectKeywords::search_type::images:
			{
				strcpy(type, "Images");
				break;
			}
			case ProjectKeywords::search_type::documents:
			{
				strcpy(type, "Documents");
				break;
			}
			case ProjectKeywords::search_type::software:
			{
				strcpy(type, "Software");
				break;
			}
			case ProjectKeywords::search_type::playlists:
			{
				strcpy(type, "Playlists");
				break;
			}
			case ProjectKeywords::search_type::none:
			{
				strcpy(type, "None");
				break;
			}
		}
		sprintf(buf,"FastTrack Search Type : %s",type);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"FastTrack Dist Power : %d",(*keywords)[i].m_kazaa_dist_power);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Hash Count
		item2=m_project_keyword_tree.InsertItem("FastTrack Hash Count",0,0,item1);
		for(j=0;j<(*keywords)[i].v_tracks_hash_counts.size();j++)
		{
			sprintf(buf,"%2d : %d",(*keywords)[i].v_tracks_hash_counts[j].m_track, (*keywords)[i].v_tracks_hash_counts[j].m_hash_count);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}

		// Query multiplier
		sprintf(buf,"Query Multiplier : %u",(*keywords)[i].m_query_multiplier);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Supply Interval
		sprintf(buf,"Supply Interval : %u",(*keywords)[i].m_interval);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Query keywords
		item2=m_project_keyword_tree.InsertItem("Query Keywords",0,0,item1);
		// Query Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Weight",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",(*keywords)[i].m_query_keywords.v_keywords[j].keyword.c_str(),(*keywords)[i].m_query_keywords.v_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Keywords
		item3=m_project_keyword_tree.InsertItem("Exact Keywords : Weight",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",(*keywords)[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),(*keywords)[i].m_query_keywords.v_exact_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_query_keywords.v_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Killwords
		item3=m_project_keyword_tree.InsertItem("Exact Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_exact_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_query_keywords.v_exact_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}

		// Supply Keywords
		item2=m_project_keyword_tree.InsertItem("Supply Keywords",0,0,item1);
		sprintf(buf,"Search String : %s",(*keywords)[i].m_supply_keywords.m_search_string.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Supply Size Threshold : %u",(*keywords)[i].m_supply_keywords.m_supply_size_threshold);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		// Supply Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Track",0,0,item2);
		for(j=0;j<(*keywords)[i].m_supply_keywords.v_keywords.size();j++)
		{
			buf[0]='\0';
			for(k=0;k<(*keywords)[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
			{
				strcat(buf,(*keywords)[i].m_supply_keywords.v_keywords[j].v_keywords[k]);
				strcat(buf," ");
			}
			sprintf(&buf[strlen(buf)],": %u",(*keywords)[i].m_supply_keywords.v_keywords[j].m_track);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Supply Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_supply_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_supply_keywords.v_killwords[j].v_keywords[0]);	// kinda kludgey *&*
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		
		// QRP Keywords
		item2=m_project_keyword_tree.InsertItem("QRP Keywords",0,0,item1);
		for(j=0;j<(*keywords)[i].m_qrp_keywords.v_keywords.size();j++)
		{
			m_project_keyword_tree.InsertItem((*keywords)[i].m_qrp_keywords.v_keywords[j].c_str(),0,0,item2);
		}

		// Poisoners
		item2=m_project_keyword_tree.InsertItem("Poisoners",0,0,item1);
		for(j=0;j<(*keywords)[i].v_poisoners.size();j++)
		{
			unsigned int ip=(*keywords)[i].v_poisoners[j].GetIPInt();
			unsigned int port=(*keywords)[i].v_poisoners[j].GetPort();
			sprintf(buf,"%u.%u.%u.%u : %u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}


	}

	// Whenever the keywords are updated, reset the status list in case any projects were removed
	//OnBnClickedResetProjectStatusList();

	// Reset the keyword count
	if(keywords->size()==1)
	{
		sprintf(buf,"Project Keywords : ( %u Project )",keywords->size());
	}
	else
	{
		sprintf(buf,"Project Keywords : ( %u Projects )",keywords->size());
	}
	GetDlgItem(IDC_Project_Keywords_Static)->SetWindowText(buf);
}

//
//
//
void CWinMxDcDlg::ClearProjectStatus(string &project)
{
	m_project_status_list.ClearProjectStatus(project);
	m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);	// refresh
}

//
//
//
void CWinMxDcDlg::AlterSupplyIntervalMultiplier(int dmulti)
{
	p_parent->AlterSupplyIntervalMultiplier(dmulti);	

	int multi=p_parent->GetSupplyIntervalMultiplier();

	// Update value
	char buf[256];
	sprintf(buf,"%u",multi);
	GetDlgItem(IDC_Supply_Interval_Multiplier_Static)->SetWindowText(buf);
}

//
//
//
LRESULT CWinMxDcDlg::InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitDBManagerSupplyThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam)
{
	// Make sure it is not the initial message that the thread sends
	if((wparam!=1) && (lparam!=1))
	{
		char msg[1024];
		CTime now=CTime::GetCurrentTime();
		sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
		GetDlgItem(IDC_Logfile_Supply_Data_Last_Written)->SetWindowText(msg);
	}

	p_parent->ReadyToWriteDataToSupplyDatabase(wparam,lparam);
	return 0;
}

//
//
//
UINT CWinMxDcDlg::GetBandwidthInBytes()
{
	return p_parent->GetBandwidthInBytes();
}

//
//
//
UINT CWinMxDcDlg::GetBandwdithInBitsPerSecond()
{
	return p_parent->GetBandwdithInBitsPerSecond();
}

//
//
//
LRESULT CWinMxDcDlg::SupplyProcessProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
void CWinMxDcDlg::UpdateDataProcessProgressBar(int supply)
{
	m_supply_progress.SetPos(supply);

	char msg[128];
	sprintf(msg, "%d %%", supply);
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText(msg);
}

//
//
//
LRESULT CWinMxDcDlg::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{

	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_ENDED_STATIC)->SetWindowText(str);

	char msg[1024];
	sprintf(msg, "%u records deleted from supply table", (UINT)wparam);
	GetDlgItem(IDC_SUPPLY_DELETED_STATIC)->SetWindowText(msg);

	p_parent->ProcessManagerMaintenanceDone(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_STARTED_STATIC)->SetWindowText(str);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam)
{
	CString done = (CTime::GetCurrentTime()).Format("Last Supply Process Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_SUPPLY_PROCESS_ENDED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText("");

	m_supply_progress.SetPos(100);
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText("100 %");
	p_parent->ProcessManagerSupplyDone(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	CString done = (CTime::GetCurrentTime()).Format("Last Supply Process Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_SUPPLY_PROCESS_STARTED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText("0 %");
	m_supply_progress.SetPos(0);
	
	p_parent->InitProcessManagerSupplyThreadData(wparam,lparam);
	return 0;
}

//
//
//
void CWinMxDcDlg::AutoPause(bool pause)
{
	if(pause)
		GetDlgItem(IDC_AUTO_PAUSE_STATIC)->SetWindowText("Auto Searching State: Pause");
	else
		GetDlgItem(IDC_AUTO_PAUSE_STATIC)->SetWindowText("Auto Searching State: Searching");
}

//
//
//
void CWinMxDcDlg::OnBnClickedApplyAutoPauseThresholdButton()
{
	if(UpdateData())
		p_parent->ApplyAutoSearchingThreshold(m_auto_pause_threshold,m_auto_resume_threshold);
}

//
//
//
void CWinMxDcDlg::SpoofEntriesRetrievalStarted()
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_STARTED_STATIC)->SetWindowText(str);
}

//
//
//
LRESULT CWinMxDcDlg::InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	m_supply_entry_progress.SetPos(0);
	p_parent->InitSupplyManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC)->SetWindowText(str);
	GetDlgItem(IDC_SUPPLY_ENTRY_STATIC)->SetWindowText("");
	m_supply_entry_progress.SetPos(100);
	//p_parent->UpdateSupplyManagerSupplyProjects(wparam,lparam);
	p_parent->DoneRetreivingSpoofEntries();
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::SupplyanagerThreadProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SUPPLY_ENTRY_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::SupplyManagerThreadProgress(WPARAM wparam,LPARAM lparam)
{
	int percent = (int)((float)wparam / (float)lparam* (float)100);
	m_supply_entry_progress.SetPos(percent);

	char msg[128];
	sprintf(msg, "%d %%", percent);
	GetDlgItem(IDC_SUPPLY_ENTRY_PERCENT_STATIC)->SetWindowText(msg);
	//p_parent->SendSupplyRetrievalProgress(percent);
	return 0;
}

//
//
//
LRESULT CWinMxDcDlg::SupplyManagerSupplyRetrievalFailed(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Failed at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC)->SetWindowText(str);
	p_parent->SupplyManagerSupplyRetrievalFailed();
	return 0;
}

//
//
//
void CWinMxDcDlg::UpdateSpoofSize(UINT spoof_size, UINT spoof_index, UINT round, UINT hash_size)
{
	char buf[128];
	sprintf(buf,"Spoof Entries Size: %u", spoof_size);
	GetDlgItem(IDC_SPOOF_SIZE_STATIC)->SetWindowText(buf);
	sprintf(buf,"Spoof Index: %u                    Round: %u", spoof_index, round);
	GetDlgItem(IDC_SPOOF_INDEX_STATIC)->SetWindowText(buf);
	sprintf(buf,"Hash Table Size: %u", hash_size);
	GetDlgItem(IDC_HASH_SIZE_STATIC)->SetWindowText(buf);
}

//
//
//
void CWinMxDcDlg::OnBnClickedFileTransferButton()
{
	m_ft_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
void CWinMxDcDlg::ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status)
{
	m_ft_dlg.ReportStatus(status);
}

//
//
//
int CWinMxDcDlg::GetModCount()
{
	return p_parent->GetModCount();
}

//
//
//
void CWinMxDcDlg::OnUnInitialize()
{
	m_ft_dlg.OnUnInitialize();
}