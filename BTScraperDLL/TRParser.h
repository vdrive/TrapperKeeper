#pragma once

#include "BTScraper.h"

class TRParser :
	public BTScraper
{
public:
	TRParser(btData *, CListCtrl *list_ctrl);
	~TRParser(void);
	btData * Parse(string );
	char * GetURL();
	string Read(char *);
	int GetNumPages(string data) { return 0; }

	bool b_search;
protected:

private:
	btData * CreateNode(string data, int * pos);
	string GetType(string type);
};
