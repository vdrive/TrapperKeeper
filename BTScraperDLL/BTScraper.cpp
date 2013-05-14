//#include "afxinet.h"
#include "BTScraper.h"
#include "torrentfile.h"
#include "HTTPConnection.h"



BTScraper::BTScraper(void)
{
	b_search = false;
	m_main_list_ctrl = NULL;
	bd = NULL;
}

BTScraper::BTScraper(btData * data, CListCtrl *list_ctrl)
{
	m_main_list_ctrl = list_ctrl;
	bd = data;
}

BTScraper::~BTScraper(void)
{
}

void BTScraper::Init(CListCtrl *list_ctrl)
{
	m_main_list_ctrl = list_ctrl;
}

void BTScraper::Clear(void) 
{
	if(bd == NULL) return;
	bd->Clear();
	delete bd;
	bd = NULL;
}

/**************
	return true if we need to do a post to 
	get the torrent file instaed of a get
**************/
bool BTScraper::NeedPostData()
{
	return false;
}

/**
		Goes to the web pages, reads the data, then 
		returns a link list with the releveant 
		information
**/
btData * BTScraper::GetList() 
{
	string s = Read();
	return Parse(s);
}

/**		
		Goes to specified url and gets the body and 
		returns it as a string
**/
string BTScraper::GetData(const char * url, const string referer) 
{
	return ::GetData(url, referer);
}

string BTScraper::Read() 
{
	return Read(GetURL());
}


string BTScraper::GetType(string search_type) {
	if(search_type.find("Music") == 0) search_type = "AUDIO";
	else if(search_type.find("Anime") == 0 || search_type.find("DVD-R") == 0 || search_type.find("Film") == 0 || search_type.find("Movies") != -1 || 
		search_type.find("Television") == 0 || search_type.find("Video") == 0 || search_type.find("TV") == 0) search_type = "VIDEO";
	else if(search_type.find("Apps") == 0 || search_type.find("Software") == 0 || search_type.find("Application") == 0 || 
		search_type.find("Mac") == 0 || search_type.find("Games") == 0 || search_type.find("Linux") == 0) search_type = "SOFTWARE";
	else if(search_type.find("XXX") == 0) search_type = "PORN";
	else if(search_type.find("Unsorted") == 0 || search_type.find("Other") == 0 || search_type.find("Picture") == 0) search_type = "OTHER";
	else if(search_type.find("Comics") == 0 || search_type.find("Books") == 0) search_type = "OTHER";
	else {
		//TRACE("setting %s to OTHER\n", search_type.c_str());
		search_type = "OTHER";
	}
	return search_type;
}

// returns the substring between start and end and sets start_pos 
// to the end of that 
string BTScraper::GetValue(const string data, int * pos, string startStr, string endStr) 
{
	int cur_pos = *pos;
	const char * temp = data.c_str();
	int start = -1;
	if(startStr.length() == 0) start = cur_pos;
	else start = (int)data.find(startStr, cur_pos);
	if(start < 0) {
		return "";
	}
	start += (int)startStr.length();
	int len = (int)data.find(endStr, start);
	if(len < 0 || endStr.length() == 0) len = (int)data.length() - start;
	else len -= start;
	cur_pos = start + len;

	*pos = cur_pos;
	//TRACE("%s\n", data.substr(start, len).c_str());
	return data.substr(start, len);
}

void BTScraper::Log( const char *buf) 
{
	if(m_main_list_ctrl == NULL) return;
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

void BTScraper::Log(string message) 
{
	Log(message.c_str());
}

