#include "StdAfx.h"
#include "TorrentPortalSearcher.h"
#include "TorrentPortalParser.h"

TorrentPortalSearcher::TorrentPortalSearcher(void)
{
	parser = new TorrentPortalParser(list, NULL);
}

TorrentPortalSearcher::~TorrentPortalSearcher(void)
{
}




void TorrentPortalSearcher::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	Log("TorrentPortalSearcherSearcher::FormURLs()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// http://www.torrentportal.com/torrents-search.php?search=family+guy&cat=0&cat=0
		strcpy(url, "http://www.torrentportal.com/torrents-search.php?search=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "+");
			strcat(url, items[j].c_str());
		}
		strcat(url, "&page=");
		v_urls.push_back(url);
	}
}

