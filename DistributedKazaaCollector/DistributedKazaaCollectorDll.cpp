#include "StdAfx.h" 
#include "DistributedKazaaCollectordll.h"
#include "../SupernodeDistributer/KazaaControllerHeader.h"
#include "Psapi.h"		// for EnumProcesses()
#include <Windows.h>	// GetWindowThreadProcessID()
#include <direct.h>		// _rmdir()

//
//
//
DistributedKazaaCollectorDll::DistributedKazaaCollectorDll(void)
{

#ifdef DEBUG
	m_search_more_count = 10;	// debug setting
#else
	m_search_more_count = 10;	// default setting
#endif

	m_total_searches=0;
	m_search_completions=0;
	m_search_returns=0;

	m_launch_buffer=0;
	m_working_project_index=0;
	m_searching = false;
	v_project_keywords.clear();
	m_sys_tray_hwnd = NULL;
}

//
//
//
DistributedKazaaCollectorDll::~DistributedKazaaCollectorDll(void)
{
}

//
//
//
void DistributedKazaaCollectorDll::DllInitialize()
{
	m_dlg.Create(IDD_DIALOG,CWnd::GetDesktopWindow());

#ifdef DEBUG
	m_dlg.GetDlgItem(IDC_SEARCH_INTERVAL)->SetWindowText("3");	// seconds
#else
	m_dlg.GetDlgItem(IDC_SEARCH_INTERVAL)->SetWindowText("10");	// seconds
#endif

	m_dlg.InitParent(this);
	m_dlg.m_worker_iter = 0;

	m_dlg.m_project_list.SetExtendedStyle(m_dlg.m_project_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_dlg.m_project_list.InsertColumn(0,"Project Name",LVCFMT_LEFT, 220);
	m_dlg.m_project_list.InsertColumn(1,"Searches",LVCFMT_LEFT, 100);
	m_dlg.m_project_list.InsertColumn(2,"Raw Results",LVCFMT_LEFT, 100);
}

//
//
//
void DistributedKazaaCollectorDll::DllUnInitialize()
{
	TRACE("DISTRIBUTED KAZAA COLLECTOR: DLL UnInitializing...\n");
	
	// Free all the worker guys;
	FreeAllWorkers();
}

//
//
//
void DistributedKazaaCollectorDll::DllStart()
{
	// Clean and prep... ;)
//	DeleteFile("Log_File.txt");
	DeleteFile("SQL_Error_Log.txt");
	DeleteMasterDir();
//	DeleteFile("query_file.txt");		// we should hold on to all the old data, so it is not lost
//	DeleteKeywordDir();					// and on load up we can keep our most recent projects...

	m_com.Register(this, 35);				// For communicating with the DCMaster (Data collecting master)
											//		- receives the projects (project keywords)
											//		- receives the KazaaController IP (to get the supernodes)
											//		- receives the DB connection info, to dump processed data into
	m_com.InitParent(this);

	m_dlg.Log("Collector Started... Waiting for Kazaa from launcher...");
	m_com.ReadKeywordDataFromFile();

//	LoadMDIpsFromFile();
	
	int r=0;
	v_md_ips.clear();
	while (v_md_ips.size() == 0)
	{
		if (r++ >= 1)	// In case the DB is getting pounded
			Sleep(3000);

		LoadMDIps();
	}

	m_dlg.m_last_update = CTime::GetCurrentTime();
	m_dlg.SetTimer(1, 1000, NULL);		// last update timer...
	m_dlg.SetTimer(2, 1000*60*5, NULL);	// Data dumping timer...
	m_dlg.SetTimer(3, 1000, NULL);		// the trigger
}

//
//
//
void DistributedKazaaCollectorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//	Return:	True, if we are taking the project for searching.
//			False, if we are already searching or no projects
//			False, if we don't know what this is.
//
bool DistributedKazaaCollectorDll::ReceivedDllData(AppID from_app_id, void* input_data, void* output_data)
{
	//received remote SD Dll data
	KazaaControllerHeader* header = (KazaaControllerHeader*)input_data;
	byte *pData=(byte *)input_data;
	pData+=sizeof(KazaaControllerHeader);

	switch(header->op)
	{
		case KazaaControllerHeader::New_Kazaa_Launched:
		{
			if ((v_project_keywords.size()==0))
			{
				*(bool*)output_data = false;
				return false;
			}

			// Let every 5th Kazaa go.  Just to be safe.
			m_launch_buffer++;
			if (m_launch_buffer > 5)
			{
				m_launch_buffer = 0;
				
				*(bool*)output_data = true;
				return false;
			}

			m_pid = *(int *)pData;

			// Do we already have a worker guy for this pid?
			for (UINT i=0; i < v_worker_guy_ptrs.size(); i++)
			{
				if ( v_worker_guy_ptrs[i]->m_process_id == m_pid )
				{
					*(bool*)output_data = true;
					return true;
				}
			}

			// This is not the 5th Kazaa, and we don't have a worker, go for it:
			Search(m_pid);
			*(bool*)output_data = true;	// we took the pid, or we have not let the first 4 go alone yet.
		}
	}

	return false;	// whatever....
}

//
//
//
void DistributedKazaaCollectorDll::Search(int pid)
{
	if (v_project_keywords.size() == 0)
	{
		Log("No projects.");
		return;
	}

	// Get the search more interval
	CString str;
	m_dlg.GetDlgItem(IDC_SEARCH_INTERVAL)->GetWindowText(str);
	m_dlg.GetDlgItem(IDC_SEARCH_INTERVAL)->SetWindowText(str);	// ;)

	if (pid == 0)
		m_dlg.Log("Manual search requested...");


	WorkerGuy* worker = new WorkerGuy;

	worker->InitParent(this);
	worker->m_process_id = pid;
	worker->SetId(pid);
	worker->m_kza_hwnd = GetHwnd(pid);	// system idle process id, should always return the first Kazaa HWND found
										// otherwise return NULL for none.
	
	worker->m_search_more_interval = atoi(str);


	// Do we already have a worker guy for this hwnd?
	bool duplicate = false;
	for (UINT i=0; i < v_worker_guy_ptrs.size(); i++)
	{
		if ( v_worker_guy_ptrs[i]->m_kza_hwnd == worker->m_kza_hwnd )
			duplicate = true;
	}

	if (worker->m_kza_hwnd != NULL && !duplicate)
	{
		Log("CREATED A NEW WORKER GUY...");
		v_worker_guy_ptrs.push_back(worker);
		m_searching = true;
		worker->m_last_state_change = CTime::GetCurrentTime();

		char str[2048];
		itoa((int)v_worker_guy_ptrs.size(), str, 10);
		m_dlg.GetDlgItem(IDC_STATIC_NUM_WORKERS)->SetWindowText(str);
	}
	else
	{
		m_dlg.Log("No Kazaa Window Found.");
		delete worker;
	}

//	RefreshSysTray();
}

BOOL CALLBACK DistributedKazaaCollectorDll::enumwndfn(HWND hWnd, LPARAM lParam)
{
	DistributedKazaaCollectorDll *me = (DistributedKazaaCollectorDll *)lParam;
	return me->enumwndfn(hWnd);
}

//
//	If the pid's match, it will clear the vector and push the correct HWND
//	If the pid is not found, the first HWND to be found will be returned.
//
BOOL DistributedKazaaCollectorDll::enumwndfn(HWND hwnd)
{
	char name[1024+1];
	memset(name,0,sizeof(name));

	// Is this window Kazaa?
	GetClassName(hwnd, name, sizeof(name)-1);
	if(strcmp(name,"KaZaA")==0)
	{
		if (v_temp_kazaa.size() == 0)
			v_temp_kazaa.push_back(hwnd);

		DWORD pid = 1;
		GetWindowThreadProcessId(hwnd, &pid);
		if ( m_pid == pid )
		{
			v_temp_kazaa.clear();
			v_temp_kazaa.push_back(hwnd);
		}
	}

	if(strcmp(name,"Shell_TrayWnd")==0)
	{
		HWND child = ::GetWindow(hwnd, GW_CHILD);
		while (child != NULL)
		{
			char buf[1024];
			memset(buf,0,sizeof(buf));
			GetClassName(child,buf,sizeof(buf));

			if(strcmp(buf,"TrayNotifyWnd")==0)
			{
				HWND next_child = ::GetWindow(child, GW_CHILD);
				while (next_child != NULL)
				{
					char next_buf[1024];
					memset(next_buf,0,sizeof(buf));
					GetClassName(next_child,next_buf,sizeof(next_buf));

					if(strcmp(next_buf,"SysPager")==0)
					{
						m_sys_tray_hwnd = GetWindow(next_child, GW_CHILD);
					}

					next_child = ::GetWindow(next_child, GW_HWNDNEXT);	// get next sibling
				}
			}

			child = ::GetWindow(child, GW_HWNDNEXT);	// get next sibling
		}
	}

	return true;
}

//
//
//
void DistributedKazaaCollectorDll::Log(char *msg)
{
	m_dlg.Log(msg);
}

HWND DistributedKazaaCollectorDll::GetHwnd(int pid)
{
	//
	//	Enumerate all kazaa window handles
	//
	v_temp_kazaa.clear();

	// Enumerate child windows of the desktop, (top-level windows)
	EnumWindows(enumwndfn, (LPARAM)this);

	//Switch between the other 3 desktops and enumerate all Kazaa Windows
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);

	if (v_temp_kazaa.size() > 0)
	{
		// v_temp_kazaa should contain our Hwnd
		return v_temp_kazaa[0];
	}
	else
		return NULL;
}

//
//	Sets the next project for the worker, and sets the m_last_project flag by
//	the return value.  Returns (project.m_name = "") if no more projects, else returns a PROJECT
//
//	If previous_project.m_project_name = "" then we should return the first good project
//
ProjectKeywords DistributedKazaaCollectorDll::GetNextProject(ProjectKeywords previous_project, WorkerGuy* wrkr)
{
	UINT iter = m_working_project_index;
	UINT num_projects = (UINT)v_project_keywords.size();

	if (num_projects == 0)
	{
		m_dlg.KillTimer(0);	// no projects
		m_searching = false;

		m_dlg.Log("No Projects.");
		ProjectKeywords blank;
		blank.m_project_name = "";
		return blank;
	}

	// Is this a new worker guy?
	if (strcmp(previous_project.m_project_name.c_str(), "") == 0)
	{
		// Get the first GOOD project
		int looping = 0;
		while (looping < 3)
		{
			if (iter >= num_projects)
			{
				iter = 0;
				looping++;	// watch out for infinite looop
			}

			CString owner = v_project_keywords[iter].m_owner.c_str();
			CString proj = v_project_keywords[iter].m_project_name.c_str();
			owner.MakeLower();
			
			// Must be active and not terminated (the filter)
			if ( (strstr(owner, "terminated")==NULL) && (v_project_keywords[iter].m_project_active==true) )
			{
				m_working_project_index = iter+1;
				return v_project_keywords[iter];
			}
			
			iter++;
		}

		m_dlg.Log("Did not find any (additional) active projects.");
		wrkr->m_last_project = true;
	
		ProjectKeywords blank;
		blank.m_project_name = "";
		return blank;
	}
	else
	{
		iter = 0;

		// catch up
		while (strcmp(previous_project.m_project_name.c_str(), v_project_keywords[iter].m_project_name.c_str()) != 0)
		{
			iter++;

			if (iter >= num_projects)
			{
				// This project does not exist anymore, return the next good project
				break;
			}
		}

		// Get the next GOOD project
		iter++;
		int looping = 0;
		while (looping < 2)
		{
			if (iter >= num_projects)
			{
				iter = 0;
				looping++;
			}

			CString owner = v_project_keywords[iter].m_owner.c_str();
			CString proj = v_project_keywords[iter].m_project_name.c_str();
			owner.MakeLower();
			
			// Must be active and not terminated (the filter)
			if ( (strstr(owner, "terminated")==NULL) && (v_project_keywords[iter].m_project_active==true) )
			{
				if (wrkr->m_number_of_projects_searched >= num_projects)
				{
					wrkr->m_last_project = true;
	
					m_search_completions++;
					char str[2048];
					itoa(m_search_completions, str, 10);
					m_dlg.GetDlgItem(IDC_STATIC_SEARCH_COMPLETIONS)->SetWindowText(str);
				}

				return v_project_keywords[iter];
			}
			
			iter++;
		}

		m_dlg.Log("Did not find any (additional) active projects.");
		wrkr->m_last_project = true;

		ProjectKeywords blank;
		blank.m_project_name = "";
		return blank;
	}
}

//
//
//
void DistributedKazaaCollectorDll::IncProjectSearch(CString pname, int num_results)
{
	char new_val[16];
	memset(&new_val, 0, sizeof(new_val));

	LVFINDINFO info;
	info.flags = LVFI_PARTIAL|LVFI_STRING;
	info.psz = pname;

	int nIndex=m_dlg.m_project_list.FindItem(&info);

	// Increment the search #
	CString strVal = m_dlg.m_project_list.GetItemText(nIndex, 1);	
	int val = atoi(strVal);
	itoa(++val, new_val, 10);
	m_dlg.m_project_list.SetItemText(nIndex, 1, new_val);

	// Add to the raw results
	strVal = m_dlg.m_project_list.GetItemText(nIndex, 2);
	val = atoi(strVal);
	itoa(val+num_results, new_val, 10);
	m_dlg.m_project_list.SetItemText(nIndex, 2, new_val);
}

//
//
//
void DistributedKazaaCollectorDll::FreeWorker(WorkerGuy *ptr)
{
	vector<WorkerGuy*>::iterator iter;
	iter = v_worker_guy_ptrs.begin();
	
	while (iter != v_worker_guy_ptrs.end())
	{
		if ((*iter)->m_kza_hwnd == ptr->m_kza_hwnd)
		{
			delete (*iter);
			v_worker_guy_ptrs.erase(iter);

			char str[2048];
			itoa((int)v_worker_guy_ptrs.size(), str, 10);
			m_dlg.GetDlgItem(IDC_STATIC_NUM_WORKERS)->SetWindowText(str);

			break;
		}

		iter++;
	}
}

//
//
//
void DistributedKazaaCollectorDll::ResetWorkersProjectCounts()
{
	vector<WorkerGuy*>::iterator iter = v_worker_guy_ptrs.begin();
	
	while (iter != v_worker_guy_ptrs.end())
	{
		(*iter)->m_number_of_projects_searched = 0;
		iter++;
	}

}
//
//
//
void DistributedKazaaCollectorDll::FreeAllWorkers()
{
	vector<WorkerGuy*>::iterator iter = v_worker_guy_ptrs.begin();
	
	while (iter != v_worker_guy_ptrs.end())
	{
		iter = v_worker_guy_ptrs.begin();
		delete (*iter);
		v_worker_guy_ptrs.erase(iter);

		char str[2048];
		itoa((int)v_worker_guy_ptrs.size(), str, 10);
		m_dlg.GetDlgItem(IDC_STATIC_NUM_WORKERS)->SetWindowText(str);
	}
}

//
//
//
void DistributedKazaaCollectorDll::RefreshSysTray()
{
	// Bling Bling... invalidate the window, causing a repaint
	if (m_sys_tray_hwnd != NULL)
	{
		CWnd* cwnd = NULL;
		cwnd = CWnd::FromHandle(m_sys_tray_hwnd);
		cwnd->Invalidate(TRUE);
	}
}

//
//
//
UINT DistributedKazaaCollectorDll::LoadMDIps()
{
	v_md_ips.clear();
	IP_RANGE range;

	// Connect to the database, then open the file just long enough to do the inserts...
	MYSQL *conn;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	conn=mysql_init(NULL);

	// Open the connection to the hub
	if (!mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","test",0,NULL,0))
	{
		TRACE0("Failed to connect to the DC Master - Test Database");
	}
	else
	{					
		// Get the table name (at least the important part, the day specific part)

		// williams1
		mysql_query(conn, "SELECT inet_aton('64.71.163.162'), inet_aton('64.71.163.189')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// williams2
		mysql_query(conn, "SELECT inet_aton('64.71.164.2'), inet_aton('64.71.164.29')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// williams3
		mysql_query(conn, "SELECT inet_aton('64.71.164.130'), inet_aton('64.71.164.157')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// williams4
		mysql_query(conn, "SELECT inet_aton('64.71.165.66'), inet_aton('64.71.165.93')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// williams5
		mysql_query(conn, "SELECT inet_aton('64.71.165.130'), inet_aton('64.71.165.157')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina04*26
		mysql_query(conn, "SELECT inet_aton('38.118.154.66'), inet_aton('38.118.154.124')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina15
		mysql_query(conn, "SELECT inet_aton('38.117.6.2'), inet_aton('38.117.6.26')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina06*28
		mysql_query(conn, "SELECT inet_aton('38.118.151.194'), inet_aton('38.118.151.246')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina07*29
		mysql_query(conn, "SELECT inet_aton('38.118.154.130'), inet_aton('38.118.154.188')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina21
		mysql_query(conn, "SELECT inet_aton('38.113.214.130'), inet_aton('38.113.214.154')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina22
		mysql_query(conn, "SELECT inet_aton('38.113.214.162'), inet_aton('38.113.214.186')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina17
		mysql_query(conn, "SELECT inet_aton('38.113.214.34'), inet_aton('38.113.214.58')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina18
		mysql_query(conn, "SELECT inet_aton('38.113.214.66'), inet_aton('38.113.214.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina19
		mysql_query(conn, "SELECT inet_aton('38.113.214.98'), inet_aton('38.113.214.122')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina16
		mysql_query(conn, "SELECT inet_aton('38.117.6.34'), inet_aton('38.117.6.58')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina20
		mysql_query(conn, "SELECT inet_aton('38.117.6.162'), inet_aton('38.117.6.186')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// cogent2a
		mysql_query(conn, "SELECT inet_aton('38.118.156.34'), inet_aton('38.118.156.40')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// cogent2b
		mysql_query(conn, "SELECT inet_aton('38.118.156.98'), inet_aton('38.118.156.104')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// cogent2c
		mysql_query(conn, "SELECT inet_aton('38.118.156.130'), inet_aton('38.118.156.136')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// cogent3a
		mysql_query(conn, "SELECT inet_aton('38.118.156.226'), inet_aton('38.118.156.232')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// cogent3b
		mysql_query(conn, "SELECT inet_aton('38.118.157.34'), inet_aton('38.118.157.40')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina05*27
		mysql_query(conn, "SELECT inet_aton('38.118.151.130'), inet_aton('38.118.151.188')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina08*30
		mysql_query(conn, "SELECT inet_aton('38.118.160.130'), inet_aton('38.118.160.188')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina09
		mysql_query(conn, "SELECT inet_aton('38.118.155.2'), inet_aton('38.118.155.5')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina10
		mysql_query(conn, "SELECT inet_aton('38.118.155.66'), inet_aton('38.118.155.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina11*cogent1
		mysql_query(conn, "SELECT inet_aton('38.118.155.130'), inet_aton('38.118.155.179')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina12
		mysql_query(conn, "SELECT inet_aton('38.113.214.2'), inet_aton('38.113.214.26')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina13
		mysql_query(conn, "SELECT inet_aton('38.118.161.2'), inet_aton('38.118.161.26')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina23
		mysql_query(conn, "SELECT inet_aton('38.118.154.2'), inet_aton('38.118.154.26')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina14
		mysql_query(conn, "SELECT inet_aton('38.118.161.66'), inet_aton('38.118.161.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron1
		mysql_query(conn, "SELECT inet_aton('66.54.72.66'), inet_aton('66.54.72.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron2
		mysql_query(conn, "SELECT inet_aton('66.160.140.162'), inet_aton('66.160.140.186')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron3
		mysql_query(conn, "SELECT inet_aton('66.54.73.66'), inet_aton('66.54.73.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron4
		mysql_query(conn, "SELECT inet_aton('66.160.140.226'), inet_aton('66.160.140.250')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron5
		mysql_query(conn, "SELECT inet_aton('66.54.74.66'), inet_aton('66.54.74.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron6
		mysql_query(conn, "SELECT inet_aton('66.160.142.162'), inet_aton('66.160.142.186')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron7
		mysql_query(conn, "SELECT inet_aton('66.54.76.66'), inet_aton('66.54.76.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron8
		mysql_query(conn, "SELECT inet_aton('66.160.142.226'), inet_aton('66.160.142.250')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron9
		mysql_query(conn, "SELECT inet_aton('64.71.163.34'), inet_aton('64.71.163.61')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// aleron10
		mysql_query(conn, "SELECT inet_aton('64.71.163.66'), inet_aton('64.71.163.93')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina24
		mysql_query(conn, "SELECT inet_aton('38.118.163.130'), inet_aton('38.118.163.159')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina25
		mysql_query(conn, "SELECT inet_aton('38.118.163.194'), inet_aton('38.118.163.218')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina31
		mysql_query(conn, "SELECT inet_aton('38.113.214.194'), inet_aton('38.113.214.221')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina32
		mysql_query(conn, "SELECT inet_aton('38.119.64.2'), inet_aton('38.119.64.26')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina33
		mysql_query(conn, "SELECT inet_aton('38.119.64.66'), inet_aton('38.119.64.90')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina33a
		mysql_query(conn, "SELECT inet_aton('38.119.64.100'), inet_aton('38.119.64.124')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina34
		mysql_query(conn, "SELECT inet_aton('38.115.4.162'), inet_aton('38.115.4.181')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina35
		mysql_query(conn, "SELECT inet_aton('38.115.4.194'), inet_aton('38.115.4.213')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina36
		mysql_query(conn, "SELECT inet_aton('38.115.4.226'), inet_aton('38.115.4.245')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina37
		mysql_query(conn, "SELECT inet_aton('38.119.36.2'), inet_aton('38.119.36.21')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina38
		mysql_query(conn, "SELECT inet_aton('38.113.196.34'), inet_aton('38.113.196.53')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina39
		mysql_query(conn, "SELECT inet_aton('38.115.4.66'), inet_aton('38.115.4.85')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina40
		mysql_query(conn, "SELECT inet_aton('38.115.4.98'), inet_aton('38.115.4.117')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina41
		mysql_query(conn, "SELECT inet_aton('38.115.4.130'), inet_aton('38.115.4.149')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina42
		mysql_query(conn, "SELECT inet_aton('38.113.214.226'), inet_aton('38.113.214.250')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		// marina43
		mysql_query(conn, "SELECT inet_aton('38.115.4.2'), inet_aton('38.115.4.21')");
		res = mysql_store_result(conn);
		row=mysql_fetch_row(res);	// allocate mem
		range.start_ip = atoi(row[0]);
		range.end_ip = atoi(row[1]);
		v_md_ips.push_back(range);
		mysql_free_result(res);		// release mem

		mysql_close(conn); // Close connection to the database
	}

	return (UINT)v_md_ips.size();
}


//
//
//
void DistributedKazaaCollectorDll::LoadMDIpsFromFile()
{
	CStdioFile file;
	CString line;
	
	v_md_ips.clear();

	if (file.Open("md_ips.txt", CFile::modeRead, NULL) != 0)
	{
		while (file.ReadString(line))
		{
			IP_RANGE range;
			int num = sscanf(line, "%*[^0-9]%d%*[^0-9]%d%*[^0-9]", &range.start_ip, &range.end_ip);

			if (num == 2)
				v_md_ips.push_back(range);
		}
	}
	else
	{
		TRACE("Error opening file 'md_ips.txt'\n");
	}

}

//
//
//
int DistributedKazaaCollectorDll::DeleteMasterDir()
{
	char *folder="Master Keyword Files\\";
	string path;

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.kwd";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CString file = "Master Keyword Files\\";
		file += file_data.cFileName;
		DeleteFile(file);
		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
	return _rmdir("Master Keyword Files\\");
}

//
//
//
int DistributedKazaaCollectorDll::DeleteKeywordDir()
{
	char *folder="Keyword Files\\";
	string path;

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.kwd";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CString file = "Keyword Files\\";
		file += file_data.cFileName;
		DeleteFile(file);
		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
	return _rmdir("Keyword Files\\");
}