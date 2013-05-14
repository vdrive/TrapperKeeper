// ConnectionModule.h

#ifndef CONNECTION_MODULE_H
#define CONNECTION_MODULE_H

#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "..\PioletSpoofer\PoisonEntry.h"
//#include "ProjectKeywords.h"
//#include "ProjectStatus.h"
//#include "ConnectionModuleStatusData.h"
//#include "SupplyProject.h"
//#include "ProjectSupplyQuery.h"
#include "FTConnectionModule.h"
#include "ListeningSocket.h"

class PioletPoisoner;
class CCriticalSection;

class ConnectionModule
{
public:
	ConnectionModule(PioletPoisoner *parent);
	~ConnectionModule();
	void InitParent(PioletPoisoner *parent);

	void TimerHasFired();
	void Log(const char *text,COLORREF color=0xFFFFFFFF,bool bold=false,bool italic=false);
	void OneMinuteTimer();
	void ReceivedFileRequest(PoisonEntry* pe);
	void OnReceivedFileRequest(PoisonEntry* pe);
	void AcceptedConnection(SOCKET hSocket, int port, const char* ip, UINT filesize, const char* md5);
	void KillListeningSocket(ListeningSocket* socket);

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	//PoisonEntry GetPoisonEntry(char* filename);
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
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData* p_thread_data;
	HWND m_main_hwnd;
	PioletPoisoner* p_parent;
	vector<ListeningSocket*> v_listening_sockets;
	vector<FTConnectionModule*> v_ft_connection_modules;


//	unsigned int m_header_counter;
//	UINT m_max_host;
//	UINT m_max_host_cache;

public:
//	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};

#endif // CONNECTION_MODULE_H