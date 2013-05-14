#pragma once
#include "..\tkcom\object.h"

class AresSearchResult : public Object
{
public:
	AresSearchResult(byte *hash,const char* artist, const char* title, const char* year, const char* genre,const char* album, const char* comments,const char* file_name,UINT host,UINT file_size,UINT bitrate,UINT media_length,byte media_type, const char* ip,unsigned short port, const char* user_name);
	~AresSearchResult(void);

	int m_count;
	byte m_hash[20];
	string m_artist;
	string m_title;
	string m_album;
	string m_comments;
	string m_genre;
	string m_file_name;
	string m_year;
	string m_user_name;
	string m_ip;
	unsigned short m_port;
	UINT m_file_size;
	UINT m_bitrate;
	UINT m_media_length;
	UINT m_host;
	byte m_media_type;
	void IncrementCount(void);
};
