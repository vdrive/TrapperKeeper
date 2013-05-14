// ConnectionModule.h

#ifndef CONNECTION_MODULE_H
#define CONNECTION_MODULE_H

#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "ProjectKeywords.h"
#include "ProjectStatus.h"
#include "ConnectionModuleStatusData.h"
#include "SupplyProject.h"
#include "ProjectSupplyQuery.h"

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
	void StatusReady(WPARAM wparam,LPARAM lparam);
	void KeywordsUpdated();


	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	ConnectionModuleStatusData m_status_data;
	vector<ProjectSupplyQuery> v_project_supply_queries;

	ConnectionModuleStatusData ReportStatus();
	void DemandSupplyDataReady();
	void ProjectSupplyUpdated(char* project);
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	unsigned int GetHeaderCounter();

private:
	vector<ProjectKeywords> v_keywords;
	vector<SupplyProject> v_supply;
	ConnectionManager *p_manager;
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData m_thread_data;
	unsigned int m_header_counter;
	UINT m_max_host;
	UINT m_max_host_cache;

public:
	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};

#endif // CONNECTION_MODULE_H