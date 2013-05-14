#pragma once
#include "btscraper.h"

class TorrentSpyParser :
	public BTScraper
{
public:
	TorrentSpyParser(btData *, CListCtrl * list_ctrl);
	~TorrentSpyParser(void);
	btData * GetList();
	btData * Parse(string );
	string Read(char *);
	int GetNumPages(string data);

	
	char * GetURL();
private:
	btData * CreateNode(string data, int * pos);
};
