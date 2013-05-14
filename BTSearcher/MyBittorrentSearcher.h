#pragma once
#include "websearcher.h"

class MyBittorrentSearcher :
	public WebSearcher
{
public:
	MyBittorrentSearcher(void);
	~MyBittorrentSearcher(void);

	void FormURLs();
};
