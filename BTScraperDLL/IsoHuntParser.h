#pragma once

#include "BTScraper.h"

class IsoHuntParser :
	public BTScraper
{
public:
	IsoHuntParser(btData *, CListCtrl*);
	~IsoHuntParser(void);
	btData * Parse(string );
	char * GetURL();
	string Read(char *);
	int GetNumPages(string data);
	bool b_scrape;
protected:
	string RemoveTags(string in);

private:
	btData * CreateNode(string data, int * pos);
};
