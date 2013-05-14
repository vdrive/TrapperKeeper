#include "IsoHuntParser.h"

IsoHuntParser::IsoHuntParser(btData *data, CListCtrl *list_ctrl)
{
	m_main_list_ctrl = list_ctrl;
	bd = data;
	b_scrape = true;
}

IsoHuntParser::~IsoHuntParser(void)
{
}

btData * IsoHuntParser::Parse(string s)
{

 	int count = 0;
	int start = (int)s.find("ShowTip('Age')");
	int end = (int)s.length();
	
	if(start < 0) {
		if(s.find("Search returned") != -1) return bd;
		Log("Parser ERROR");
		return bd;
	}

	int pos = (int)s.find("hlRow",start);
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
	//Log(message);

	return bd;
}

btData * IsoHuntParser::CreateNode(string data, int * pos) {
	string source = "IsoHunt";
	string title = "";
	string size = "";
	string url = "";
	string file = "";
	string seed = "";
	string dl = "";
	string main_type = "OTHER";
	
	int cur_pos = *pos;
	const char * temp = data.c_str();
	int temp_pos = 0;
	if(b_scrape) {
		GetValue(data, &cur_pos, "1>", "<"); // age
		main_type = GetType(GetValue(data, &cur_pos, "3>", "<"));
		url = "http://isohunt.com/" + GetValue(data, &cur_pos, "href=\"", "\"");
		title = GetValue(data, &cur_pos, ">", "<");
		size = GetValue(data, &cur_pos, "3>", "<") + "MB";
		file = GetValue(data, &cur_pos, "3>", "<");
		seed = GetValue(data, &cur_pos, "3>", "<");
		dl = GetValue(data, &cur_pos, "3>", "<");
	}
	else {
		main_type = GetType(GetValue(data, &cur_pos, "3>", "<"));
		url = "http://isohunt.com/" + GetValue(data, &cur_pos, "href=\'", "\'");
		title = GetValue(data, &cur_pos, "a>", "</td");
		title = RemoveTags(title);

		size = GetValue(data, &cur_pos, "3\">", "<") + "MB";
		file = GetValue(data, &cur_pos, "3\">", "<");
		seed = GetValue(data, &cur_pos, "3\">", "<");
		dl = GetValue(data, &cur_pos, "3\">", "<");
	}
	btData * newData = new btData(title, size, url, main_type, seed, dl, source);
	cur_pos = (int)data.find("hlRow", cur_pos);

	*pos = cur_pos;
	return newData;
}

char * IsoHuntParser::GetURL() 
{
	
	return "http://isohunt.com/latest.php?mode=bt";
}
    

string IsoHuntParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}

int IsoHuntParser::GetNumPages(string data) 
{
	int pos = 0;
	pos = (int)data.find("Page");
	if(pos == -1) return 1;
	string page_str = GetValue(data, &pos, "of ", "<");
	if(page_str.length() > 0) 
		return atoi(page_str.c_str());
	return 1;
	
}

string IsoHuntParser::RemoveTags(string in)
{
	 //<span class=hl>21.Grams</span>.Limited.DVDR-aNBc
	string output = "";
	int cur_pos = 0;
	while(in.find("<", cur_pos) != -1) {
		output += GetValue(in, &cur_pos, "", "<");
		//TRACE("rt>%s\n", output.c_str());
		if(GetValue(in, &cur_pos, "<", ">").compare("br") == 0) output += " ";
		cur_pos++;
		//TRACE("rt>%s\n", output.c_str());
	}
	output += GetValue(in, &cur_pos, "", "");
	//TRACE("rt>%s\n", output.c_str());
	return output;
}