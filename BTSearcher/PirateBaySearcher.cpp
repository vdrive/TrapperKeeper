#include "StdAfx.h"
#include "PirateBaysearcher.h"
#include "PirateBayparser.h"

PirateBaySearcher::PirateBaySearcher(void)
{
	parser = new PirateBayParser(list, NULL);
}

PirateBaySearcher::~PirateBaySearcher(void)
{
}




void PirateBaySearcher::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	Log("PirateBaySearcher::FormURLs()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// http://thepiratebay.org/search.php?q=family%20guy&audio=&video=&apps=&games=&other=&what=search&orderby=&page=0
		strcpy(url, "http://thepiratebay.org/search.php?q=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "%20");
			strcat(url, items[j].c_str());
		}
		strcat(url, "&audio=&video=&apps=&games=&other=&what=search&orderby=&page=");
		v_urls.push_back(url);
	}
}

int PirateBaySearcher::GetFirstPage()
{
	return 0;
}