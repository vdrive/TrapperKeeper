#pragma once
#include "stdafx.h"
#include "ProjectData.h"
#include "btData.h"
#include "btscraper.h"

class WebSearcher
{
public:
	WebSearcher(void);
	~WebSearcher(void);

	void Init(vector<ProjectData> * projects, CListCtrl *list_ctrl);
	virtual void Collect();

	vector<ProjectData> * v_project_data;
	vector<string> v_urls;

	virtual void FormURLs();
	
	BTScraper *parser;
	btData * list;

	vector<string> GetSearchString(string search_string);

	void Write(btData * list, ProjectData pd);
	bool ContainsAny(string title, string temp);
	bool ContainsAll(string title, string temp);

	void Log(string msg);
	void Log(const char *msg);

	virtual int GetFirstPage() { return 1; }
protected:
	CListCtrl  * m_main_list_ctrl;
};
