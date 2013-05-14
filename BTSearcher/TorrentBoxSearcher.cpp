#include "StdAfx.h"
#include "TorrentBoxsearcher.h"
#include "TBParser.h"

TorrentBoxSearcher::TorrentBoxSearcher(void)
{
	parser = new TBParser(list, NULL);
}

TorrentBoxSearcher::~TorrentBoxSearcher(void)
{
}




void TorrentBoxSearcher::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	Log("IsoHuntSearcher::FormURLs()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// http://www.torrentbox.com/torrents-search.php?search=live&cat=0&onlyname=yes&submit=Search
		strcpy(url, "http://www.torrentbox.com/torrents-search.php?search=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "+");
			strcat(url, items[j].c_str());
		}
		strcat(url, "&cat=0&onlyname=yes&submit=Search&pg=");
		v_urls.push_back(url);
	}
}

