#pragma once
#include "Dll.h"
#include "../DllLoader/AppID.h"
#include "../DCMaster/ProjectKeywords.h"
#include "KazaaSupplyTakerCom.h"
#include "DistributedKazaaCollectorDlg.h"
#include "WorkerGuy.h"
#include "DBInterface.h"
#include "IP_RANGE.h"

class DistributedKazaaCollectorDll :	public Dll
{
public:

	DistributedKazaaCollectorDll(void);
	~DistributedKazaaCollectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	
	//override function to receive data from the Interface
	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);
	

	// ***********************************************************************************************************************
	//
	//	DLL Specific Functions / Members:
	//
	// ***********************************************************************************************************************

	HWND GetHwnd(int pid);	// get's the Hwnd for a process id, if the process id does not exist,
							// the return value is the first kazaa HWND found.

	ProjectKeywords GetNextProject(ProjectKeywords previous_project, WorkerGuy *wrkr);

	void Log(char *msg);
	void Search(int pid);									// search one iteration for all projects, on pid -> m_kza_hwnd
	void IncProjectSearch(CString pname, int num_results);	// update the CListBox
	void FreeWorker(WorkerGuy*);
	void FreeAllWorkers();
	void RefreshSysTray();
	void LoadMDIpsFromFile();
	void ResetWorkersProjectCounts();

	UINT LoadMDIps();
	void SendResults();
	int DeleteMasterDir();
	int DeleteKeywordDir();

	static BOOL CALLBACK enumwndfn(HWND hWnd, LPARAM lParam);
	BOOL enumwndfn(HWND hwnd);

	DWORD m_pid;
	HWND m_sys_tray_hwnd;

	int m_launch_buffer;
	int m_search_more_count;
	int m_working_project_index;

	int m_total_searches;
	int m_search_completions;
	long m_search_returns;

	bool m_searching;

	vector<ProjectKeywords> v_project_keywords;
	vector<bool> v_project_searched;

	vector<HWND> v_temp_kazaa;
	vector<IP_RANGE> v_md_ips;
	vector<WorkerGuy*> v_worker_guy_ptrs;

	DBInterface m_db;
	KazaaSupplyTakerCom m_com;
	CDistributedKazaaCollectorDlg m_dlg;
};
