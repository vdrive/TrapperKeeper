#pragma once
#include "websearcher.h"

class TorrentReactorSearcher :
	public WebSearcher
{
public:
	TorrentReactorSearcher(void);
	~TorrentReactorSearcher(void);

	void FormURLs();
	void Collect();

	vector<string> v_post_info;
};
