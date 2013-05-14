#pragma once

class MusicInfo
{
public:
	MusicInfo(void);
	~MusicInfo(void);

	int m_single_offset;

	string m_genre;
	string m_year;
	string m_track;
	string m_filename;
	string m_artist;
	string m_album;
	string m_title;
	string m_comments;
};
