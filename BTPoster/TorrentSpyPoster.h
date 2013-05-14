#pragma once
#include "poster.h"

class TorrentSpyPoster :
	public Poster
{
public:
	TorrentSpyPoster(void);
	~TorrentSpyPoster(void);

	//bool Post(char * torrent);
	string GetBeginData(string filename, string name, string boundry);
	string GetEndData(string filename, string name, string boundry);
	string GetPostURL();
	string GetRefererURL();
	string GetMainCategory(); 
	string GetSubCategory(); 
};
