#pragma once
#include "websearcher.h"

class TorrentBoxSearcher :
	public WebSearcher
{
public:
	TorrentBoxSearcher(void);
	~TorrentBoxSearcher(void);

	void FormURLs();
};
