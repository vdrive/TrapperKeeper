#include "StdAfx.h"
#include "isohuntSearcher.h"
#include "isohuntparser.h"

IsoHuntSearcher::IsoHuntSearcher(void)
{
	parser = new IsoHuntParser(list, NULL);
	((IsoHuntParser*)parser)->b_scrape = false;
}

IsoHuntSearcher::~IsoHuntSearcher(void)
{
}

void IsoHuntSearcher::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	Log("IsoHuntSearcher::FormURLs()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// http://isohunt.com/torrents.php?ihq=untold+story&ext=&op=and
		strcpy(url, "http://isohunt.com/torrents.php?ihq=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "+");
			strcat(url, items[j].c_str());
		}
		strcat(url, "&ext=&op=and&ihs1=18&iho1=d&iht=-1&ihp=");
		v_urls.push_back(url);
	}
}

