#include "TorrentSpyParser.h"



TorrentSpyParser::~TorrentSpyParser(void)
{
}

TorrentSpyParser::TorrentSpyParser(btData *data, CListCtrl * list_ctrl) {
	m_main_list_ctrl = list_ctrl;
	bd = data;
}


btData * TorrentSpyParser::GetList() 
{
	string data = "";
	btData * list = NULL;
	char url[64];

	char * baseURL = "http://www.torrentspy.com/latest.asp?mode=&pg=";
	data = GetData(GetURL());
	list = Parse(data);
	int num_pages = GetNumPages(data);
	for(int i = 1; i < num_pages; i++) {
		sprintf(url, "%s%d", baseURL, i);
		TRACE("url: %s\n", url);
		data = GetData(url, "http://www.torrentspy.com/latest.asp");
		list = Parse(data);
	}
	return list;

}

btData * TorrentSpyParser::Parse(string s) {
	if(s.find("request failed") == 0) {
		Log("FAILED TO CONNECT: ");
		return bd;
	}
	
	int count = 0;
	int pos = 0;
	int len = (int)s.length();
	const char * buf = s.c_str();
	//pos = (int)s.find("0 width=\"100%", pos);
	pos = (int)s.find("Health", pos);
	if(pos < 0) {
		if(s.find("Your search did not result in any matches") != -1) return bd;
		Log("Parse Error");
		//TRACE("Parser Error:\n");
		return bd;
	}

	
	while(pos < len && pos > 0) 
	{
		btData * newNode = CreateNode(s, &pos);
		//TRACE("title: %s\n", newNode->GetTitle().c_str());
		if(bd == NULL) bd = newNode;
		else bd->AddNode(newNode);
		count++;
	}

	char temp[16];
	itoa(count, temp, 10);
	char message[32];
	strcpy(message, "total count: ");
	strcat(message, temp);
	TRACE("%s\n", message);
	Log(message);

	return bd;
}

btData * TorrentSpyParser::CreateNode(string data, int * pos) {
	string source = "TorrentSpy";
	int cur_pos = *pos;
	const char * temp = data.c_str();
	string url = "http://www.torrentspy.com/download.asp?id=" + GetValue(data, &cur_pos, "download.asp?id=", "\"");
	cur_pos = (int)data.find(".asp", cur_pos);
	//TRACE("cur_pos: %d\n", cur_pos);
	string title = GetValue(data, &cur_pos, "title=", ">");
	if(title.compare("\"Download Torrent\"") == 0) for(int i=0;i < 5; i++) {
		if(title.compare("\"Download Torrent\"") == 0) title = GetValue(data, &cur_pos, "title=", ">");
	}
	//TRACE("Pre: >%s<\n", title.c_str());
	if(title.find("\"") == 0) {
		title = title.substr(1, title.length()-2);
	}
	//TRACE("post: >%s<\n", title.c_str());
	cur_pos = (int)data.find("dir", cur_pos);
	string search_type = GetValue(data, &cur_pos, "\">", "<");
	string size = GetValue(data, &cur_pos, "nowrap>", "<");
	string files = GetValue(data, &cur_pos, "nowrap>", "<"); // files
	string seed = GetValue(data, &cur_pos, "nowrap>", "<");
	string peers = GetValue(data, &cur_pos, "nowrap>", "<");


	if(search_type.find("Music") == 0) search_type = "AUDIO";
	else if(search_type.find("DVD-R") == 0 || search_type.find("Film") == 0 || search_type.find("Movies") != -1 || 
		search_type.find("TV") == 0 || search_type.find("Video") == 0 || search_type.find("Anime") == 0) search_type = "VIDEO";
	else if(search_type.find("Application") == 0 || search_type.find("Games") == 0) search_type = "SOFTWARE";
	else {
		//TRACE("setting %s to OTHER\n", search_type.c_str());
		search_type = "OTHER";
	}
	
	btData * newData = new btData(title, size, url, search_type, seed, peers, source);
	newData->SetFiles(files);
	//TRACE("torrentspy: %s\n", newData->ToString().c_str());

	cur_pos = (int)data.find("<tr ", cur_pos);
	*pos = cur_pos;
	return newData;
}


string TorrentSpyParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
	
char * TorrentSpyParser::GetURL() {

	return "http://www.torrentspy.com/latest.asp";
}

int TorrentSpyParser::GetNumPages(string data) 
{
	int pos = 0;
	string page_str = GetValue(data, &pos, "Pages (", ")");
	if(page_str.length() > 0) 
		return atoi(page_str.c_str());
	return 1;
	
}

