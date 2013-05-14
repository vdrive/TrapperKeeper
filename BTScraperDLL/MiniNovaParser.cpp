#include "MiniNovaParser.h"



MiniNovaParser::MiniNovaParser(btData *data, CListCtrl *list_ctrl)
{
	bd = data;
	m_main_list_ctrl = list_ctrl;
}

MiniNovaParser::~MiniNovaParser(void)
{
}



btData * MiniNovaParser::GetList() 
{
	string data = "";
	btData * list = NULL;


	data = GetData("http://mininova.org/yesterday/", GetURL());
	list = Parse(data);
	data = GetData(GetURL());
	list = Parse(data);
	return list;

}

btData * MiniNovaParser::Parse(string s) {
	if(s.find("request failed") == 0) {
		Log("FAILED TO CONNECT: ");
		return bd;
	}
	
	int count = 0;
	int pos = 0;
	int len = (int)s.length();
	pos = (int)s.find(" torrents", pos);
	if(pos < 0) {
		Log("Parse Error");
		//TRACE("Parser Error:\n");
		return bd;
	}
	len = (int)s.find("div id=\"footer\"", pos);
	TRACE("start: %d, len: %d\n", pos, len);
	while(pos < len && pos > 0) 
	{
		btData * newNode = CreateNode(s, &pos);
		if(bd == NULL) bd = newNode;
		else bd->AddNode(newNode);
		//TRACE("node:%s\n", newNode->ToString().c_str());
		count++;
	}

	char temp[16];
	itoa(count, temp, 10);
	char message[32];
	strcpy(message, "total count: ");
	strcat(message, temp);
	Log(message);

	return bd;
}

btData * MiniNovaParser::CreateNode(string data, int * pos) {
	string source = "MiniNova";
	int cur_pos = *pos;
	const char * temp = data.c_str();
	GetValue(data, &cur_pos, "<td>", "</td>"); // time
	string search_type = GetValue(data, &cur_pos, "\">", "<");
	string id = GetValue(data, &cur_pos, "tor/", "\"");
	string title = GetValue(data, &cur_pos, ">", "<");
	string url = "http://mininova.org/get/" + id + "/";// + title + ".torrent";
	string size = GetValue(data, &cur_pos, "\">", "<");
	if(size.find("&nbsp;") != -1) {
		int index = (int)size.find("&nbsp;");
		size = size.substr(0, index) + string(" ") + size.substr(index+6);
	}
	string seed, peers;
	cur_pos = (int)data.find("\">", cur_pos) + 2;
	if(data[cur_pos] != '-') seed = GetValue(data, &cur_pos, "\">", "<");
	else seed = "0";
	cur_pos = (int)data.find("\">", cur_pos) + 2;
	if(data[cur_pos] != '-') peers = GetValue(data, &cur_pos, "\">", "<");
	else peers = "0";


	if(search_type.find("Music") == 0) search_type = "AUDIO";
	else if(search_type.find("DVD-R") == 0 || search_type.find("Film") == 0 || search_type.find("Movies") != -1 || 
		search_type.find("TV") == 0 || search_type.find("Video") == 0 || search_type.find("Anime") == 0) search_type = "VIDEO";
	else if(search_type.find("Software") == 0 || search_type.find("Application") == 0 || search_type.find("Games") == 0) search_type = "SOFTWARE";
	else {
		//TRACE("setting %s to OTHER\n", search_type.c_str());
		search_type = "OTHER";
	}


	btData * newData = new btData(title, size, url, search_type, seed, peers, source);
	//cur_pos = (int)data.find("<td width=\"52%\" nowrap", cur_pos);
	*pos = cur_pos;
	return newData;
}


string MiniNovaParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
	
char * MiniNovaParser::GetURL() {

	return "http://mininova.org/";
}
