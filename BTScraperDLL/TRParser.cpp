#include "trparser.h"


TRParser::TRParser(btData *data, CListCtrl *list_ctrl)
{
	b_search = false;
	m_main_list_ctrl = list_ctrl;
	bd = data;
}

TRParser::~TRParser(void)
{
}

string TRParser::GetType(string value) 
{
	string search_type = "OTHER";
	if(value.find("Music") == 0 || value.find("Rock") == 0) search_type = "AUDIO";
	else if(value.find("Anime") == 0 || value.find("Series") == 0 || value.find("Movies") == 0) 
		search_type = "VIDEO";
	else if(value.find("Appz") == 0 || value.find("Games") == 0) search_type = "SOFTWARE";
	else if(value.find("Other") == 0) search_type = "OTHER";
	else if(value.find("XXX/Porn") == 0 || value.find("Hentai") == 0) search_type = "PORN";
	else {
		TRACE("%s\n", value.c_str());
		search_type = "OTHER";
	}

	return search_type;
}

btData * TRParser::Parse(string data)
{
	CTime now_time = (CTime::GetCurrentTime());
	char year[5];
	sprintf(year, "%d", now_time.GetYear());

	int count = 0;
	int start = 0;
	int end =  0;
	if(!b_search) {
		start = (int)data.find(year);
		end = (int)data.rfind("What is torrentreactor.to Anyway?");
	}
	else {
		start = (int)data.rfind("Found");
		end = (int)data.rfind("/table");
	}
	const char * temp = data.c_str();
	if(start < 0) {
		if(data.find("Sorry, no Torrents found") != -1 || data.find("More than 30 Torrents found") != -1) return bd;
		Log("Parser ERROR");
		return bd;
	}
	string main_type = "OTHER";
	int next_type = 0;
	int cur_pos = start;

	cur_pos = (int)data.find("<TR", cur_pos);
	next_type = (int)data.find("\"big\">", cur_pos);
	while(cur_pos < end && cur_pos > 0) 
	{
		btData * newNode = CreateNode(data, &cur_pos);
		if(cur_pos > next_type && !b_search) {
			main_type = GetValue(data.c_str(), &next_type, "big\">", "<");
			//TRACE("maintype(%d): '%s'\n", main_type.length(), main_type.c_str());
			if(main_type.length() == 1) main_type = GetValue(data.c_str(), &next_type, "big\">", "<");
			//TRACE("maintype: '%s'\n", main_type.c_str());
			if(main_type.find(year) != -1) main_type = GetValue(data.c_str(), &next_type, "big\">", "<");
			//TRACE("maintype: '%s'\n", main_type.c_str());
			next_type = (int)data.find("\"big\">", cur_pos);
			main_type = GetType(main_type);
			//TRACE("TYPE: %s\n", main_type.c_str());
			cur_pos = (int)data.find("<TR", cur_pos);
			newNode->SetType(main_type);
		}
		
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

btData * TRParser::CreateNode(string data, int * pos) {
	string source = "Torrentreactor";
	string title = "";
	string size = "";
	string url = "";
	string seed = "";
	string dl = "";
	string type = "OTHER";
	int cur_pos = *pos;
	const char * temp = data.c_str();
	GetValue(data, &cur_pos, "R\">", "<"); // age
	url = GetValue(data, &cur_pos, "HREF=\"", "\"");
	cur_pos = (int)data.find("<A", cur_pos);
	title = GetValue(data, &cur_pos, "title=\"", "\"");
	size = GetValue(data, &cur_pos, "R\">", "<");
	seed = GetValue(data, &cur_pos, "R\">", "<");
	dl = GetValue(data, &cur_pos, "R\">", "<");
	if (b_search) {
		cur_pos = (int)data.find("section", cur_pos);
		type = GetType(GetValue(data, &cur_pos, ">", "<"));
	}
	btData * newData = new btData(title, size, url, "", seed, dl, source);
	cur_pos = (int)data.find("<TR", cur_pos);
	*pos = cur_pos;
	return newData;
}

char * TRParser::GetURL() 
{
	
	return "http://www.torrentreactor.to/index.php";
}
    

string TRParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
