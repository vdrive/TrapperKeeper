#pragma once
#include "stdafx.h"
#include "WebSearcher.h"

class TorrentSpySearcher :
	public WebSearcher
{
public:
	TorrentSpySearcher();
	~TorrentSpySearcher(void);

	void FormURLs();
};
