#pragma once
#include "btscraper.h"

class MyBittorrentParser :
	public BTScraper
{
public:
	MyBittorrentParser(btData *, CListCtrl *list_ctrl);
	~MyBittorrentParser(void);
	btData * GetList();
	btData * Parse(string );
	string Read(char *);
	bool NeedPostData();
	char * GetURL();
	int GetNumPages(string data) { return 0; }
private:
	btData * CreateNode(string data, int * pos);
};
