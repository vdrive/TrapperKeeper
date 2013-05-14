#pragma once

#include "btData.h"
#include "BTScraperDlg.h"

class BTScraper
{
public:
	BTScraper(btData *, CListCtrl *list_ctrl);
	virtual ~BTScraper(void);

	virtual btData * GetList();
	virtual bool NeedPostData();
	virtual char * GetURL() = 0;

	void Clear(void);

	void Log(const char * message);
	void Log(string message);

	string GetData(const char *url, const string referer="");
	string GetValue(const string data, int * pos, string startStr, string endStr); 
	btData *bd;

	virtual int GetNumPages(string data) = 0;
	virtual btData * Parse(string ) = 0;
	void Init(CListCtrl *list_ctrl);

	bool b_search;

protected:
	virtual string Read(char *) = 0;
	virtual string Read();
	BTScraper(void );
	CListCtrl  * m_main_list_ctrl;
	string GetType(string search_type);
private:
	
};
