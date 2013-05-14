// ConnectionModule.h

#ifndef CONNECTION_MODULE_H
#define CONNECTION_MODULE_H

#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "..\DCMaster\ProjectKeywords.h"
#include "ConnectionModuleStatusData.h"
#include "..\SupplyProcessor\SupplyData.h"
#include <afxmt.h>	// for CCriticalSection
//#include "HashManager.h"
#include "SupplyDataSet.h"

class ConnectionManager;
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
	ConnectionModuleStatusData ReportStatus();
	void StatusReady();
	void Log(const char* log);
	void ReadInSupplyEntries();
	void SupplySynched();
	hash_set<SupplyDataSet>::iterator GetSupplyPointer(UINT project_id);


	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	CCriticalSection m_supplies_lock;
	ConnectionModuleStatusData m_status_data;
	//vector<SupplyData> v_supply_entries;
	//HashManager m_hash_manager;
	vector<ProjectKeywords> v_keywords;

	hash_set<SupplyDataSet,SupplyDataSetHash> hs_supply_data;

private:
	UINT GetNumSupplies();

	ConnectionManager *p_manager;
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData m_thread_data;
	CWinThread* m_thread;
	UINT m_num_supplies;
};

#endif // CONNECTION_MODULE_H