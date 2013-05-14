// ConnectionModule.h

#ifndef FT_CONNECTION_MODULE_H
#define FT_CONNECTION_MODULE_H

#include "FTConnectionModuleMessageWnd.h"
#include "FTConnectionModuleThreadData.h"
//#include "ProjectKeywords.h"
//#include "ProjectStatus.h"
//#include "ConnectionModuleStatusData.h"
//#include "SupplyProject.h"
//#include "ProjectSupplyQuery.h"

class ConnectionModule;
class CCriticalSection;

class FTConnectionModule
{
public:
	FTConnectionModule();
	~FTConnectionModule();
	void InitParent(ConnectionModule *parent);
	void TimerHasFired(HWND hwnd);
	bool ConnectToHost(char* ip, int port, UINT file_length, const char* md5);
	bool AcceptConnection(SOCKET hSocket, const char* ip, int port, UINT file_length, const char* md5);
	int GetNumIdleSockets();
	void OneMinuteTimer();

//	void Log(const char *text,COLORREF color=0xFFFFFFFF,bool bold=false,bool italic=false);

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
//	void StatusReady(WPARAM wparam,LPARAM lparam);
//	void KeywordsUpdated();

	HWND m_hwnd;
	CCriticalSection *p_critical_section;
//	ConnectionModuleStatusData m_status_data;
//	vector<ProjectSupplyQuery> v_project_supply_queries;
//	vector<SupplyProject>* p_supply;
//	vector<CString> v_piolet_dcs;

//	ConnectionModuleStatusData ReportStatus();
//	void SpoofDataReady();
//	void ProjectSupplyUpdated(char* project);
//	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
//	unsigned int GetHeaderCounter();
//	void AddDC(CString dc);

private:
//	vector<ProjectKeywords> v_keywords;
//	ConnectionManager *p_manager;
	FTConnectionModuleMessageWnd m_wnd;
	FTConnectionModuleThreadData* p_thread_data;
	ConnectionModule* p_parent;
	vector<SOCKET> v_cached_incoming_connection_socket_handles;

//	unsigned int m_header_counter;
//	UINT m_max_host;
//	UINT m_max_host_cache;

public:
//	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};

#endif // CONNECTION_MODULE_H