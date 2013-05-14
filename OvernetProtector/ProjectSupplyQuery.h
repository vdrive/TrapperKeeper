// ProjectSupplyQuery.h
#pragma once

#include "ProjectSupplyQueryKeywords.h"
#include "..\DCMaster\QueryKeywords.h"

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

	void ExtractProjectKeywordData(ProjectKeywords *keywords/*, int track=-1*/);
	void ExtractProjectKeywordTrackData(ProjectKeywords *keywords, int track);

	// Public Data Members	
	string m_project;
	UINT m_project_id;

	string m_search_string;
	unsigned int m_supply_size_threshold;
	vector<ProjectSupplyQueryKeywords> v_keywords;
	vector<char *> v_killwords;
	vector<CString> v_search_words;
	//unsigned int m_project_status_index;
	byte m_keyword_hash[16];
	CString m_search_type;
	QueryKeywords m_query_keywords;
	int m_track;

private:
	void ExtractKeywordsFromKeywordString(char *keyword_string,vector<char *> &keywords);
	void ExtractKeywords(const char* artist, vector<CString>* keywords);
	void RemoveWord(CString& string, const char* word);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
};