// KazaaManager.h: interface for the KazaaManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KAZAAMANAGER_H__BF1390A3_4E47_483E_AEB2_A0BDE93FF5B4__INCLUDED_)
#define AFX_KAZAAMANAGER_H__BF1390A3_4E47_483E_AEB2_A0BDE93FF5B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "KazaaLauncher.h"
#include "WorkerGuy.h"
#include "Project.h"
#include "DBInterface.h"
#include "KazaaSupplyTakerCom.h"
//#include "KazaaSupplySupernodeCom.h"
#include "../DCMaster/ProjectKeywords.h"
#include "Supernode.h"
#include "ID_HWND.h"
#include "ProcessInfo.h"

class CKazaaSupplyDlg;

class KazaaManager  
{
public:

	KazaaManager();
	virtual ~KazaaManager();

	void InitParent(CKazaaSupplyDlg *parent);

//	void KickOffKazaas();	// This needs to be implemented when ready

	void GetKazaaList();	// Clear and populate v_kazaa

	void Log(int worker_id, HWND hwnd, char *buf);
	void Log(int worker_id, int start_ip, int end_ip, char *buf);

	void CreateWorkerGuy(int id);
	void FreeWorkerGuys();
	void CloseAllKazaas();

//	void RequestSuperNodeList();
//	void FilterRegistrySupernodeList();

//	void SetSuperNodeList(char *data, int data_length);
//	void ReadInConfigFile();
//	void EditRegistry(vector<Supernode> supernodes);
//	void SetApproxNodeRange(int id, UINT start_ip, UINT end_ip);

//	bool KillOldProcessIds();		// return TRUE if we find an old process id
//	bool StartNewKazaa(int id);		// Will use KazaaLauncher to create a new Kazaa for this id.

//	vector<Supernode> GetNextNodeRange();

	int GetLock();
	int ReleaseLock();

	HWND GetFreeKazaa(int id);		// Returns the HWND to a free kazaa

	static BOOL CALLBACK enumwndfn(HWND hWnd, LPARAM lParam);
	BOOL enumwndfn(HWND hwnd);

	int LoadProjects();
	ProjectKeywords GetNextProject(ProjectKeywords previous_project);
	ProjectKeywords GetNextProjectSerial();
	
	ProjectKeywords m_project_to_return;

	CString m_supernode_distributer_ip;

	int m_current_project_index;
	int m_search_more_count;

	vector<ID_HWND> v_kazaa;						// Vector of Kazaa window pointers
	vector<HWND>	v_temp_kazaa;

	vector<WorkerGuy *> v_ptrWorkerGuys;			// Searching objects. One per kazaa
	vector<Project> v_projects;
	vector<ProjectKeywords> v_project_keywords;		// Our projects
//	vector<Supernode> v_supernodes;					// The supernode list
	vector<ProcessInfo> v_used_kza_process_ids;		// Process info of kazaas that have been used
	vector<DWORD> v_current_kza_process_ids;		// Process ids of kazaas currently in use

//	int m_num_supernodes;
//	int m_last_node_element_used;

//	KazaaLauncher m_launcher;
	DBInterface m_db;
	CKazaaSupplyDlg *p_dlg;

	bool m_minimize;
	bool m_processing;
//	bool m_got_sd_ip;

	HWND m_dlg_hwnd;

	KazaaSupplyTakerCom m_com;
//	KazaaSupplySupernodeCom m_supernode_com;

/*
	CString m_ip;
	CString m_login;
	CString m_pass;
	CString m_path;
*/
};

#endif // !defined(AFX_KAZAAMANAGER_H__BF1390A3_4E47_483E_AEB2_A0BDE93FF5B4__INCLUDED_)
