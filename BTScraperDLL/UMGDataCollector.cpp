#include "umgdatacollector.h"
#include "projectmanager.h"
#include "TorrentSpyParser.h"
#include "sqlinterface.h"
#include "HTTPConnection.h"


UMGDataCollector::UMGDataCollector(void)
{
	list = NULL;
}

UMGDataCollector::~UMGDataCollector(void)
{
	if(list != NULL) {
		list->Clear();
		delete list;
	}
}


void UMGDataCollector::Collect() 
{

	ProjectManager pm;
	//string owner = "Fox";
	string owner = "Universal Music";
	//string owner = "Universal Pictures";

	string query = 
		"select search_string, search_type, id, owner from projects p ";
	query += 
		//"where (owner like 'Fox%' or owner like 'Universal Music%' or owner like 'Universal Pictures%' ) ";
		"where owner like '"+owner+"%' ";
	query += "and active = 'A' order by id";

	pm.SetQuery(query);
	pm.GetProjectData(&v_project_data);
	FormURLs();
		
	char url[1024];
	TorrentSpyParser parser(list, NULL);
	for(unsigned int i = 0; i < v_urls.size(); i++) {
		sprintf(url, "%s%d", v_urls[i].c_str(), 1);
		TRACE("(%d/%d) URLS: %s\n", i, v_urls.size(), url);
		string data = GetData(url, "");
		list = parser.Parse(data);
		int num_pages = parser.GetNumPages(data);
		for(int j = 1; j < num_pages; j++) {
			sprintf(url, "%s%d", v_urls[i].c_str(), j+1);
			TRACE("URLS: %s\n", url);
			data = GetData(url, "");
			list = parser.Parse(data);
		}
		Write(list, v_project_data[i]);
		parser.Clear();
		list = NULL;
	}

	
}

void UMGDataCollector::Write(btData * list, ProjectData pd) 
{
	SQLInterface sql;
	btData * curr = list;
	char buf[1024];
	string insert_query = "insert ignore into supply_data (project_id, url, title, search_string, seed, size, files, date_found, times_seen, owner) values ";
	string update_query = "update supply_data set times_seen=times_seen+1, last_seen=curdate() where (last_seen is null or last_seen <> curdate()) and url in (";
	vector<string> v_update_seed_queries;
	bool b_first = true;
	bool b_valid = false;
	if(list == NULL) return;
	while (curr!= NULL) {
		string title = curr->GetTitle();
		string url = curr->GetURL();
		int seed = curr->GetSeeds();
		string size = curr->GetSize();
		int files = curr->GetFiles();
		string type = curr->GetType();
		//string project_id = curr->GetProjectID();
		if(type.compare(pd.m_search_types) != 0 && type.compare("OTHER") != 0 || !ContainsAll(title, pd.m_search_strings) || ContainsAny(title, pd.m_kill_words)) {
			curr = curr->GetNext();
			continue;
		}
		b_valid = true;
		sprintf(buf, "(%s, '%s', '%s', '%s', %d, '%s', %d, curdate(), 0, '%s')", pd.m_id.c_str(), url.c_str(), sql.EscapedString(title).c_str(), 
			pd.m_search_strings.c_str(), seed, size.c_str(), files, pd.m_owner.c_str());
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
	sql.Execute(&conn, insert_query);
	sql.Execute(&conn, update_query);
	for(unsigned int i = 0; i < v_update_seed_queries.size(); i++) {
		sql.Execute(&conn, v_update_seed_queries[i]);
	}
	sql.CloseDB(&conn);

}

void UMGDataCollector::FormURLs()
{
	v_urls.clear();
	char url[1024+1];
	for(unsigned int i = 0; i < v_project_data.size(); i++) {
		ProjectData pd = v_project_data[i];
		// http://www.torrentspy.com/search.asp?mode=advanced&query=50%20cent%20&maincat=3&subcat=0&db=2&pg=1
		strcpy(url, "http://www.torrentspy.com/search.asp?mode=advanced&query=");
		vector<string> items = GetSearchString(pd.m_search_strings);
		for(unsigned int j=0; j < items.size(); j++) {
			if(j!=0) strcat(url, "+");
			strcat(url, items[j].c_str());
		}
		strcat(url, "&maincat=0&subcat=0&db=2&pg=");
		v_urls.push_back(url);
	}
}

vector<string> UMGDataCollector::GetSearchString(string temp)
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

bool UMGDataCollector::ContainsAll(string title, string temp) {
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
		if(item.compare("b") == 0 || (search_items.size() > 2 && (item.compare("and") == 0 || item.compare("the") == 0))) continue;

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
		TRACE("not found: title=%s, temp=%s\n", title.c_str(),temp.c_str());
	}

	return b_found;
}

bool UMGDataCollector::ContainsAny(string title, string temp) {
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
