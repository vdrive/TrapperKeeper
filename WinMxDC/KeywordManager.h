// KeywordManager.h
#pragma once
#include "Manager.h"
#include "ProjectKeywords.h"
#include "ProjectChecksums.h"

class ConnectionManager;
#ifdef WINMX_DC
class SupplyManager;
#endif
class KeywordManager : public Manager
{
	ConnectionManager * p_connection_manager;
#ifdef WINMX_DC
	SupplyManager * p_supply_manager;
#endif
public:
	KeywordManager();
	~KeywordManager();

	vector<ProjectKeywords> v_all_project_keywords;

	void CheckProjectChecksums(char* dc_master, ProjectChecksums &current_projects);
	void RemoveProjects(vector<string> & removed_projects);
	void BroadcastUpdateAlert();
	void NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords);

	void WriteKeywordDataToFile();
	void ReadKeywordDataFromFile();

	void SetConnectionManager(ConnectionManager * cm);
#ifdef WINMX_DC
	void SetSupplyManager(SupplyManager *sm);
#endif
	vector<ProjectKeywords>* ReturnProjectKeywords();

private:
	vector<ProjectKeywords> v_project_keywords;

	void CreateProjectSupplyTable();
};