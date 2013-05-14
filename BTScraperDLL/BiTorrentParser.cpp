#include "BiTorrentparser.h"

BiTorrentParser::BiTorrentParser(BTScraperDlg * dialog)
{
	m_dialog = dialog;
}

BiTorrentParser::~BiTorrentParser(void)
{
}

BiTorrentParser::BiTorrentParser(btData *data, BTScraperDlg * dialog) {
	bd = data;
	m_dialog = dialog;
	string types[] = { "", "time", "torrent", "size",  "seeds", "dl"}; 
	for(int i = 0; i < 6; i++) {
		m_types.push_back(types[i]);
	}
}

btData * BiTorrentParser::Parse(string s) {
	if(s.find("request failed") == 0) {
		TRACE("FAILED TO CONNECT: %s\n", GetURL());
		return bd;
	}
	
	int count = 0;

	int start = (int)s.find("class=\"torrents");
	if(start < 0) return bd;

	string lineBreak = "/tr>";
	vector<string> tokens = Split(s.substr(start, s.length()), lineBreak);  // break it up by lines

	int size = (int)tokens.size();  
	string search_type = "OTHER";

	for(int i = 0; i < size; i++) // goes through all tokens to find the ones with the data 
	{
		string type = "OTHER";
		string token = tokens[i];


		vector<string> data = Split(token, "<td"); // // break it up by params

		if(data.size() == 1) { // find search_type
			string value = token;
			int start = (int)value.find("a>")+3; // extra for the "
			int len = (int)value.find("<", start) - start;

			value = value.substr(start, len);
			if(value.find("Music") == 0) search_type = "AUDIO";
			else if(value.find("Anime") == 0 || value.find("Movies") == 0 || value.find("TV shows") == 0) 
				search_type = "VIDEO";
			else if(value.find("Application") == 0 || value.find("Games") == 0 || value.find("Mac Stuff") == 0) 
				search_type = "SOFTWARE";
			else search_type = "OTHER";
			i++;
		}
		else if(data.size() >= m_types.size() ) {
			count++;
			btData * newData = CreateNode(data, m_types);
			newData->SetType(search_type);
			if(bd != NULL) {
				bd->AddNode(newData);
			} else {
				bd = newData;
			} 		
		}
	} // for .. tr

	char temp[16];
	itoa(count, temp, 10);
	char message[32];
	strcpy(message, "total count: ");
	strcat(message, temp);
	Log(message);

	return bd;
}

btData * BiTorrentParser::CreateNode(vector<string> data, vector<string> types) {
	string source = "Bi-torrent";
	string title = "";
	string size = "";
	string url = "";
	string seed = "";
	string dl = "";
	int num_types = (int)types.size();

	for(int i = 0; i < num_types; i++) { // find the actual values we need

		string type = types[i];
		string value = data[i];
		string startString = ">";
		string endString = "</";

		if(type == "torrent") { // title, href
			startString = "title=\"";
			endString = "\">";
		}

		int start = (int)value.find(startString)+(int)startString.length();
		int length = (int)value.find(endString, start) - start;

		if (length > 0) value = value.substr(start, length);

		if(type == "torrent") { // title, href
			title = value;
			value = data[i];
			start = (int)value.find("href=\"")+6;
			length = (int)value.find("\"", start) - start;
			url = value.substr(start, length);
		} else if(type == "size") {
			size = value;
		} else if(type == "seeds") {
			start = (int)value.find(">");
			if(start > 0)
				value = value.substr(start+1);
			seed = value;	
		} else if(type == "dl") {
			dl = value;	
		}
	}
	btData * newData = new btData(title, size, url, "", seed, dl, source);
	return newData;
}

string BiTorrentParser::Read(char *url) {
	string line = "";
	line = GetData(url);
	return line;
}
	
char * BiTorrentParser::GetURL() {
	return "http://www.bi-torrent.com/index.htm";
}
