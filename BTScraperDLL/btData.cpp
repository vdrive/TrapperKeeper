#include "btdata.h"
#include "stdafx.h"

btData::btData(void)
{
	Init(NULL, "", "", "", "", "", "");
	
}

btData::btData(string title, string size, string url, string type, 
			   string seed, string dl, string source) 
{
	Init(title, size, url, type, seed, dl, source);
}


btData::btData(string title, string source) {
	Init(title, "", "", "", "", "", source);
}

btData::~btData(void)
{
	//if(next != NULL) 
	//	delete next;
}



void btData::Clear(void) 
{
	btData * temp = next;
	while(next != NULL) {
		temp = next->next;
		delete next;
		next = temp;
	}
}

bool btData::NeedPostData() 
{
	return b_post;
}

void btData::NeedPostData(bool post) 
{
	b_post = post;
}


bool btData::AddNode(btData *bt) 
{
	if(bt == NULL) return false;

	if(this->next == NULL) 
	{
		next = bt;
		return true;
	}
	else 
	{
		return next->AddNode(bt);
	}
	return false;
}

string btData::GetTitle() 
{	
	return m_title;
}

string btData::GetSizeStr() 
{	
	return m_size_str;
}

unsigned int btData::GetSize() 
{	
	return m_size;
}

int btData::GetSeeds() 
{	
	int value = atoi(m_seed.c_str()); 
	if(value == 0 && m_seed.compare("0") != 0) value = -1;
	return value;
}

int btData::GetDownloads() 
{	
	int value = atoi(m_dl.c_str()); 
	if(value == 0 && m_dl.compare("0") != 0) value = -1;
	return value;
}

int btData::GetFiles() 
{	
	int value = atoi(m_files.c_str()); 
	if(value == 0 && m_files.compare("0") != 0) value = -1;
	return value;
}

string btData::GetType() 
{	
	return m_type;
}

btData * btData::GetNext() 
{
	return next;
}


string btData::GetURL() 
{
	return m_url;
}

string btData::GetPostURL() 
{
	return m_post_url;
}

string btData::GetRefererURL() 
{
	return m_referer_url;
}

string btData::GetSource() 
{
	return m_source;
}

string btData::GetTracker() 
{
	return m_tracker;
}

string btData::GetHash() 
{
	return m_hash;
}

string btData::GetProjectID()
{
	return m_project_id;
}

void btData::SetTitle(string title) 
{
	int index = (int)title.find('\'');
	while(index > 0) {
		title[index] = 	'_';
		index = (int)title.find('\'');
	}
	m_title = title;
	int len = (int)m_title.length();
	char * temp = new char[len+1];
	strcpy(temp, m_title.c_str());
	while(temp[len-1] == ' ') {
		temp[len-1] = '\0';
		len--;
	}
	m_title = string(temp);
	delete temp;
}

void btData::SetSize(string s)
{
	m_size_str = s;
	unsigned int k = 1024;
	unsigned int m = 1024*1024;
	unsigned int g = 1024*1024*1024;
	char num[16];
	char *buf = num;
	strcpy(num, "");
	unsigned int size = 0;
	const char * value = s.c_str();
	bool b_found = false;
	char type = '\0';
	//TRACE("value: %s = ", value);
	for(int i = 0; i < (int)s.length(); i++ ) {
		type = value[i];
		if(isdigit(type) || type == '.') {
			memcpy(buf, &value[i], 1);
			buf++;
			memcpy(buf, "\0", 1);
		}
		if(type == 'M') {
			b_found = true;
			size = (unsigned int)(atof(num) * m);
			break;
		} 
		else if (type == 'G') {
			b_found = true;
			if(atof(num) > 4.29) size = (unsigned int)(4.29 * g);
			else size = (unsigned int)(atof(num) * g);
			break;
		}
		else if (type == 'K') {
			b_found = true;
			size = (unsigned int)(atof(num) * k);
			break;
		}
		else if (type == 'B') {
			b_found = true;
			size = (unsigned int)atof(num);
			break;
		}
	}
	if(s.length() == 0) {
		b_found = true;
	}
	if(!b_found) {
		TRACE("no size, %s\n", s.c_str());
	}
	//TRACE("%s %c, %u\n", num, type, size);
	sprintf(num, "%s %c", num, type);
	m_size_str = string(num);
	m_size = size;
}

void btData::SetSeeds(string s)
{
	m_seed = s;
}

void btData::SetFiles(string files)
{
	m_files = files;
}

void btData::SetType(string t) 
{
if(t.compare("AUDIO") != 0 && t.compare("VIDEO") != 0 && t.compare("SOFTWARE") != 0 && t.compare("EVERYTHING") != 0) {
		m_type = "OTHER";
	}
	else m_type = t;
}

void btData::SetTracker(string tracker) 
{
	m_tracker = tracker;
}

void btData::SetDownloads(string d)
{
	m_dl = d;
}

void btData::SetURL(string url) 
{
	m_url=url;
}

void btData::SetPostURL(string url) 
{
	m_post_url=url;
}

void btData::SetRefererURL(string r_url) 
{
	m_referer_url = r_url;
}

void btData::SetHash(string hash) 
{
	m_hash = hash;
}

void btData::SetProjectID(string id)
{
	m_project_id = id;
}

string btData::ToString() 
{
	char buf[32];
	string retStr = "btData::[title:";
	if(m_title == "") 
		m_title = "NO TITLE";
	if(m_title.length() > 0) retStr += m_title;
	sprintf(buf, "%s(%u)", m_size_str.c_str(), m_size);
	retStr +=  ",size:" + string(buf);
	if(m_url.length() > 0) retStr += ",url:" + m_url;
	if(m_tracker.length() > 0) retStr += ",tracker:" + m_tracker;
	if(m_type.length() > 0) retStr += ",type:" + m_type;
	if(m_seed.length() > 0) retStr += ",seed:" + m_seed;
	if(m_dl.length() > 0) retStr += ",dl:" + m_dl;
	if(m_source.length() > 0) retStr += ",source:" + m_source;
	retStr += "]";

	//if(next != NULL) 
	//	return sb->Append(next->ToString())->ToString();
	//else 
		return retStr;
}

bool btData::Equals(btData *right)  
{
	return Equals(this, right);
}

bool btData::Equals(btData *left, btData *right) 
{	
	string leftStr = left->GetTitle();
	string rightStr = right->GetTitle();

	if(leftStr.length() != rightStr.length())
		return false;
	if(leftStr.compare(rightStr) == 0) return true;
	else return false;
}

void btData::Init(string title, string size, string url, string type, 
				  string seed, string dl, string source)
{
	b_post = false;
	m_source = source;
	SetTitle(title);
	SetSize(size);
	SetURL(url);
	SetSeeds(seed);
	SetDownloads(dl);
	SetType(type);
	m_tracker = "";
	next = NULL;
	m_referer_url = "";
	m_hash = "";
	m_project_id = "0";
	m_files = "0";
}

