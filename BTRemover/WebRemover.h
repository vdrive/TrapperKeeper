#pragma once
#include "stdafx.h"
#include "ProjectData.h"
#include "btData.h"
#include "btscraper.h"

class WebRemover
{
public:
	WebRemover(void);
	~WebRemover(void);

	void Init(vector<string> * ids, CListCtrl *list_ctrl);
	virtual void Remove();

	vector<string> * v_ids;

	virtual void FormURLs();
	
	vector<string> GetSearchString(string search_string);

	void Write(vector<string> ids, string source);
	bool ContainsAny(string title, string temp);
	bool ContainsAll(string title, string temp);

	void Log(string msg);
	void Log(const char *msg);
	
	bool Removed(string data);
protected:
	CListCtrl  * m_main_list_ctrl;
};
