#pragma once
#include "..\sampleplugin\dll.h"
#include "..\StatusSource\MyComInterface.h"
#include "OvernetSpooferMainDlg.h"
#include "..\TKProjectManager\ProjectManagerInterface.h"
#include "..\DCMaster\ProjectKeywordsVector.h"
#include "ConnectionManager.h"
#include "SyncherDest.h"
#include "afxmt.h"

class OvernetSpooferDll :
	public Dll
{
public:
	OvernetSpooferDll(void);
	~OvernetSpooferDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void OnTimer(UINT nIDEvent);
	void Log(const char* log);

	void StatusReady(ConnectionModuleStatusData& status);
	void GetProjects(ProjectKeywordsVector& projects);
	void SupplySynched(const char* source_ip);

	
private:
	void ExtractApostropheDashWords(vector<CString> *keywords,const char *title);

	COvernetSpooferMainDlg m_dlg;
	ProjectManagerInterface m_project_interface;
	ConnectionManager* p_connection_manager;
	ProjectKeywordsVector m_projects;
	CCriticalSection m_lock;
	SyncherDest m_syncher;
};
