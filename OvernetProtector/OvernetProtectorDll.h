#pragma once
#include "c:\cvs\mdproj\trapperkeeper\sampleplugin\dll.h"
#include "OvernetMainDlg.h"
#include "..\TKProjectManager\ProjectManagerInterface.h"
#include "..\DCMaster\ProjectKeywordsVector.h"
#include "ConnectionManager.h"
#include "..\SupplyProcessor\SupplyProcessorInterface.h"
#include "afxmt.h"

class OvernetProtectorDll :
	public Dll
{
public:
	OvernetProtectorDll(void);
	~OvernetProtectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void OnTimer(UINT nIDEvent);
	void Log(const char* log);

	//override function to receive data from the Interface
	//void DataReceived(char *source_name, void *data, int data_length);

	void StatusReady(ConnectionModuleStatusData& status);
	void GetProjects(ProjectKeywordsVector& projects);
	void RetrievingProject(const char* project, int track, int percent);
	void DoneRetrievingProject();
	void RetreiveNewSpoofEntries();

	
private:
	//MyComInterface m_com;
	COvernetMainDlg m_dlg;
	ProjectManagerInterface m_project_interface;
	ConnectionManager* p_connection_manager;
	ProjectKeywordsVector m_projects;
	SupplyProcessorInterface m_supply;
	bool m_retreiving_spoof_entries;
	CCriticalSection m_lock;
};
