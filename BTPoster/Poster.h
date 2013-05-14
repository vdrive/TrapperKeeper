#pragma once

#include "HTTPConnector.h"


class Poster
{
public:
	Poster(void);
	~Poster(void);
	virtual bool Post(string filename, string name, char *torrent, int torrent_len);
	string GetBoundry();
	string GetHeader(char * torrent, string boundry);
	int GetPostData(char * post_data, char * torrent);
	string GetFormatedPostData(string border, string name, string value);
	virtual string GetBeginData(string filename, string name, string boundry);
	virtual string GetEndData(string filename, string name, string boundry);
	virtual string GetPostURL();
	virtual string GetRefererURL();

	string GetMainCategory(); 
	string GetSubCategory(); 
	string m_main_category;
	string m_sub_category;

	bool CreateFile(string filename);
	void OpenFile(string filename, CFile *t_file);
	void CloseFile(CFile *file);
	void WriteTorrentFile(string filename, char * data, int len);
	void WriteFile(string filename, char * data, int len);
};
