// SupplyManager.h
#pragma once
#include "Manager.h"
//#include "ProjectSupplyFiles.h"
#include "SupplyProject.h"
#include "ProjectQueryHits.h"
//#include "SupplyManagerThreadData.h"
//#include "SupplyQuery.h"

class ConnectionManager;
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
	void OnFiveSecondTimer();
	void AutoPauseSearching(bool pause);
	void ManualPauseSearching(bool pause);
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
	bool m_reconnected;
	int m_search_counts;

	//vector<SupplyManagerThreadData *> v_thread_data;

	//vector<SupplyQuery> v_cached_supply_queries;
};