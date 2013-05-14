// ProjectSupplyQuery.h
#pragma once

#include "ProjectSupplyQueryKeywords.h"

class ProjectKeywords;
class ProjectStatus;

class ProjectSupplyQuery
{
public:
	// Public Member Functions
	ProjectSupplyQuery();
	ProjectSupplyQuery(const ProjectSupplyQuery &psq);	// copy constructor
	~ProjectSupplyQuery();
	void Clear();

	ProjectSupplyQuery& operator=(const ProjectSupplyQuery &psq);

	void ExtractProjectKeywordData(ProjectKeywords *keywords);

	// Public Data Members	
	string m_project;
	GUID m_guid;
	UINT m_project_id;

	string m_search_string;
	unsigned int m_supply_size_threshold;
	vector<ProjectSupplyQueryKeywords> v_keywords;
	vector<char *> v_killwords;

	unsigned int m_project_status_index;
	bool m_audio_project;

//	bool m_is_searcher_query;

private:
	void ExtractKeywordsFromKeywordString(char *keyword_string,vector<char *> &keywords);
};