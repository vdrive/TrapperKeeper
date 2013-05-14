#include "piratebayparser.h"


PirateBayParser::PirateBayParser(btData *data, CListCtrl *list_ctrl)
{
	bd = data;
	m_main_list_ctrl = list_ctrl;
}

PirateBayParser::~PirateBayParser(void)
{
}

btData * PirateBayParser::GetList() 
{
	string data = "";
	btData * list = NULL;
	char temp[16];
	char url[128];

	char * baseURL = "http://thepiratebay.org/recent.php?orderby=&page=";
	for(int i = 10; i > -1; i--) {
		itoa(i, temp, 10);
		strcpy(url, baseURL);
		strcat(url, temp);
		data = GetData(url, GetURL());
		list = Parse(data);
	}
	data = GetData("http://thepiratebay.org/top100.php");
	list = Parse(data);
	return list;
}


btData * PirateBayParser::Parse(string input)
{
	if(input.find("request failed") == 0) {
		Log("FAILED TO CONNECT: ");
		return bd;
	}
	int count = 0;
	int pos = 0;
	int len = (int)input.length();
	pos = (int)input.find("ULed", pos);
	if(pos < 0) {
		if(input.find("Search") != -1) return bd;
		return bd;
		Log("Parse Error");
		return bd;
	}

	pos = (int)input.find("<tr>");
	while(pos < len && pos > 0) 
	{
		//pos = (int)input.find("<tr>", pos);
		btData * newNode = CreateNode(input, &pos);
		if(newNode == NULL) break;
		if(bd == NULL) bd = newNode;
		else bd->AddNode(newNode);
		//Log(newNode->ToString());
		//TRACE("%s\n", newNode->GetTitle().c_str());
		//TRACE("node: %s\n", newNode->ToString().c_str());
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

string PirateBayParser::Read(char *url)
{
	string line = "";
	line = GetData(url);
	return line;
}

int PirateBayParser::GetNumPages(string data)
{
	const char * temp = data.c_str();
	int cur_pos = (int)data.find("&page=1");
	if(cur_pos == -1) return 0;
	int count = 0;
	while(cur_pos > 0) {
		cur_pos = (int)data.find("page=", cur_pos+1);
		if(cur_pos > 0) count++;
	}
	return count;
}

btData * PirateBayParser::CreateNode(string data, int * pos)
{
	string source = "PirateBay";
	int cur_pos = *pos;
	const char * buf = data.c_str();

	if((int)data.find("y\">", cur_pos) == -1) return NULL;;
	string search_type = GetValue(data, &cur_pos, "y\">", " >");
	string title = GetValue(data, &cur_pos, "\">", "<");
	//TRACE("%s\n", title.c_str());
	string date = GetValue(data, &cur_pos, "td>", "<");
	string url = GetValue(data, &cur_pos, "href=\"", "\"");
	string size = GetValue(data, &cur_pos, "right\">", "<"); 
	GetValue(data, &cur_pos, "\">", "<"); // dl
	string seed = GetValue(data, &cur_pos, "\">", "<");
	string peers = GetValue(data, &cur_pos, "\">", "<");

	cur_pos = (int)data.find("y\">", cur_pos);

	if(search_type.find("Audio") == 0) search_type = "AUDIO";
	else if(search_type.find("TV") == 0 || search_type.find("Film") == 0 || search_type.find("Movies") == 0 || 
		search_type.find("Serier") == 0 || search_type.find("Video") == 0) search_type = "VIDEO";
	else if(search_type.find("Applications") == 0 || search_type.find("Games") == 0) search_type = "SOFTWARE";
	else if(search_type.find("Other") == 0) search_type = "OTHER";
	else if(search_type.find("Porn") == 0) search_type = "PORN";
	else {
		//TRACE("setting %s to OTHER\n", search_type.c_str());
		search_type = "OTHER";
	}

	btData * newData = new btData(title, size, url, search_type, seed, peers, source);

	*pos = cur_pos;
	return newData;
}

char * PirateBayParser::GetURL(void) 
{
	return "http://thepiratebay.org/recent.php";
}
