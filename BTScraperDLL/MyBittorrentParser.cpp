#include "MyBittorrentParser.h"



MyBittorrentParser::~MyBittorrentParser(void)
{
}

MyBittorrentParser::MyBittorrentParser(btData *data, CListCtrl * list_ctrl) 
{
	bd = data;
	m_main_list_ctrl = list_ctrl;
}


bool MyBittorrentParser::NeedPostData()
{
	return true;
}

btData * MyBittorrentParser::GetList() 
{
	string data = "";
	btData * list = NULL;


	data = GetData(GetURL());
	list = Parse(data);
	return list;

}

btData * MyBittorrentParser::Parse(string s) {
	if(s.find("request failed") == 0) {
		Log("FAILED TO CONNECT: ");
		return bd;
	}
	
	int count = 0;
	int pos = 0;
	int len = (int)s.length();
	pos = (int)s.find("Top Releases", pos);
	if(pos < 0) {
		Log("Parse Error");
		//TRACE("Parser Error:\n");
		return bd;
	}
	
	pos = (int)s.find("class=\"Name\"", pos);
	const char * temp = s.c_str();
	while(pos < len && pos > 0) 
	{
		btData * newNode = CreateNode(s, &pos);
		if(newNode->GetTitle().compare("NO TITLE") == 0) {
			delete newNode;
			break;
		}
		if(bd == NULL) bd = newNode;
		else bd->AddNode(newNode);
		count++;
	}

	char message[32];
	sprintf(message, "total count: %d", count);
	Log(message);

	return bd;
}

btData * MyBittorrentParser::CreateNode(string data, int * pos) {
	string source = "myBittorrent";
	int cur_pos = *pos;
	const char * temp = data.c_str();
	string url = "http://www.mybittorrent.com/" + GetValue(data, &cur_pos, "href=\"", "\"");
	string title = GetValue(data, &cur_pos, ">", "<");
	string type = GetType(GetValue(data, &cur_pos, "\">", "<"));
	string size = GetValue(data, &cur_pos, "\">", "<"); // size
	string seed = GetValue(data, &cur_pos, "\">", "<");
	string peers = GetValue(data, &cur_pos, "\">", "<");


	btData * newData = new btData(title, size, url, type, seed, peers, source);
	cur_pos = (int)data.find("class=\"Name\"", cur_pos);
	*pos = cur_pos;
	
	newData->NeedPostData(true);
	return newData;
}


string MyBittorrentParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
	
char * MyBittorrentParser::GetURL() {

	return "http://www.mybittorrent.com/";
}
