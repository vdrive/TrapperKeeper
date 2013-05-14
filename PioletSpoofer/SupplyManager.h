// SupplyManager.h
#pragma once
#include "Manager.h"
//#include "ProjectSupplyFiles.h"
#include "SupplyProject.h"
//#include "ProjectQueryHits.h"
//#include "SupplyManagerThreadData.h"
//#include "SupplyQuery.h"
#include "DataBaseInfo.h"
#include "SupplyManagerThreadData.h"
#include <afxmt.h>	// for CCriticalSection


class CCriticalSection;
class PioletSpooferDll;
class ConnectionManager;

class SupplyManager : public Manager
{
public:
	SupplyManager();
	~SupplyManager();

	void InitParent(PioletSpooferDll* parent);
	void InitConnectionManager(ConnectionManager* cm);
	void UpdateProjects(vector<SupplyProject> &supply_projects);
	void UpdatePioletRawDBInfo(DataBaseInfo& raw_db);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	void OnOneMinuteTimer();
//	void LockSupplyProject();
//	void ReleaseSupplyProject();

//	void OnHeartbeat();
//	void QueryHitsReceived(char * project_name, vector<QueryHit> &hits);
//	void CalculateProjectSupply(string &project_name);
	void WriteProjectSupplyToFile();
	void ReadSupplyFromFiles();
	bool IsSupplyProjectsBeingUpdated();
	void DoneRetreivingSpoofEntries();

	//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	void ProjectSupplySorted(string &project_name);

//	vector<ProjectSupplyFiles> v_project_supply_files;
//	vector<ProjectQueryHits> v_project_hits;
	vector<SupplyProject> v_supply_projects;

private:
	bool m_retreiving_spoof_entries;
	bool m_updating_supply_projects;
	//unsigned int m_minute_counter;
//	bool m_5_mins_passed;
//	UINT m_second_counter;
	UINT m_minute_counter;
	PioletSpooferDll * p_parent;
	ConnectionManager * p_connection_manager;
	DataBaseInfo m_piolet_raw_db_info;
	SupplyManagerThreadData* p_thread_data;
	CCriticalSection m_critical_section;
	CCriticalSection* p_critical_section;

	//vector<SupplyManagerThreadData *> v_thread_data;
	//vector<SupplyQuery> v_cached_supply_queries;
	bool RetreiveNewSpoofEntries();

};