#pragma once

class MovieInfo
{
public:
	MovieInfo(void);
	~MovieInfo(void);

	string m_filename;
	string m_artist;
	string m_category;
	string m_keywords;
	string m_description;
	int m_length;

};
