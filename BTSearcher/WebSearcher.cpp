#include "projectmanager.h"
#include "WebSearcher.h"
#include "TorrentSpyParser.h"
#include "SQLinterface.h"
#include "HTTPConnector.h"


WebSearcher::WebSearcher(void)
{
	parser = NULL;
	list = NULL;
	m_main_list_ctrl = NULL;
}

WebSearcher::~WebSearcher(void)
{
	if(list != NULL) {
		list->Clear();
		delete list;
	}
}

void WebSearcher::Init(vector<ProjectData> * projects, CListCtrl *list_ctrl)
{
	v_project_data = projects;
	m_main_list_ctrl = list_ctrl;
	if(parser != NULL) {
		parser->Init(list_ctrl);
		parser->b_search = true;
	}

}

void WebSearcher::Collect() 
{

	Log("Collect Started");
	bool b_post = parser->NeedPostData();
	
	FormURLs(); // populates v_urls, if post data, this will have the post info
		
	char url[1024];
	char msg[1024];
	for(unsigned int i = 0; i < v_urls.size(); i++) {
		//if(i < 449) continue;
		sprintf(url, "%s%d", v_urls[i].c_str(), GetFirstPage());
		sprintf(msg, "(%d/%d) URLS: %s\n", i, v_urls.size(), url);
		Log(msg);
		TRACE(msg);
		string data = GetData(url, "");
		list = parser->Parse(data);
		int num_pages = parser->GetNumPages(data);
		for(int j = 1; j < num_pages; j++) {
			sprintf(url, "%s%d", v_urls[i].c_str(), j+GetFirstPage());
			TRACE("URLS: %s\n", url);
			if(j%5 == 1) Log(url);
			data = GetData(url, "");
			list = parser->Parse(data);
			if(j%100 == 1) {
				Write(list, (*v_project_data)[i]);
				parser->Clear();
				list = NULL;
			}
		}
		Write(list, (*v_project_data)[i]);
		parser->Clear();
		list = NULL;
	}

	Log("Collect Done");
}

void WebSearcher::Write(btData * list, ProjectData pd) 
{
	SQLInterface sql;
	btData * curr = list;
	char buf[1024];
	string insert_query = "insert ignore into supply_data (project_id, url, title, search_string, seed, size, files, date_found, times_seen, owner, source, deleted) values ";
	string update_query = "update supply_data set times_seen=times_seen+1, last_seen=curdate() where (last_seen is null or last_seen <> curdate()) and url in (";
	vector<string> v_update_seed_queries;
	bool b_first = true;
	bool b_valid = false;
	if(list == NULL) return;
	int count = 0;
	int found = 0;
	while (curr!= NULL) {
		string title = curr->GetTitle();
		string url = curr->GetURL();
		int seed = curr->GetSeeds();
		unsigned int file_size = curr->GetSize();
		string size = curr->GetSizeStr();
		int files = curr->GetFiles();
		string type = curr->GetType();
		string source = curr->GetSource();
		//string project_id = curr->GetProjectID();
		count++;
		if((type.compare(pd.m_search_types) != 0 && type.compare("OTHER") != 0 )
			|| file_size < (unsigned int)atoi(pd.m_min_size.c_str())
			|| !ContainsAll(title, pd.m_search_strings) || ContainsAny(title, pd.m_kill_words)) {
			curr = curr->GetNext();
			continue;
		}
		found++;
		b_valid = true;
		sprintf(buf, "(%s, '%s', '%s', '%s', %d, '%s', %d, curdate(), 0, '%s', '%s', 'N')", pd.m_id.c_str(), url.c_str(), sql.EscapedString(title).c_str(), 
			pd.m_search_strings.c_str(), seed, size.c_str(), files, pd.m_owner.c_str(), source.c_str());
		if(b_first) {
			insert_query += string(buf);
			update_query += string("'") + url + string("'");
			b_first = false;
		} else {
			insert_query += string(",") + string(buf);
			update_query += string(",'") + url + string("'");
		}
		
		sprintf(buf, "update supply_data set seed = %d where url = '%s'", seed, url.c_str());
		v_update_seed_queries.push_back(string(buf));
		//TRACE("node: %s\n", curr->ToString().c_str());
		curr = curr->GetNext();
	}

	update_query += ")";
	if(!b_valid) return;
	MYSQL conn;
	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");
	TRACE("Insert query %s\n", insert_query.c_str());
	TRACE("Update query %s\n", update_query.c_str());
	TRACE("%d/%d\n", found, count);
	Log("Inserting...");
	sql.Execute(&conn, insert_query);
	Log("Updating...");
	sql.Execute(&conn, update_query);
	Log("Updating seed counts..");
	for(unsigned int i = 0; i < v_update_seed_queries.size(); i++) {
		sql.Execute(&conn, v_update_seed_queries[i]);
	}
	sql.CloseDB(&conn);

}

void WebSearcher::FormURLs()
{
}

vector<string> WebSearcher::GetSearchString(string temp)
{
	vector<string> items;
	int start = 0;
	int index = (int)temp.find(" ");
	string substring = "";
	while(index > 0) {
		substring = temp.substr(start, index - start);
		if(substring.compare("mp3") != 0) items.push_back(substring);
		start = index+1;
		index = (int)temp.find(" ", start);
	}
	substring = temp.substr(start);
	if(substring.compare("mp3") != 0) items.push_back(substring);
	return items;
}

bool WebSearcher::ContainsAll(string title, string temp) {
	//TRACE("comparing %s to %s\n", title.c_str(), temp.c_str());
	CString lower = title.c_str();
	lower = lower.MakeLower();
	title = lower.GetString();

	lower = temp.c_str();
	lower = lower.MakeLower();
	temp = lower.GetString();

	vector<string> search_items;
	int start = 0;
	int index = (int)temp.find(" ");
	string substring = "";
	while(index > 0) {
		substring = temp.substr(start, index - start);
		if(substring.compare("mp3") != 0) search_items.push_back(substring);
		start = index+1;
		index = (int)temp.find(" ", start);
	}
	substring = temp.substr(start);
	if(substring.compare("mp3") != 0) search_items.push_back(substring);
	
	bool b_found = true; 

	index = -1;
	const char *c_title = title.c_str();

	for(unsigned int i = 0; i < search_items.size(); i++) {
		string item = search_items[i];
		if(search_items.size() > 2 && (item.compare("and") == 0 || item.compare("the") == 0)) continue;

		if(index < (int)title.find(item, (index+1)))
		{	
			index = (int)title.find(item, (index+1));
		}
		else {
			b_found = false;
			break;
		}
		if((index == 0 || !(iswalpha(c_title[index-1]))) && 
			((index+item.length() == title.length()) || !(iswalpha(c_title[index+item.length()])))) {
			//TRACE("MATCH: %s, >%s<, %s\n", title.c_str(), item.c_str(), temp.c_str());
			b_found = true;
		}
		else {
			//TRACE("NO MATCH: %s, >%s<, %s\n", title.c_str(), item.c_str(), temp.c_str());
			i--;
			continue;
		}
	}
	if(b_found) {
		//TRACE("found: title=%s, temp=%s\n", title.c_str(),temp.c_str());
	}
	else  {
		//TRACE("not found: title=%s, temp=%s\n", title.c_str(),temp.c_str());
	}

	return b_found;
}

bool WebSearcher::ContainsAny(string title, string temp) {
	//TRACE("comparing %s to %s\n", title.c_str(), temp.c_str());
	CString lower = title.c_str();
	lower = lower.MakeLower();
	title = lower.GetString();

	vector<string> searchItems;
	int start = 0;
	int index = (int)temp.find(" ");
	while(index > 0) {
		searchItems.push_back(temp.substr(start, index - start));
		start = index+1;
		index = (int)temp.find(" ", start);
	}
	searchItems.push_back(temp.substr(start));
	
	bool found = false; 
	vector <string>::iterator v1_Iter;

	int indexFound = -1;
	for(v1_Iter = searchItems.begin(); v1_Iter != searchItems.end( ) ;v1_Iter++) {
		string item = *v1_Iter;
		if(item == "") continue;
		lower = item.c_str();
		lower = lower.MakeLower();
		item = lower.GetString();
		
		
		if(title.find(item) != -1) {
			//Log(string("title: "+title+" killword: "+item).c_str());
			TRACE(string("title: "+title+" killword: "+item+"\n").c_str());
			return true;
		}
	
	}
	return found;
}

void WebSearcher::Log(string msg)
{
	Log(msg.c_str());
}

void WebSearcher::Log(const char * buf)
{
	char msg[4096];
	if(strlen(buf) > 4005) {
		memcpy(msg, buf, 4005);
		msg[4005] = '\0';
		Log(msg);
		return;
	}
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	strcat(msg, buf);

	int count = m_main_list_ctrl->GetItemCount();
	if(count > 1000) m_main_list_ctrl->DeleteItem(1000);

	m_main_list_ctrl->InsertItem(0, msg);

}

