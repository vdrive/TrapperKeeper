#include "StdAfx.h"
#include "TorrentReactorsearcher.h"
#include "httpconnector.h"
#include "TRParser.h"


TorrentReactorSearcher::TorrentReactorSearcher(void)
{
	parser = new TRParser(list, NULL);
	((TRParser*)parser)->b_search = true;
}

TorrentReactorSearcher::~TorrentReactorSearcher(void)
{
}


void TorrentReactorSearcher::FormURLs()
{
	v_post_info.clear();
	char info[1024+1];
	Log("TorrentReactorSearcher::FormURLs()");
	for(unsigned int i = 0; i < (*v_project_data).size(); i++) {
		ProjectData pd = (*v_project_data)[i];
		// q=28+days+later&lang=All
		strcpy(info, "q=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(info, "+");
			strcat(info, items[j].c_str());
		}
		strcat(info, "&lang=All");
		v_post_info.push_back(info);
	}
}

void TorrentReactorSearcher::Collect() 
{
	char * base_url = "http://www.torrentreactor.to/torrents/search";

	Log("Collect Started");
	bool b_post = parser->NeedPostData();
	
	FormURLs(); // populates v_urls, if post data, this will have the post info
		
	char msg[1024];
	for(unsigned int i = 0; i < v_post_info.size(); i++) {
		sprintf(msg, "(%d/%d) URLS: %s\n", (i+1), v_post_info.size(), v_post_info[i].c_str());
		Log(msg);
		TRACE(msg);
		string data = PostData(base_url, "http://www.torrentreactor.to/torrents/search", v_post_info[i]); 
		list = parser->Parse(data);
		//if(list == NULL) TRACE("not found: %s\n", v_post_info[i].c_str());
		Write(list, (*v_project_data)[i]);
		parser->Clear();
		list = NULL;
	}

	Log("Collect Done");
}