// SupplyManager.h
#pragma once
#include "Manager.h"
//#include "ProjectSupplyFiles.h"
//#include "SupplyProject.h"
#include "ProjectQueryHits.h"
#include "ProjectSupplyQuery.h"
#include "ProjectKeywords.h"
//#include "SupplyManagerThreadData.h"
//#include "SupplyQuery.h"

class GnutellaSupplyDll;
class ConnectionManager;

class SupplyManager : public Manager
{
public:
	SupplyManager();
	~SupplyManager();

	void InitParent(GnutellaSupplyDll* parent);
	void InitConnectionManager(ConnectionManager* cm);
	void UpdateProjects(vector<ProjectKeywords> &supply_projects);
	void AlterSupplyIntervalMultiplier(int multi);
	//void OnHeartbeat();
	//void OnOneMinuteTimer();
	void OnOneSecondTimer();
//	void QueryHitsReceived(char * project_name, vector<QueryHit> &hits);
//	void CalculateProjectSupply(string &project_name);
//	void WriteProjectSupplyToFile(string &project_name);
//	void ReadSupplyFromFiles();
//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	void ProjectSupplySorted(string &project_name);

//	vector<ProjectSupplyFiles> v_project_supply_files;
	//vector<SupplyProject> v_supply_projects;
//	vector<ProjectQueryHits> v_project_hits;
	UINT m_supply_progress;

private:
	//unsigned int m_minute_counter;
	bool m_5_mins_passed;
	UINT m_second_counter;
//	UINT m_minute_counter;
	GnutellaSupplyDll * p_parent;
	ConnectionManager * p_connection_manager;
	int m_supply_interval_multiplier;
	UINT m_last_searching_index;

	//vector<SupplyManagerThreadData *> v_thread_data;

	//vector<SupplyQuery> v_cached_supply_queries;
	vector<ProjectSupplyQuery> v_project_supply_queries;
};