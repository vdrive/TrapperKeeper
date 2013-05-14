#pragma once
#include "stdafx.h"
#include "ProjectData.h"
#include "btData.h"

class UMGDataCollector
{
public:
	UMGDataCollector(void);
	~UMGDataCollector(void);

	void Collect();

	vector<ProjectData> v_project_data;
	vector<string> v_urls;

	void FormURLs();
	btData * list;

	vector<string> GetSearchString(string search_string);

	void Write(btData * list, ProjectData pd);
	bool ContainsAny(string title, string temp);
	bool ContainsAll(string title, string temp);
};
