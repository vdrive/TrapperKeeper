#pragma once

class SearchInfo
{
public:
	SearchInfo(SearchInfo &si);
	~SearchInfo(void);
	vector<string> GetKeywords();
	string GetType();
	string GetProject();

    SearchInfo * GetNext();
	void Add(SearchInfo *si);

	SearchInfo & operator=(SearchInfo &si);

	static SearchInfo * GetSearchInfo();

protected:
	void SetKeywords(vector<string> keywords);
	void SetKeywords(string keywords); // space deliminated list of keywords
	void SetType(string type);
	void SetProject(string project);
	SearchInfo(void);

    vector<string> v_keywords;
	string m_type;
	string m_project;
	SearchInfo * next;
};
