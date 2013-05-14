#include "projectmanager.h"
#include "WebRemover.h"
#include "TorrentSpyParser.h"
#include "SQLinterface.h"
#include "HTTPConnector.h"


WebRemover::WebRemover(void)
{
	m_main_list_ctrl = NULL;
}

WebRemover::~WebRemover(void)
{
}

void WebRemover::Init(vector<string> * ids, CListCtrl *list_ctrl)
{
	v_ids = ids;
	m_main_list_ctrl = list_ctrl;
}

void WebRemover::Remove() 
{

	Log("Remove Started");
	

	PostData("http://torrentspy.com/login.asp?mode=login", "", "submit=Submit&returnurl=profile.asp&username=UMGAntipiracy&password=%21umg819&image1.x=0&image1.y=0");

	vector<string> v_removed_urls;
	char url[128];
	char referer[128];
	char msg[1024];
	for(unsigned int i = 0; i < v_ids->size(); i++) {
		const char * id = (*v_ids)[i].c_str();
		sprintf(referer, "http://www.torrentspy.com/directory.asp?mode=torrentdetails&id=%s", id);
		sprintf(url, "http://www.torrentspy.com/modadmin.asp?mode=dmca&action=removetorrent&id=%s&referer=", id);
		sprintf(msg, "(%d/%d) URLS: %s\n", i, v_ids->size(), url);
		Log(msg);
		TRACE(msg);
		string data = GetData(url, referer);
		if(Removed(data)) {
			sprintf(url, "http://www.torrentspy.com/download.asp?id=%s", id);
			TRACE("REMOVING: %s\n", url);
			v_removed_urls.push_back(string(url));
			if(i%100 == 0 && i > 0) { 
				Write(v_removed_urls, "torrentspy");
				v_removed_urls.clear();
			}
			if(i == 1000) {
				TRACE("1000\n");
			}
		}
	}
	Write(v_removed_urls, "torrentspy");
	Log("Remove Done");
}

bool WebRemover::Removed(string data)
{
	const char * temp = data.c_str();
	int cur_pos = 0;
	if(data.find("Torrent has been removed") != -1) return true;
	else {
		TRACE("error\n");
	}
	return true;
}


void WebRemover::Write(vector<string> v_urls, string source) 
{
	SQLInterface sql;
	char buf[1024];
	char ids[1024];

	strcpy(ids, "");
	MYSQL conn;
	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");
	for (unsigned int i = 0; i < v_urls.size(); i++) {
		string url = v_urls[i];
		if(strlen(ids) != 0) strcat(ids,",");
		strcat(ids, "'");
		strcat(ids, url.c_str());
		strcat(ids, "'");
		if(i%10 == 0 && i > 0) {
			sprintf(buf, "update supply_data set deleted='Y' where url in (%s) and source = '%s'", ids, source.c_str());
			string update_query = string(buf);
			Log("Updating...");
			sql.Execute(&conn, update_query);
			strcpy(ids, "");
		}
	}
	if(strlen(ids) > 0) {
		sprintf(buf, "update supply_data set deleted='Y' where url in (%s) and source = '%s'", ids, source.c_str());
		string update_query = string(buf);
		Log("Updating...");
		sql.Execute(&conn, update_query);
	}
	sql.CloseDB(&conn);

}

void WebRemover::FormURLs()
{
}

vector<string> WebRemover::GetSearchString(string temp)
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

bool WebRemover::ContainsAll(string title, string temp) {
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

bool WebRemover::ContainsAny(string title, string temp) {
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

void WebRemover::Log(string msg)
{
	Log(msg.c_str());
}

void WebRemover::Log(const char * buf)
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

