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
#include "PoisonEntries.h"
#include "PoisonerStatus.h"
#include "osrng.h"	// for CryptoPP


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
	void Log(char *buf, COLORREF color=0xFFFFFFFF, bool bold=false, bool italic=false);



	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	ConnectionModuleStatusData m_status_data;
	vector<ProjectSupplyQuery> v_project_supply_queries;
	vector<CString> v_piolet_dcs;
	vector<SupplyProject>* p_supply;

	ConnectionModuleStatusData ReportStatus();
	void SpoofDataReady();
	void ProjectSupplyUpdated(char* project);
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	unsigned int GetHeaderCounter();
	void AddDC(CString dc);
	void SendPoisonEntry(char* poisoner, PoisonEntries& pe);
	vector<PoisonerStatus>* GetPoisoners();
//	vector<SupplyProject>* GetSupplyProjectPointer();
//	void ReleaseSupplyProjectPointer();
	bool IsProjectSupplyBeingUpdated();
	int GetNumPoisoners();


private:
	vector<ProjectKeywords> v_keywords;
	ConnectionManager *p_manager;
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData m_thread_data;
	unsigned int m_header_counter;
	UINT m_max_host;
	UINT m_max_host_cache;

public:
	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
	CryptoPP::AutoSeededRandomPool m_rng;
};

#endif // CONNECTION_MODULE_H