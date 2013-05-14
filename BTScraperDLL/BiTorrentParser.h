#pragma once
#include "BTScraper.h"

class BiTorrentParser :
	public BTScraper
{
public:
	BiTorrentParser(BTScraperDlg * dialog);
	BiTorrentParser(btData *, BTScraperDlg * dialog);
	~BiTorrentParser(void);
	btData * Parse(string );
	string Read(char *);

	char * GetURL();
private:
	btData * CreateNode(vector<string> data, vector<string> types);
	vector<string> m_types;
};
