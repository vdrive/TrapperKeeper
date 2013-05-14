#include "StdAfx.h"
#include "aressearchresult.h"

AresSearchResult::AresSearchResult(byte *hash,const char* artist, const char* title, const char* year, const char* genre,const char* album, const char* comments,const char* file_name,UINT host,UINT file_size,UINT bitrate,UINT media_length,byte media_type, const char* ip,unsigned short port, const char* user_name)
{
	m_count=1;
	memcpy(m_hash,hash,20);
	m_artist=artist;
	m_title=title;
	m_year=year;
	m_genre=genre;
	m_file_size=file_size;
	m_file_name=file_name;
	m_comments=comments;
	m_bitrate=bitrate;
	m_media_length=media_length;
	m_media_type=media_type;
	m_host=host;
	m_ip=ip;
	m_port=port;
	m_user_name=user_name;


	TRACE("---- SEARCH RESULT ----\n");
	TRACE("   title = %s\n",m_title.c_str());
	TRACE("   file_name = %s\n",m_file_name.c_str());
	TRACE("   year = %s\n",m_year.c_str());
	TRACE("   artist = %s\n",m_artist.c_str());
	TRACE("   album = %s\n",m_album.c_str());
	TRACE("   genre = %s\n",m_genre.c_str());
	TRACE("   comments = %s\n",m_comments.c_str());
	TRACE("   size = %d\n",m_file_size);
	TRACE("   bitrate = %d\n",m_bitrate);
	TRACE("   media_length = %d\n",m_media_length);
	TRACE("   media_type = %d\n",m_media_type);
	TRACE("   host = %d\n",m_host);
	TRACE("==== END SEARCH RESULT ====\n");
}

AresSearchResult::~AresSearchResult(void)
{
}

void AresSearchResult::IncrementCount(void)
{
	m_count++;
}
