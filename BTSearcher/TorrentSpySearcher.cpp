#include "projectmanager.h"
#include "TorrentSpySearcher.h"
#include "TorrentSpyParser.h"
#include "SQLinterface.h"
#include "HTTPConnector.h"


TorrentSpySearcher::TorrentSpySearcher(void)
{
	parser = new TorrentSpyParser(list, NULL);
}

TorrentSpySearcher::~TorrentSpySearcher(void)
{
}




void TorrentSpySearcher::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	Log("TorrentSpySearcher::Form()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// http://www.torrentspy.com/search.asp?mode=advanced&query=50%20cent%20&maincat=3&subcat=0&db=2&pg=1
		//strcpy(url, "http://www.torrentspy.com/search.asp?mode=advanced&query=");
		strcpy(url, "http://www.torrentspy.com/search.asp?query=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "+");
			strcat(url, items[j].c_str());
		}
		//strcat(url, "&maincat=0&subcat=0&db=2&pg=");
		strcat(url, "&db=2&pg=");
		v_urls.push_back(url);
	}
}

