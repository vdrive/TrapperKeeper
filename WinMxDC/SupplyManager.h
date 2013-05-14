// SupplyManager.h
#pragma once
#include "Manager.h"
//#include "ProjectSupplyFiles.h"
#include "SupplyProject.h"
#include "ProjectQueryHits.h"
//#include "SupplyManagerThreadData.h"
//#include "SupplyQuery.h"
#include "SupplyManagerThreadData.h"
#include <afxmt.h>	// for CCriticalSection


class KeywordManager;
class ConnectionManager;
class WinMxDcDll;
class SupplyManager : public Manager
{
public:
	SupplyManager();
	~SupplyManager();

	void SetConnectionManager(ConnectionManager* cm);
	void UpdateProjects(vector<SupplyProject> &supply_projects);
	void AlterSupplyIntervalMultiplier(int multi);
	void RestartSearching(){ m_last_searching_index = 0;};
	//void OnHeartbeat();
	//void OnOneMinuteTimer();
	void OnTenSecondTimer();
	void AutoPauseSearching(bool pause);
	void ManualPauseSearching(bool pause);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void TerminateThread();
	void DoneRetreivingSpoofEntries();
	void SetKeywordManager(KeywordManager* manager);
	void KeywordsUpdated();
	void SupplyRetrievalFailed();
	void InitParent(WinMxDcDll* parent);
	void SupplyManagerSupplyRetrievalFailed();
	void RetreiveNewSpoofEntries();



//	void QueryHitsReceived(char * project_name, vector<QueryHit> &hits);
//	void CalculateProjectSupply(string &project_name);
//	void WriteProjectSupplyToFile(string &project_name);
//	void ReadSupplyFromFiles();
//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	void ProjectSupplySorted(string &project_name);

//	vector<ProjectSupplyFiles> v_project_supply_files;
	vector<SupplyProject> v_supply_projects;
//	vector<ProjectQueryHits> v_project_hits;

private:
	//unsigned int m_minute_counter;
	bool m_5_mins_passed,m_first_round_searching;
	UINT m_second_counter;
//	UINT m_minute_counter;
	ConnectionManager * p_connection_manager;
	int m_supply_interval_multiplier;
	bool m_auto_pause_searching;
	bool m_manual_pause_searching;
	int m_last_searching_index;
	//bool m_reconnected;
	int m_search_counts;
	CTime m_last_search_time;
	int m_delay_reconnect_count;
	bool m_retreiving_spoof_entries;

	SupplyManagerThreadData* p_thread_data;
	CCriticalSection* p_critical_section;
	KeywordManager* p_keyword_manager;
	CWinThread* m_thread;

	//vector<SupplyQuery> v_cached_supply_queries;

	void DeleteAllSpoofFiles();
};