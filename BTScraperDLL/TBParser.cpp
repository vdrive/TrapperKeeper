#include "TBParser.h"


TBParser::TBParser(btData *data, CListCtrl *list_ctrl)
{
	bd = data;
	m_main_list_ctrl = list_ctrl;
}

TBParser::~TBParser(void)
{
}


btData * TBParser::Parse(string s)
{
	if(s.find("request failed") == 0) {
		TRACE("FAILED TO CONNECT: %s\n", GetURL());
		return bd;
	}
	
	int count = 0;
	int start = (int)s.find("torrents-browse.php");
	if(start < 0) {
		if(s.find("Your search did not result in any matches") > 0) return bd;
		Log("Parse Error");
		TRACE("PARSER ERROR: %s\n", GetURL());
		return bd;
	}
	//start = (int)s.find("<table", start);
	
	//int length = (int)s.find("</table", start) - start;

	if(start < 0) return bd;

	int pos = start;
	while(pos > 0) // goes through all tokens to find the ones with the data 
	{
		count++;
		btData * newData = CreateNode(s, &pos);
		if(bd != NULL) {
			bd->AddNode(newData);
		} else {
			bd = newData;
		} 		
		pos = (int)s.find("www.torrentbox.com/torrents-browse.php", pos);
	} 
	char temp[16];
	itoa(count, temp, 10);
	char message[32];
	strcpy(message, "total count: ");
	strcat(message, temp);
	Log(message);

	return bd;
}

btData * TBParser::CreateNode(string data, int * pos) {
	int cur_pos = *pos;
	string source = "TorrentBox";
	string title = "";
	string size = "";
	string url = "";
	string seed = "";
	string leech = "";
	string search_type = "";

	search_type = GetValue(data, &cur_pos, "<u>", "</");
	title = GetValue(data, &cur_pos, "<align=left>&nbsp &nbsp;", "<");
	url = GetValue(data, &cur_pos, "href=\"", "\"");
	GetValue(data, &cur_pos, "na>", "<");
	size = GetValue(data, &cur_pos, "na>", "<");
	GetValue(data, &cur_pos, "na>", "<"); // dl 
	seed = GetValue(data, &cur_pos, "na>", "<");
	leech = GetValue(data, &cur_pos, "na>", "<");

	if(search_type.find("Music") == 0) search_type = "AUDIO";
	else if(search_type.find("DVD-R") == 0 || search_type.find("Film") == 0 || search_type.find("Movies") == 0 || 
		search_type.find("TV") == 0 || search_type.find("Video") == 0) search_type = "VIDEO";
	else if(search_type.find("Apps") == 0 || search_type.find("Games") == 0) search_type = "SOFTWARE";
	else search_type = "OTHER";

	btData * newData = new btData(title, size, url, search_type, seed, leech, source);
	newData->SetRefererURL(GetURL());

	*pos = cur_pos;
	return newData;

}

char * TBParser::GetURL() 
{
	return "http://www.torrentbox.com/index.php";

}

string TBParser::Read(char *url)
{
	string line = "";
	line = GetData(url);
	return line;
}

