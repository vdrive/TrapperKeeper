#pragma once

#include "BTScraper.h"

class TBParser :
	public BTScraper
{
public:
	TBParser(btData *, CListCtrl *list_ctrl);
	~TBParser(void);
	btData * Parse(string );
	char * GetURL();
	string Read(char *);
	int GetNumPages(string data) { return 0; }
protected:

private:
	btData * CreateNode(string data, int * pos);
	int num_types;
	void Init();
	void Init(btData *);
};
