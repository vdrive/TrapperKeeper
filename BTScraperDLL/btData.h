#pragma once

//#include "stdafx.h"
//#include <iostream>
#include <string>

using namespace std ;

class btData
{
public:
	btData(void);
	btData(string title, string size, string url, string type, 
			   string seed, string dl, string source);
	btData(string title, string source);
	~btData(void);
	void Clear(void);

    bool NeedPostData();
	void NeedPostData(bool post);
	bool Equals(btData *, btData*);
	bool Equals(btData*);

	string ToString();
	string GetTitle();
	const char * GetTitleAsChar(int len);
	unsigned int GetSize();
	string GetSizeStr();
	string GetURL();
	string GetPostURL();
	string GetRefererURL();
	int GetSeeds();
	int GetDownloads();
	int GetFiles();
	string GetSource();
	string GetType();
	string GetTracker();
	string GetHash();
	btData * GetNext();
	string GetProjectID();
	

	void SetTitle(string );
	void SetSize(string size);
	void SetSeeds(string );
	void SetDownloads(string );
	void SetFiles(string files);
	void SetURL(string );
	void SetPostURL(string );
	void SetRefererURL(string );
	void SetType(string );
	void SetTracker(string );
	void SetHash(string hash);
	void SetProjectID(string id);

	bool AddNode(btData *);

	
private:
	// title, size, url, type, seed, download, source
	void Init(string title, string size, string url, string type, 
				  string seed, string dl, string source);
	string m_title;
	unsigned int m_size; 
	string m_url;
	string m_post_url;
	string m_referer_url;
	string m_seed;
	string m_dl;
	string m_files;
	string m_source;
	string m_type;
	string m_tracker;
	string m_hash;
	string m_project_id;
	string m_size_str;
	bool b_post;
	btData * next;


};
