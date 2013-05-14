#pragma once
#include "../tkcom/tkcominterface.h"
#include "../DCMaster/ProjectChecksums.h"
#include "../DCMaster/ProjectKeywords.h"

class KazaaSupplyTakerDll;

class KazaaSupplyTakerCom :
	public TKComInterface
{
public:
	KazaaSupplyTakerCom(void);
	~KazaaSupplyTakerCom(void);

	void InitParent(KazaaSupplyTakerDll *dll);
	void AnalyzeProjects(char *source_ip, ProjectChecksums current_projects);
	void RemoveProjects(vector<string> & removed_projects);
	void NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords);
	void WriteKeywordDataToFile();
	void ReadKeywordDataFromFile();
	void RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update);

	KazaaSupplyTakerDll *p_dll;

	bool m_reading_new_projects;

protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
