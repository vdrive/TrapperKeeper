#pragma once
#include "websearcher.h"

class TorrentPortalSearcher :
	public WebSearcher
{
public:
	TorrentPortalSearcher(void);
	~TorrentPortalSearcher(void);

	void FormURLs();
	int GetFirstPage() { return 0; }
};
