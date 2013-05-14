// KeywordManager.h
#pragma once
#include "Manager.h"
#include "ProjectKeywords.h"
#include "ProjectChecksums.h"

class ConnectionManager;
class SupplyManager;

class KeywordManager : public Manager
{
	ConnectionManager * p_connection_manager;
	SupplyManager * p_supply_manager;

public:
	KeywordManager();
	~KeywordManager();

	vector<ProjectKeywords> v_project_keywords;
	vector<ProjectKeywords> v_all_project_keywords;

	void CheckProjectChecksums(char* dc_master, ProjectChecksums &current_projects);
	void RemoveProjects(vector<string> & removed_projects);
	void BroadcastUpdateAlert();
	void NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords);

	void WriteKeywordDataToFile();
	void ReadKeywordDataFromFile();

	void SetConnectionManager(ConnectionManager * cm);
	void SetSupplyManager(SupplyManager *sm);
};