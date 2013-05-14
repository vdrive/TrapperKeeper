#include "TorrentPortalParser.h"

TorrentPortalParser::TorrentPortalParser(btData *data, CListCtrl *list_ctrl)
{
	m_main_list_ctrl = list_ctrl;
	bd = data;
}

TorrentPortalParser::~TorrentPortalParser(void)
{
}

btData * TorrentPortalParser::Parse(string s)
{

	int count = 0;
	int start = (int)s.find("Name");
	int end = (int)s.length();
	
	if(start < 0) {
		if(s.find("No Torrents Found") != -1) return bd;
		Log("Parser ERROR");
		return bd;
	}

	int pos = start;
	while(pos < end && pos > 0) 
	{
		btData * newNode = CreateNode(s, &pos);
		if(bd == NULL) bd = newNode;
		else bd->AddNode(newNode);
		//TRACE("node:%s\n", newNode->ToString().c_str());
		count++;
	}

	char message[32];
	sprintf(message, "total count: %d", count);
	Log(message);

	return bd;
}

btData * TorrentPortalParser::CreateNode(string data, int * pos) {
	string source = "TorrentPortal";
	string title = "";
	string size = "";
	string url = "";
	string seed = "";
	string dl = "";
	string type = "";
	
	int cur_pos = *pos;
	const char * temp = data.c_str();
	url = "http://www.torrentportal.com" + GetValue(data, &cur_pos, "href=\"", "\"");
	cur_pos = (int)data.find("php", cur_pos);
	type = GetValue(data, &cur_pos, "\">", "<");
	type = GetType(type);
	title = GetValue(data, &cur_pos, "<b>", "</b");
	cur_pos = (int)data.find("Verdana", cur_pos);
	size = GetValue(data, &cur_pos, ">", "<");
	seed = GetValue(data, &cur_pos, "\">", "<");
	dl = GetValue(data, &cur_pos, "\">", "<");
	
	btData * newData = new btData(title, size, url, type, seed, dl, source);
	cur_pos = (int)data.find("<tr>", cur_pos); // category

	*pos = cur_pos;
	return newData;
}

int TorrentPortalParser::GetNumPages(string data) { 
	const char * temp = data.c_str();
	int cur_pos = 0;
	int index = (int)data.find("Prev");
	if(index == -1) return 1;
	cur_pos = index;
	int end = (int)data.find("Type", cur_pos);
	if(data.find("page=", cur_pos) == -1) return 1;
	string page = "";
	while(cur_pos != -1 && cur_pos < end) {
		string new_page = GetValue(data, &cur_pos, "page=", "\"");
		if(cur_pos < end) page = new_page;
	}
	return atoi(page.c_str());
}


char * TorrentPortalParser::GetURL() 
{
	
	return "http://www.torrentportal.com/new-torrents.php";
}
    

string TorrentPortalParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
