#pragma once
#include "btscraper.h"

class MiniNovaParser :
	public BTScraper
{
public:
	MiniNovaParser(btData *, CListCtrl *list_ctrl);
	~MiniNovaParser(void);
	btData * GetList();
	btData * Parse(string );
	string Read(char *);
	
	char * GetURL();
	int GetNumPages(string data) { return -1; }
private:
	btData * CreateNode(string data, int * pos);
};
