// ConnectionModule.h

#ifndef CONNECTION_MODULE_H
#define CONNECTION_MODULE_H

#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "..\DCMaster\ProjectKeywords.h"
#include "ProjectSupplyQuery.h"
#include "ConnectionModuleStatusData.h"

class ConnectionManager;
class CCriticalSection;
class ConnectionModule
{
public:
	ConnectionModule();
	~ConnectionModule();
	void InitParent(ConnectionManager *manager);

	void TimerHasFired();

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void KeywordsUpdated();
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	ConnectionModuleStatusData ReportStatus();
	void StatusReady();
	void Log(const char* log);
	ProjectSupplyQuery GetPSQ(const byte* hash);


	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	vector<ProjectSupplyQuery> v_project_supply_queries;
	ConnectionModuleStatusData m_status_data;

private:
	vector<ProjectKeywords> v_keywords;
	ConnectionManager *p_manager;
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData m_thread_data;
	CWinThread* m_thread;
};

#endif // CONNECTION_MODULE_H