// PioletDCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PioletDCDlg.h"
#include "PioletDCDll.h"


// CPioletDCDlg dialog

IMPLEMENT_DYNAMIC(CPioletDCDlg, CDialog)
CPioletDCDlg::CPioletDCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPioletDCDlg::IDD, pParent)
	, m_maximum_hosts(0)
	, m_maximum_hosts_cache(0)
{
	p_parent = NULL;
}

CPioletDCDlg::~CPioletDCDlg()
{
}

void CPioletDCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Status_List, m_status_list);
	DDX_Text(pDX, IDC_MAXIMUM_HOST_EDIT, m_maximum_hosts);
	DDX_Text(pDX, IDC_MAXIMUM_HOST_CACHE_EDIT, m_maximum_hosts_cache);
	DDX_Control(pDX, IDC_VENDOR_LIST, m_vendor_list);
	DDX_Control(pDX, IDC_Project_Keyword_Tree, m_project_keyword_tree);
	DDX_Control(pDX, IDC_DEMAND_PROGRESS, m_demand_progress);
	DDX_Control(pDX, IDC_SUPPLY_PROGRESS, m_supply_progress);
}


BEGIN_MESSAGE_MAP(CPioletDCDlg, CDialog)
	ON_WM_TIMER()
	
	ON_MESSAGE(WM_INIT_DB_MANAGER_THREAD_DATA,InitDBManagerThreadData)
	ON_MESSAGE(WM_READY_TO_WRITE_DATA_TO_DB,ReadyToWriteDataToDatabase)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_THREAD_DATA,InitProcessManagerThreadData)
	ON_MESSAGE(WM_PROCESST_MANAGER_DONE, ProcessManagerDone)
	ON_MESSAGE(WM_SUPPLY_PROCESS_PROJECT, SupplyProcessProject)
	ON_MESSAGE(WM_DEMAND_PROCESS_PROJECT, DemandProcessProject)
	ON_MESSAGE(WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,InitProcessManagerMaintenanceThreadData)
	ON_MESSAGE(WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE,ProcessManagerMaintenanceDone)

	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_DATA_PROCESS_BUTTON, OnBnClickedDataProcessButton)
END_MESSAGE_MAP()


// CPioletDCDlg message handlers
//
//
void CPioletDCDlg::InitParent(PioletDCDll* parent)
{
	p_parent = parent;
}
void CPioletDCDlg::OnTimer(UINT nIDEvent)
{
	if(p_parent!=NULL)
		p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
BOOL CPioletDCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_demand_progress.SetRange(0,100);
	m_supply_progress.SetRange(0,100);

	m_ping_count=0;
	m_pong_count=0;
	m_query_count=0;
	m_query_hit_count=0;
	m_unknown=0;
	m_error_pong=0;
    ReadMaximumHostsDataFromRegistry();

	IncrementCounters(0,0,0,0,0,0);
	InitRunningSinceWindowText();

	m_vendor_list.InsertColumn(0, "Vendor",LVCFMT_LEFT,220);
	m_vendor_list.InsertColumn(1, "Count",LVCFMT_LEFT,50);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CPioletDCDlg::Log(char *buf)
{
//	m_rich_status_edit.SetSel(-1,-1);
//	m_rich_status_edit.ReplaceSel(buf);
	
	CString cstring=buf;
	cstring.Trim();
	m_status_list.InsertString(0,cstring);

	while(m_status_list.GetCount()>1000)
	{
		m_status_list.DeleteString(1000);
	}
}


//
//
//
void CPioletDCDlg::IncrementCounters(unsigned int ping,unsigned int pong,unsigned int query,unsigned int query_hit,unsigned int unknown,unsigned int error_pong)
{
	m_ping_count+=ping;
	m_pong_count+=pong;
	m_query_count+=query;
	m_query_hit_count+=query_hit;
	m_unknown+=unknown;
	m_error_pong+=error_pong;

	char msg[1024];
	sprintf(msg,"Ping : %u   Pong : %u   Query : %u   Query Hit : %u   Unknown : %u   Error Pong : %u",m_ping_count,m_pong_count,m_query_count,m_query_hit_count,m_unknown,m_error_pong);
	GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
}

//
//
//
LRESULT CPioletDCDlg::InitDBManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitDBManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CPioletDCDlg::ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam)
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
LRESULT CPioletDCDlg::InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam)
{

	CString done = (CTime::GetCurrentTime()).Format("Last Data Process Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_PROCESS_STARTED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_DEMAND_PERCENT_STATIC)->SetWindowText("0 %");
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText("0 %");
	m_demand_progress.SetPos(0);
	m_supply_progress.SetPos(0);
	
	p_parent->InitProcessManagerThreadData(wparam,lparam);

	return 0;
}

//
//
//
LRESULT CPioletDCDlg::ProcessManagerDone(WPARAM wparam,LPARAM lparam)
{

	CString done = (CTime::GetCurrentTime()).Format("Last Data Process Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_PROCESS_ENDED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_DEMAND_PROJECT_PROCESS_STATIC)->SetWindowText("");
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText("");

	m_demand_progress.SetPos(100);
	m_supply_progress.SetPos(100);
	GetDlgItem(IDC_DEMAND_PERCENT_STATIC)->SetWindowText("100 %");
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText("100 %");

	p_parent->ProcessManagerDone(wparam,lparam);

	return 0;
}


//
//
//
LRESULT CPioletDCDlg::InitProcessManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam)
{
	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_STARTED_STATIC)->SetWindowText(str);
	return 0;
}

//
//
//
LRESULT CPioletDCDlg::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{

	CString str = (CTime::GetCurrentTime()).Format("Last DB Maintenance Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_MAINTENANCE_ENDED_STATIC)->SetWindowText(str);

	char msg[1024];
	sprintf(msg, "%u records deleted from demand table", (UINT)wparam);
	GetDlgItem(IDC_DEMAND_DELETED_STATIC)->SetWindowText(msg);
	sprintf(msg, "%u records deleted from supply table", (UINT)lparam);
	GetDlgItem(IDC_SUPPLY_DELETED_STATIC)->SetWindowText(msg);

	p_parent->ProcessManagerMaintenanceDone(wparam,lparam);

	return 0;
}


//
//
//
LRESULT CPioletDCDlg::DemandProcessProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_DEMAND_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
LRESULT CPioletDCDlg::SupplyProcessProject(WPARAM wparam,LPARAM lparam)
{
	GetDlgItem(IDC_SUPPLY_PROJECT_PROCESS_STATIC)->SetWindowText((char*)wparam);
	return 0;
}

//
//
//
void CPioletDCDlg::InitRunningSinceWindowText()
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
void CPioletDCDlg::OnBnClickedButton1()
{
	UpdateData(TRUE);
	WriteMaximumHostsDataToRegistry();
	p_parent->UpdateHostLimits(m_maximum_hosts,m_maximum_hosts_cache);
}

//
//
//
void CPioletDCDlg::ReadMaximumHostsDataFromRegistry()
{
	// If this is the first time running, or the key doesn't exist, return default values
	m_maximum_hosts=5000;
	m_maximum_hosts_cache=5000;

	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletDC\\Host Limits";

	if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		UINT val;
		DWORD cbData=sizeof(int);

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)&val,&cbData)==ERROR_SUCCESS)
		{
			if(strcmp(szName,"Max Hosts")==0)
			{
				m_maximum_hosts=val;
			}
			else if(strcmp(szName,"Max Hosts Cache")==0)
			{
				m_maximum_hosts_cache=val;
			}

			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}
	RegCloseKey(hKey);
	UpdateData(FALSE);
}

//
//
//
void CPioletDCDlg::WriteMaximumHostsDataToRegistry()
{
	HKEY hKey;
	char subkey[]="Software\\MediaDefender\\PioletDC\\Host Limits";
	RegCreateKeyEx(HKEY_CURRENT_USER,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(int);
	RegSetValueEx(hKey,"Max Hosts",0,REG_DWORD,(unsigned char *)&m_maximum_hosts,cbData);
	RegSetValueEx(hKey,"Max Hosts Cache",0,REG_DWORD,(unsigned char *)&m_maximum_hosts_cache,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void CPioletDCDlg::ReportVendorCounts(vector<VendorCount>* vc)
{
	char tmp[32];
	int total=0;
	m_vendor_list.DeleteAllItems();
	for(UINT i=0;i<vc->size();i++)
	{
		total += (*vc)[i].m_count;
		int ret = m_vendor_list.InsertItem(m_vendor_list.GetItemCount(), (*vc)[i].m_vendor.c_str());
		m_vendor_list.SetItemText(ret,1,itoa((*vc)[i].m_count,tmp,10));
	}
	GetDlgItem(IDC_CONNECTED_HOST_STATIC)->SetWindowText(itoa(total,tmp,10));
}

//
//
//
void CPioletDCDlg::KeywordsUpdated(vector<ProjectKeywords> &keywords)
{
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
void CPioletDCDlg::OnBnClickedDataProcessButton()
{
	p_parent->StartDataProcess();
}

//
//
//
void CPioletDCDlg::UpdateDataProcessProgressBar(int demand, int supply)
{
	m_demand_progress.SetPos(demand);
	m_supply_progress.SetPos(supply);

	char msg[128];
	sprintf(msg, "%d %%", demand);
	GetDlgItem(IDC_DEMAND_PERCENT_STATIC)->SetWindowText(msg);
	sprintf(msg, "%d %%", supply);
	GetDlgItem(IDC_SUPPLY_PERCENT_STATIC)->SetWindowText(msg);

}