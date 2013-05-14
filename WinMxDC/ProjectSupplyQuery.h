// ProjectSupplyQuery.h
#pragma once

#include "ProjectSupplyQueryKeywords.h"
#include "ProjectKeywords.h"

//class ProjectKeywords;
class ProjectStatus;

class ProjectSupplyQuery
{
public:
	// Public Member Functions
	ProjectSupplyQuery();
	ProjectSupplyQuery(const ProjectSupplyQuery &psq);	// copy constructor
	~ProjectSupplyQuery();
	void Clear();
	bool operator < (const ProjectSupplyQuery&  other)const;
	bool operator < (const unsigned short& other)const;

	ProjectSupplyQuery& operator=(const ProjectSupplyQuery &psq);

	void ExtractProjectKeywordData(ProjectKeywords *keywords);

	// Public Data Members	
	string m_project;
	int m_search_id;

	string m_search_string;
	unsigned int m_supply_size_threshold;
	vector<ProjectSupplyQueryKeywords> v_keywords;
	vector<char *> v_killwords;

	unsigned int m_project_status_index;
	ProjectKeywords::search_type m_search_type;
	unsigned int m_project_id;
	QueryKeywords m_query_keywords;
	bool m_is_searching;


//	bool m_is_searcher_query;

private:
	void ExtractKeywordsFromKeywordString(char *keyword_string,vector<char *> &keywords);
};

class ProjectSupplyQueryHash : public hash_compare <ProjectSupplyQuery>
{
public:
	using hash_compare<ProjectSupplyQuery>::operator();
	size_t operator()(const ProjectSupplyQuery& key)const
	{
		return key.m_search_id;
	}
};