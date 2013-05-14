#include "BTScraper.h"

class TorrentPortalParser :
	public BTScraper
{
public:
	TorrentPortalParser(btData *, CListCtrl *list_ctrl);
	~TorrentPortalParser(void);
	btData * Parse(string );
	char * GetURL();
	string Read(char *);
	int GetNumPages(string data);
protected:

private:
	btData * CreateNode(string data, int * pos);
};
