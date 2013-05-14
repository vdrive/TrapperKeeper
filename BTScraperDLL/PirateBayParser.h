#pragma once
#include "btscraper.h"

class PirateBayParser :
	public BTScraper
{
public:
	PirateBayParser(btData *, CListCtrl *list_ctrl);
	~PirateBayParser(void);

	btData * GetList();
	btData * Parse(string input);
	string Read(char *);
	
	char * GetURL(void);
	int GetNumPages(string data);
private:
	btData * CreateNode(string data, int * pos);
};
