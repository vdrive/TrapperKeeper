// SupplyManager.h
#pragma once
#include "Manager.h"
//#include "ProjectSupplyFiles.h"
#include "SupplyProject.h"
//#include "ProjectQueryHits.h"
#include "SupplyManagerThreadData.h"
#include "DataBaseInfo.h"
//#include "SupplyQuery.h"
#include <afxmt.h>	// for CCriticalSection


class GnutellaProtectorDll;
class ConnectionManager;

class SupplyManager : public Manager
{
public:
	SupplyManager();
	~SupplyManager();

	void InitParent(GnutellaProtectorDll* parent);
	void InitConnectionManager(ConnectionManager* cm);
	void UpdateProjects(vector<SupplyProject> &supply_projects);
	void UpdateGnutellaRawDBInfo(DataBaseInfo& raw_db);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	void DoneRetreivingSpoofEntries();
	void AbortedRetreivingSpoofEntries();

	//void OnHeartbeat();
	void OnOneMinuteTimer();
//	void QueryHitsReceived(char * project_name, vector<QueryHit> &hits);
//	void CalculateProjectSupply(string &project_name);
	void WriteProjectSupplyToFile();
	void ReadSupplyFromFiles();
//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	void ProjectSupplySorted(string &project_name);

//	vector<ProjectSupplyFiles> v_project_supply_files;
	vector<SupplyProject> v_supply_projects;
//	vector<ProjectQueryHits> v_project_hits;
	bool IsSupplyProjectsBeingUpdated();
	void TerminateThread();
	void RetrieveSupplyEntriesWithZeroSize();
	void KeywordsUpdated();

private:
	//unsigned int m_minute_counter;
//	bool m_5_mins_passed;
//	UINT m_second_counter;
	//bool m_retrieve_spoof_entries_again;
	bool m_updating_supply_projects;
	UINT m_minute_counter;
	GnutellaProtectorDll * p_parent;
	ConnectionManager * p_connection_manager;
	//vector<SupplyManagerThreadData *> v_thread_data;
	//vector<SupplyQuery> v_cached_supply_queries;
	DataBaseInfo m_gnutella_raw_db_info;
	SupplyManagerThreadData* p_thread_data;
	//CCriticalSection m_critical_section;
	CCriticalSection* p_critical_section;
	bool m_retreiving_spoof_entries;
	CWinThread* m_thread;
	vector<SupplyProject> v_tmp_supply_projects;

	bool RetreiveNewSpoofEntries();
};