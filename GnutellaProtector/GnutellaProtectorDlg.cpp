// GnutellaProtectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GnutellaProtectorDlg.h"
#include "GnutellaProtectorDll.h"

#define SUB_MOD								0
#define SUB_CONNECTED_CONNECTING_IDLE		1
#define SUB_PING_PONG_PUSH_QUERY_QUERYHIT	2
#define SUB_CACHED_QUERY_HITS				3

#define SUB_SNV_NUM		0
#define SUB_SNV_VENDOR	1

// CGnutellaProtectorDlg dialog

IMPLEMENT_DYNAMIC(CGnutellaProtectorDlg, CDialog)
CGnutellaProtectorDlg::CGnutellaProtectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGnutellaProtectorDlg::IDD, pParent)
	, m_add_host(_T(""))
	, m_vendor_counts_enabled(FALSE)
{
}

CGnutellaProtectorDlg::~CGnutellaProtectorDlg()
{
}

void CGnutellaProtectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Project_Status_List, m_project_status_list);
	DDX_Control(pDX, IDC_Module_Connection_List, m_module_connection_list);
	DDX_Control(pDX, IDC_Project_Keyword_Tree, m_project_keyword_tree);
	DDX_Control(pDX, IDC_Supernode_Vendor_List, m_supernode_vendor_list);
	DDX_Control(pDX, IDC_SPOOF_PROGRESS, m_spoof_progress);
	DDX_Control(pDX, IDC_SUPPLY_PROGRESS, m_supply_progress);
	DDX_Text(pDX, IDC_HOST_EDIT, m_add_host);
	DDX_Check(pDX, IDC_VENDOR_COUNTS_CHECK, m_vendor_counts_enabled);
}


BEGIN_MESSAGE_MAP(CGnutellaProtectorDlg, CDialog)
	ON_BN_CLICKED(IDC_Show_Log_Window, OnBnClickedShowLogWindow)
	ON_BN_CLICKED(IDC_Reset_Project_Status_List, OnBnClickedResetProjectStatusList)
	ON_BN_CLICKED(IDC_DATA_PROCESS_BUTTON, OnBnClickedDataProcessButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Min_Module_Count_Spin, OnDeltaposMinModuleCountSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Max_Module_Count_Spin, OnDeltaposMaxModuleCountSpin)
	ON_WM_TIMER()
//	ON_MESSAGE(WM_SUPPLY_MANAGER_THREAD_DATA_READY,SupplyManagerThreadDataReady)
//	ON_MESSAGE(WM_INIT_LOGFILE_MANAGER_THREAD_DATA,InitLogfileManagerThreadData)
//	ON_MESSAGE(WM_READY_TO_WRITE_LOGFILE_DATA_TO_FILE,ReadyToWriteLogfileDataToFile)
	ON_MESSAGE(WM_INIT_DB_MANAGER_THREAD_DATA,InitDBManagerThreadData)
	ON_MESSAGE(WM_READY_TO_WRITE_DATA_TO_DB,ReadyToWriteDataToDatabase)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_THREAD_DATA,InitProcessManagerThreadData)
	ON_MESSAGE(WM_PROCESST_MANAGER_DONE, ProcessManagerDone)
	ON_MESSAGE(WM_SUPPLY_PROCESS_PROJECT, SupplyProcessProject)
	ON_MESSAGE(WM_DEMAND_PROCESS_PROJECT, DemandProcessProject)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,InitProcessManagerMaintenanceThreadData)
	ON_MESSAGE(WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE,ProcessManagerMaintenanceDone)
	ON_MESSAGE(WM_INIT_SUPPLY_MANAGER_THREAD_DATA,InitSupplyManagerThreadData)
	ON_MESSAGE(WM_UPDATE_SUPPLY_MANAGER_SUPPLY_PROJECTS,UpdateSupplyManagerSupplyProjects)
	ON_MESSAGE(WM_SUPPLY_MANAGER_THREAD_PROJECT,SupplyManagerThreadProject)
	ON_MESSAGE(WM_SUPPLY_MANAGER_THREAD_PROGRESS,SupplyManagerThreadProgress)
	ON_MESSAGE(WM_SUPPLY_MANAGER_RETRIEVING_SUPPLY_ABORTED,SupplyManagerThreadAborted)

	ON_BN_CLICKED(IDC_Retrieve_BUTTON, OnBnClickedRetrieveButton)
	ON_BN_CLICKED(IDC_ADD_HOST_BUTTON, OnBnClickedAddHostButton)
	ON_BN_CLICKED(IDC_VENDOR_COUNTS_CHECK, OnBnClickedVendorCountsCheck)
END_MESSAGE_MAP()


// CGnutellaProtectorDlg message handlers
void CGnutellaProtectorDlg::InitParent(GnutellaProtectorDll* parent)
{
	p_parent = parent;
	char buf[256];
	sprintf(buf,"%u",p_parent->AlterModuleCounts(-1,-1));
	GetDlgItem(IDC_Min_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",p_parent->AlterModuleCounts(1,1));
	GetDlgItem(IDC_Max_Modules_Static)->SetWindowText(buf);
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedShowLogWindow()
{
	p_parent->ShowLogWindow();
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedResetProjectStatusList()
{
	m_project_status_list.DeleteAllItems();
	m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);	// refresh
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedDataProcessButton()
{
	p_parent->StartDataProcess();
}


//
//
//
void CGnutellaProtectorDlg::OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts((-1)*pNMUpDown->iDelta,0);
	*pResult = 0;
}


//
//
//
void CGnutellaProtectorDlg::OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts(0,(-1)*pNMUpDown->iDelta);
	*pResult = 0;
}


//
//
//
BOOL CGnutellaProtectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_spoof_progress.SetRange(0,100);
	m_supply_progress.SetRange(0,100);

	CRect rect;
	m_module_connection_list.GetWindowRect(&rect);
	int nWidth = rect.Width();
	m_module_connection_list.InsertColumn(SUB_MOD,"M",LVCFMT_LEFT,nWidth *22/242);
	m_module_connection_list.InsertColumn(SUB_CONNECTED_CONNECTING_IDLE,"+   0   -",LVCFMT_CENTER,nWidth *55/242);
	m_module_connection_list.InsertColumn(SUB_PING_PONG_PUSH_QUERY_QUERYHIT,"Pi Po Pu Qu QH",LVCFMT_CENTER,nWidth *90/242);
	m_module_connection_list.InsertColumn(SUB_CACHED_QUERY_HITS,"Cached QH",LVCFMT_LEFT,nWidth *75/242);

	m_supernode_vendor_list.GetWindowRect(&rect);
	nWidth = rect.Width();
	m_supernode_vendor_list.InsertColumn(SUB_SNV_NUM,"Num",LVCFMT_LEFT,nWidth *40/260);
	m_supernode_vendor_list.InsertColumn(SUB_SNV_VENDOR,"Vendor",LVCFMT_LEFT,nWidth *220/260);

	m_project_status_list.GetWindowRect(&rect);
	nWidth = rect.Width();
	m_project_status_list.InsertColumn(SUB_PS_PROJECT,"Project",LVCFMT_LEFT,nWidth *150/500);
	m_project_status_list.InsertColumn(SUB_PS_SPOOFS_DISTS_POISONS,"S : D : P",LVCFMT_CENTER,nWidth *100/500);
	m_project_status_list.InsertColumn(SUB_PS_QUERY,"Qu",LVCFMT_CENTER,nWidth *50/500);
	m_project_status_list.InsertColumn(SUB_PS_TRACK_QUERY,"Qu / Track",LVCFMT_LEFT,nWidth *200/500);
//	m_project_status_list.InsertColumn(SUB_PS_QUERY_HIT,"QH",LVCFMT_CENTER,nWidth *50/925);
//	m_project_status_list.InsertColumn(SUB_PS_TRACK_QUERY_HIT,"QH / Track",LVCFMT_LEFT,nWidth *375/925);

	InitRunningSinceWindowText();

	m_DlgResizer.InitResizer(this);
	
	m_DlgResizer.SizeItem(IDC_Module_Connection_List, CDlgResizer::Down);
	m_DlgResizer.ListCtrlItem(IDC_Module_Connection_List);

	m_DlgResizer.SizeItem(IDC_Supernode_Vendor_List, CDlgResizer::DownAndRight);
	m_DlgResizer.ListCtrlItem(IDC_Supernode_Vendor_List);

	m_DlgResizer.SizeItem(IDC_Project_Keyword_Tree, CDlgResizer::Down);

	m_DlgResizer.MoveSizeItem(IDC_Project_Status_List, CDlgResizer::Down,CDlgResizer::Right);
	m_DlgResizer.ListCtrlItem(IDC_Project_Status_List);

	m_DlgResizer.MoveItem(IDC_Reset_Project_Status_List,   CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_DATA_PROCESS_BUTTON,   CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Computer_Status_Static,   CDlgResizer::DownAndRight);
	m_DlgResizer.MoveItem(IDC_Show_Log_Window,   CDlgResizer::Right);
	m_DlgResizer.MoveItem(IDC_Connection_Status_Static,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_MAX_STATIC,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_MIN_STATIC,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_MODULE_STATIC,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Max_Module_Count_Spin,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Max_Modules_Static,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Min_Module_Count_Spin,   CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Min_Modules_Static,   CDlgResizer::Down);

	m_DlgResizer.MoveItem(IDC_SPOOF_GROUP, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SUPPLY_ENTRIES_GROUP, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SPOOF_PROJECT_PROCESS_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SPOOF_PROGRESS, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SUPPLY_PROGRESS, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_Retrieve_BUTTON, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_HOST_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_HOST_EDIT, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_ADD_HOST_BUTTON, CDlgResizer::Down);
	
	m_DlgResizer.MoveItem(IDC_MAINTENANCE_GROUP_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_MAINTENANCE_STARTED_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_MAINTENANCE_ENDED_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_DEMAND_DELETED_STATIC, CDlgResizer::Down);

	m_DlgResizer.MoveItem(IDC_PROCESS_STARTED_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_PROCESS_ENDED_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_RETRIEVAL_PROCESS_STARTED_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC, CDlgResizer::Down);

	
	m_DlgResizer.MoveItem(IDC_SPOOF_PERCENT_STATIC, CDlgResizer::Down);
	m_DlgResizer.MoveItem(IDC_SUPPLY_PERCENT_STATIC, CDlgResizer::Down);
	
	
	
	m_DlgResizer.DialogIsMinSize();
	
	m_DlgResizer.Done();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


//
//
//
void CGnutellaProtectorDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CGnutellaProtectorDlg::InitRunningSinceWindowText()
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
void CGnutellaProtectorDlg::ReportConnectionStatus(ConnectionModuleStatusData &status,vector<ConnectionModuleStatusData> *all_mod_status)
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

	sprintf(buf,"%u",status.m_cached_query_hit_count);
	m_module_connection_list.SetItemText(index,SUB_CACHED_QUERY_HITS,buf);

	// Update supernode vendor list	
//	UpdateSupernodeVendorList(all_mod_status);

	// Update project status list counters
	bool refresh=false;
	for(i=0;i<status.v_project_status.size();i++)
	{
		if(status.v_project_status[i].m_total_query_count>0)
		{
			refresh=true;

			m_project_status_list.IncrementCounters(status.v_project_status[i]);
		}
	}

	// Update the virtual list control
	if(refresh)
	{
		m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);
	}

	// If there are any searcher socket uptimes, display them
	//UpdateSearcherQueryCounts(status);
}

//
//
//
void CGnutellaProtectorDlg::KeywordsUpdated(vector<ProjectKeywords> &keywords)
{
	/*
	UINT i,j,k;
	
	m_project_keyword_tree.DeleteAllItems();

	HTREEITEM item1,item2,item3;
	char buf[1024];

	for(i=0;i<keywords.size();i++)
	{
		// Project name
		item1=m_project_keyword_tree.InsertItem(keywords[i].m_project_name.c_str(),0,0,TVI_ROOT);

		// Flags
		item2=m_project_keyword_tree.InsertItem("Flags",0,0,item1);

		sprintf(buf,"Project Active : %u",keywords[i].m_project_active);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Auto Multiplier : %u",keywords[i].m_auto_multiplier_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"BearShare Disting : %u",keywords[i].m_bearshare_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		
		sprintf(buf,"GUID Spoofing : %u",keywords[i].m_guid_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		
		sprintf(buf,"Poisoning : %u",keywords[i].m_poisoning_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Uber Disting: %u",keywords[i].m_uber_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Kazaa Find More: %u",keywords[i].m_find_more);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Emule Spoofing: %u",keywords[i].m_emule_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Emule Swarming: %u",keywords[i].m_emule_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		//Kazaa Search Type
		char type[32];
		switch(keywords[i].m_search_type)
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
		sprintf(buf,"Kazaa Search Type : %s",type);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);


		// Query multiplier
		sprintf(buf,"Query Multiplier : %u",keywords[i].m_query_multiplier);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Supply Interval
		sprintf(buf,"Supply Interval : %u",keywords[i].m_interval);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Query keywords
		item2=m_project_keyword_tree.InsertItem("Query Keywords",0,0,item1);
		// Query Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Weight",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",keywords[i].m_query_keywords.v_keywords[j].keyword.c_str(),keywords[i].m_query_keywords.v_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Keywords
		item3=m_project_keyword_tree.InsertItem("Exact Keywords : Weight",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",keywords[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),keywords[i].m_query_keywords.v_exact_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_query_keywords.v_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Killwords
		item3=m_project_keyword_tree.InsertItem("Exact Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_exact_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_query_keywords.v_exact_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}

		// Supply Keywords
		item2=m_project_keyword_tree.InsertItem("Supply Keywords",0,0,item1);
		sprintf(buf,"Search String : %s",keywords[i].m_supply_keywords.m_search_string.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Supply Size Threshold : %u",keywords[i].m_supply_keywords.m_supply_size_threshold);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		// Supply Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Track",0,0,item2);
		for(j=0;j<keywords[i].m_supply_keywords.v_keywords.size();j++)
		{
			buf[0]='\0';
			for(k=0;k<keywords[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
			{
				strcat(buf,keywords[i].m_supply_keywords.v_keywords[j].v_keywords[k]);
				strcat(buf," ");
			}
			sprintf(&buf[strlen(buf)],": %u",keywords[i].m_supply_keywords.v_keywords[j].m_track);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Supply Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_supply_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_supply_keywords.v_killwords[j].v_keywords[0]);	// kinda kludgey *&*
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		
		// QRP Keywords
		item2=m_project_keyword_tree.InsertItem("QRP Keywords",0,0,item1);
		for(j=0;j<keywords[i].m_qrp_keywords.v_keywords.size();j++)
		{
			m_project_keyword_tree.InsertItem(keywords[i].m_qrp_keywords.v_keywords[j].c_str(),0,0,item2);
		}

		// Poisoners
		item2=m_project_keyword_tree.InsertItem("Poisoners",0,0,item1);
		for(j=0;j<keywords[i].v_poisoners.size();j++)
		{
			unsigned int ip=keywords[i].v_poisoners[j].GetIPInt();
			unsigned int port=keywords[i].v_poisoners[j].GetPort();
			sprintf(buf,"%u.%u.%u.%u : %u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}

		// Hash Count
		item2=m_project_keyword_tree.InsertItem("Hash Count",0,0,item1);
		for(j=0;j<keywords[i].v_tracks_hash_counts.size();j++)
		{
			sprintf(buf,"%2d : %d",keywords[i].v_tracks_hash_counts[j].m_track, keywords[i].v_tracks_hash_counts[j].m_hash_count);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}
	}

	// Whenever the keywords are updated, reset the status list in case any projects were removed
	OnBnClickedResetProjectStatusList();

	// Reset the keyword count
	if(keywords.size()==1)
	{
		sprintf(buf,"Project Keywords : ( %u Project )",keywords.size());
	}
	else
	{
		sprintf(buf,"Project Keywords : ( %u Projects )",keywords.size());
	}
	GetDlgItem(IDC_Project_Keywords_Static)->SetWindowText(buf);
	*/
	UINT i,j,k;
	
	m_project_keyword_tree.DeleteAllItems();

	HTREEITEM item1,item2,item3,item4;
	char buf[1024];

	for(i=0;i<keywords.size();i++)
	{
		// Project name
		item1=m_project_keyword_tree.InsertItem(keywords[i].m_project_name.c_str(),0,0,TVI_ROOT);
		sprintf(buf,"Project ID: %u",keywords[i].m_id);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Artist: %s",keywords[i].m_artist_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Album: %s",keywords[i].m_album_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Owner: %s",keywords[i].m_owner.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		item4=m_project_keyword_tree.InsertItem("Viewers",0,0,item1);
		for(j=0;j<keywords[i].m_viewers.v_strings.size();j++)
		{
			m_project_keyword_tree.InsertItem(keywords[i].m_viewers.v_strings[j].c_str(),0,0,item4);
		}

		sprintf(buf,"Version: %u",keywords[i].m_version);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);



		// Flags
		item2=m_project_keyword_tree.InsertItem("Flags",0,0,item1);

		sprintf(buf,"Project Active : %u",keywords[i].m_project_active);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Auto Multiplier : %u",keywords[i].m_auto_multiplier_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BearShare Disting : %u",keywords[i].m_bearshare_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"GUID Spoofing : %u",keywords[i].m_guid_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Poisoning : %u",keywords[i].m_poisoning_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Uber Disting: %u",keywords[i].m_uber_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Emule Spoofing: %u",keywords[i].m_emule_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Swarming: %u",keywords[i].m_emule_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Decoying: %u",keywords[i].m_emule_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Supply: %u",keywords[i].m_emule_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"DirectConnect Decoying: %u",keywords[i].m_directconnect_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Spoofing: %u",keywords[i].m_directconnect_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Supply: %u",keywords[i].m_directconnect_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"FastTrack Decoying: %u",keywords[i].m_fasttrack_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Spoofing: %u",keywords[i].m_fasttrack_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Swarming: %u",keywords[i].m_fasttrack_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Supply: %u",keywords[i].m_fasttrack_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Find More: %u",keywords[i].m_find_more);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gnutella Supply: %u",keywords[i].m_gnutella_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Demand: %u",keywords[i].m_gnutella_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Decoying: %u",keywords[i].m_gnutella_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Spoofing: %u",keywords[i].m_gnutella_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Swarming: %u",keywords[i].m_gnutella_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);


		sprintf(buf,"Piolet Spoofing: %u",keywords[i].m_piolet_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Demand: %u",keywords[i].m_piolet_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Supply: %u",keywords[i].m_piolet_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gift Spoofing: %u",keywords[i].m_gift_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Decoys: %u",keywords[i].m_gift_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Supply: %u",keywords[i].m_gift_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		//Kazaa Search Type
		char type[32];
		switch(keywords[i].m_search_type)
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
		sprintf(buf,"FastTrack Dist Power : %d",keywords[i].m_kazaa_dist_power);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Hash Count
		item2=m_project_keyword_tree.InsertItem("FastTrack Hash Count",0,0,item1);
		for(j=0;j<keywords[i].v_tracks_hash_counts.size();j++)
		{
			sprintf(buf,"%2d : %d",keywords[i].v_tracks_hash_counts[j].m_track, keywords[i].v_tracks_hash_counts[j].m_hash_count);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}

		// Query multiplier
		sprintf(buf,"Query Multiplier : %u",keywords[i].m_query_multiplier);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Supply Interval
		sprintf(buf,"Supply Interval : %u",keywords[i].m_interval);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Query keywords
		item2=m_project_keyword_tree.InsertItem("Query Keywords",0,0,item1);
		// Query Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Weight",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",keywords[i].m_query_keywords.v_keywords[j].keyword.c_str(),keywords[i].m_query_keywords.v_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Keywords
		item3=m_project_keyword_tree.InsertItem("Exact Keywords : Weight",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",keywords[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),keywords[i].m_query_keywords.v_exact_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_query_keywords.v_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Killwords
		item3=m_project_keyword_tree.InsertItem("Exact Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_query_keywords.v_exact_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_query_keywords.v_exact_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}

		// Supply Keywords
		item2=m_project_keyword_tree.InsertItem("Supply Keywords",0,0,item1);
		sprintf(buf,"Search String : %s",keywords[i].m_supply_keywords.m_search_string.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Supply Size Threshold : %u",keywords[i].m_supply_keywords.m_supply_size_threshold);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		// Supply Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Track",0,0,item2);
		for(j=0;j<keywords[i].m_supply_keywords.v_keywords.size();j++)
		{
			buf[0]='\0';
			for(k=0;k<keywords[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
			{
				strcat(buf,keywords[i].m_supply_keywords.v_keywords[j].v_keywords[k]);
				strcat(buf," ");
			}
			sprintf(&buf[strlen(buf)],": %u",keywords[i].m_supply_keywords.v_keywords[j].m_track);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Supply Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<keywords[i].m_supply_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",keywords[i].m_supply_keywords.v_killwords[j].v_keywords[0]);	// kinda kludgey *&*
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		
		// QRP Keywords
		item2=m_project_keyword_tree.InsertItem("QRP Keywords",0,0,item1);
		for(j=0;j<keywords[i].m_qrp_keywords.v_keywords.size();j++)
		{
			m_project_keyword_tree.InsertItem(keywords[i].m_qrp_keywords.v_keywords[j].c_str(),0,0,item2);
		}

		// Poisoners
		item2=m_project_keyword_tree.InsertItem("Poisoners",0,0,item1);
		for(j=0;j<keywords[i].v_poisoners.size();j++)
		{
			unsigned int ip=keywords[i].v_poisoners[j].GetIPInt();
			unsigned int port=keywords[i].v_poisoners[j].GetPort();
			sprintf(buf,"%u.%u.%u.%u : %u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}


	}

	// Whenever the keywords are updated, reset the status list in case any projects were removed
	//OnBnClickedResetProjectStatusList();

	// Reset the keyword count
	if(keywords.size()==1)
	{
		sprintf(buf,"Project Keywords : ( %u Project )",keywords.size());
	}
	else
	{
		sprintf(buf,"Project Keywords : ( %u Projects )",keywords.size());
	}
	GetDlgItem(IDC_Project_Keywords_Static)->SetWindowText(buf);
}

//
//
//
/*
void CGnutellaProtectorDlg::SupplyUpdated(vector<SupplyProject> &supply_projects)
{
	int show_details=((CButton *)GetDlgItem(IDC_Project_Supply_Tree_Details_Check))->GetCheck();

	UINT i,j,k,l,m;
	
	m_project_supply_tree.DeleteAllItems();

	HTREEITEM item1,item2,item3,item4,item5;
	char buf[1024];

	for(i=0;i<supply_projects.size();i++)
	{
		// Project name
		sprintf(buf,"%s - %u / %u",supply_projects[i].m_name.c_str(),supply_projects[i].v_spoof_entries.size(),
			supply_projects[i].v_dist_entries.size());
		item1=m_project_supply_tree.InsertItem(buf,0,0,TVI_ROOT);

		// Check to see if we are supposed to add all of this detailed crap to the tree
		if(show_details)
		{
			// Flags
			item2=m_project_supply_tree.InsertItem("Flags",0,0,item1);
		
			// Uber Disting
			sprintf(buf,"Uber Disting: %u",supply_projects[i].m_uber_dist_enabled);
			m_project_supply_tree.InsertItem(buf,0,0,item2);

			// Spoof Entries
			sprintf(buf,"%u - Spoof Entries",supply_projects[i].v_spoof_entries.size());
			item2=m_project_supply_tree.InsertItem(buf,0,0,item1);

			// File names
			for(j=0;j<supply_projects[i].v_spoof_entries.size();j++)
			{
				sprintf(buf,"%u - %s",supply_projects[i].v_spoof_entries[j].v_entry_sizes.size(),
					supply_projects[i].v_spoof_entries[j].Filename());
				item3=m_project_supply_tree.InsertItem(buf,0,0,item2);

				// File sizes
				for(k=0;k<supply_projects[i].v_spoof_entries[j].v_entry_sizes.size();k++)
				{
					sprintf(buf,"%u - %u bytes",supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos.size(),
						supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].m_entry_size);
					item4=m_project_supply_tree.InsertItem(buf,0,0,item3);

					// File infos
					for(l=0;l<supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos.size();l++)
					{
						if(strlen(supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos[l].Info())>0)
						{
							sprintf(buf,"%u - %s",supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos[l].m_num_spoofs,
								supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos[l].Info());

						}
						else
						{
							sprintf(buf,"%u - %s",supply_projects[i].v_spoof_entries[j].v_entry_sizes[k].v_entry_infos[l].m_num_spoofs,"none");

						}

						m_project_supply_tree.InsertItem(buf,0,0,item4);

						// Spoofs have no file locations
					}
				}
			}

			// Dist Entries
			sprintf(buf,"%u - Dist Entries",supply_projects[i].v_dist_entries.size());
			item2=m_project_supply_tree.InsertItem(buf,0,0,item1);

			// File names
			for(j=0;j<supply_projects[i].v_dist_entries.size();j++)
			{
				sprintf(buf,"%u - %s",supply_projects[i].v_dist_entries[j].v_entry_sizes.size(),
					supply_projects[i].v_dist_entries[j].Filename());
				item3=m_project_supply_tree.InsertItem(buf,0,0,item2);

				// File sizes
				for(k=0;k<supply_projects[i].v_dist_entries[j].v_entry_sizes.size();k++)
				{
					sprintf(buf,"%u - %u bytes",supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos.size(),
						supply_projects[i].v_dist_entries[j].v_entry_sizes[k].m_entry_size);
					item4=m_project_supply_tree.InsertItem(buf,0,0,item3);

					// File infos
					for(l=0;l<supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos.size();l++)
					{
						if(strlen(supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].Info())>0)
						{
							sprintf(buf,"%u - %u - %s",supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations.size(),
								supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].m_num_spoofs,
								supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].Info());
						}
						else
						{
							sprintf(buf,"%u - %u - %s",supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations.size(),
								supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].m_num_spoofs,
								"none");
						}

						item5=m_project_supply_tree.InsertItem(buf,0,0,item4);

						// File info locations
						for(m=0;m<supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations.size();m++)
						{
							if(m>0)
							{
								int sss=3;
							}

							unsigned int ip=supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations[m].m_ip;
							unsigned short int port=supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations[m].m_port;
							unsigned int index=supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations[m].m_index;
							unsigned int speed=supply_projects[i].v_dist_entries[j].v_entry_sizes[k].v_entry_infos[l].v_entry_locations[m].m_speed;

							sprintf(buf,"%u.%u.%u.%u : %u : %u : %u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port,index,speed);
							m_project_supply_tree.InsertItem(buf,0,0,item5);
						}
					}
				}
			}
		}
	}

	// Reset the keyword count
	if(supply_projects.size()==1)
	{
		sprintf(buf,"Project Supply : ( %u Project )",supply_projects.size());
	}
	else
	{
		sprintf(buf,"Project Supply : ( %u Projects )",supply_projects.size());
	}
	GetDlgItem(IDC_Project_Supply_Static)->SetWindowText(buf);
}
*/
//
//
//
/*
LRESULT CGnutellaProtectorDlg::SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam)
{
	p_parent->SupplyManagerThreadDataReady(wparam,lparam);
	return 0;
}
*/
//
//
//
/*
LRESULT CGnutellaProtectorDlg::InitLogfileManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitLogfileManagerThreadData(wparam,lparam);
	return 0;
}
*/
//
//
//
LRESULT CGnutellaProtectorDlg::InitDBManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitDBManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
void CGnutellaProtectorDlg::ClearProjectStatus(string &project)
{
	m_project_status_list.ClearProjectStatus(project);
	m_project_status_list.SetItemCountEx(m_project_status_list.GetItemCount(),LVSICF_NOSCROLL);	// refresh
}

//
//
//
void CGnutellaProtectorDlg::AlterModuleCounts(int dmin,int dmax)
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
void CGnutellaProtectorDlg::ModuleCountHasChanged(int count)
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
/*
LRESULT CGnutellaProtectorDlg::ReadyToWriteLogfileDataToFile(WPARAM wparam,LPARAM lparam)
{
	p_parent->ReadyToWriteLogfileDataToFile(wparam,lparam);
	return 0;
}
*/
//
//
//
LRESULT CGnutellaProtectorDlg::ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam)
{
	// Make sure it is not the initial message that the thread sends
	if((wparam!=1) && (lparam!=1))
	{
		char msg[1024];
		CTime now=CTime::GetCurrentTime();
		sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
		GetDlgItem(IDC_Logfile_Data_Last_Written)->SetWindowText(msg);
	}

	p_parent->ReadyToWriteDataToDatabase(wparam,lparam);
	return 0;
}

//
//
//
void CGnutellaProtectorDlg::UpdateVendorCounts(vector<VendorCount> &vendor_counts,int compression_on_counts)
{
	UINT i;
	int total_hosts=0;

	// Sort the list
	sort(vendor_counts.begin(),vendor_counts.end());

	m_supernode_vendor_list.DeleteAllItems();

	for(i=0;i<vendor_counts.size();i++)
	{
		char count[32];
		_itoa(vendor_counts[i].m_count,count,10);
		int index=m_supernode_vendor_list.InsertItem(0,count,0);
		m_supernode_vendor_list.SetItemText(index,1,vendor_counts[i].m_vendor.c_str());
		total_hosts+=vendor_counts[i].m_count;
	}

	char buf[1024];
	CTime now=CTime::GetCurrentTime();
	sprintf(buf,"Supernode Vendors as of : %04u-%02u-%02u %02u:%02u:%02u",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	GetDlgItem(IDC_Supernode_Vendors_Static)->SetWindowText(buf);

	sprintf(buf,"Compression Supported Vendors: %d",compression_on_counts);
	GetDlgItem(IDC_Compression_On_Static)->SetWindowText(buf);

	sprintf(buf,"Total Connected Hosts: %d",total_hosts);
	GetDlgItem(IDC_TOTAL_HOSTS_STATIC)->SetWindowText(buf);

}

//
//
//
LRESULT CGnutellaProtectorDlg::InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	CString done = (CTime::GetCurrentTime()).Format("Last Data Process Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_PROCESS_STARTED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SPOOF_PERCENT_STATIC)->SetWindowText("0 %");
	m_spoof_progress.SetPos(0);
	
	p_parent->InitProcessManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::ProcessManagerDone(WPARAM wparam,LPARAM lparam)
{
	CString done = (CTime::GetCurrentTime()).Format("Last Data Process Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_PROCESS_ENDED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SPOOF_PROJECT_PROCESS_STATIC)->SetWindowText("");

	m_spoof_progress.SetPos(100);
	GetDlgItem(IDC_SPOOF_PERCENT_STATIC)->SetWindowText("100 %");

	p_parent->ProcessManagerDone(wparam,lparam);
	return 0;
}


//
//
//
LRESULT CGnutellaProtectorDlg::InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_STARTED_STATIC)->SetWindowText(str);

	//p_parent->InitDBManagerMaintenanceThreadData(wparam,lparam);
	return 0;
}
//
//
//
LRESULT CGnutellaProtectorDlg::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{

	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_ENDED_STATIC)->SetWindowText(str);

	char msg[1024];
	sprintf(msg, "%u records deleted from spoof table", (UINT)wparam);
	GetDlgItem(IDC_DEMAND_DELETED_STATIC)->SetWindowText(msg);

	p_parent->ProcessManagerMaintenanceDone(wparam,lparam);
	return 0;
}


//
//
//
LRESULT CGnutellaProtectorDlg::DemandProcessProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SPOOF_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::SupplyProcessProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
void CGnutellaProtectorDlg::UpdateDataProcessProgressBar(int demand)
{
	m_spoof_progress.SetPos(demand);

	char msg[128];
	sprintf(msg, "%d %%", demand);
	GetDlgItem(IDC_SPOOF_PERCENT_STATIC)->SetWindowText(msg);
}

//
//
//
LRESULT CGnutellaProtectorDlg::InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitSupplyManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC)->SetWindowText(str);
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText("");
	m_supply_progress.SetPos(100);
	p_parent->UpdateSupplyManagerSupplyProjects(wparam,lparam);
	p_parent->m_supply_manager.DoneRetreivingSpoofEntries();
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::SupplyManagerThreadProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::SupplyManagerThreadProgress(WPARAM wparam,LPARAM lparam)
{
	int percent = (int)((float)wparam / (float)lparam* (float)100);
	m_supply_progress.SetPos(percent);

	char msg[128];
	sprintf(msg, "%d %%", percent);
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText(msg);
	p_parent->SendSupplyRetrievalProgress(percent);
	return 0;
}

//
//
//
LRESULT CGnutellaProtectorDlg::SupplyManagerThreadAborted(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Aborted at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC)->SetWindowText(str);
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText("");
	m_supply_progress.SetPos(0);
	p_parent->m_supply_manager.AbortedRetreivingSpoofEntries();
	return 0;
}
//
//
//
void CGnutellaProtectorDlg::SpoofEntriesRetrievalStarted()
{
	CString str = (CTime::GetCurrentTime()).Format("Last Spoof Entries Retrieval Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_STARTED_STATIC)->SetWindowText(str);
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedRetrieveButton()
{
	p_parent->RetrieveSupplyEntries();
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedAddHostButton()
{
	UpdateData();
	p_parent->AddHost(m_add_host);
}

//
//
//
void CGnutellaProtectorDlg::OnBnClickedVendorCountsCheck()
{
	m_supernode_vendor_list.DeleteAllItems();
	UpdateData();
	p_parent->EnableVendorCounts(m_vendor_counts_enabled);
}
