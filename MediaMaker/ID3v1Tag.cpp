// ID3v1Tag.cpp

#include "stdafx.h"
#include "ID3v1Tag.h"

//
//
//
ID3v1Tag::ID3v1Tag()
{
	Clear();
	Init();
}

//
//
//
void ID3v1Tag::Clear()
{
//	memset(this,0,sizeof(ID3v1Tag));
	memset(m_tag,0,sizeof(m_tag));		// "TAG"
	memset(m_title,0,sizeof(m_title));
	memset(m_artist,0,sizeof(m_artist));
	memset(m_album,0,sizeof(m_album));
	memset(m_year,0,sizeof(m_year));
	memset(m_comment,0,sizeof(m_comment));
	memset(m_genre,0,sizeof(m_genre));
}

//
//
//
void ID3v1Tag::Init()
{
	memcpy(m_tag,"TAG",3);
}
/*
//
// 1 - read in tag, 0 - there is no tag in the file, -1 - error opening file
//
int ID3v1Tag::ExtractTag(char *filename)
{
	CFile file;
	if(file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return -1;
	}

	file.Seek(0-sizeof(ID3v1Tag),CFile::end);

	file.Read(this,sizeof(ID3v1Tag));

	file.Close();

	if(IsValid())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
*/
/*
//
//
//
bool ID3v1Tag::IsValid()
{
	if(memcmp(m_tag,"TAG",sizeof(m_tag))==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
*/
//
//
//
string ID3v1Tag::ReturnTitle()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_title,sizeof(m_title));

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnArtist()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_artist,sizeof(m_artist));

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnAlbum()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_album,sizeof(m_album));

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnYear()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_year,sizeof(m_year));

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnComment()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_comment,sizeof(m_comment));

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnGenre()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	_itoa(m_genre[0],buf,10);

	CString cstring=buf;
	cstring.TrimLeft();
	cstring.TrimRight();

	string ret=cstring;
	return ret;
}

//
//
//
void ID3v1Tag::SetTitle(char *title)
{
	memset(m_title,0,sizeof(m_title));	// clear title

	unsigned int len=(unsigned int)strlen(title);
	if(len>sizeof(m_title))
	{
		len=sizeof(m_title);
		memcpy(m_title,title,len);
		m_title[len] = '\0';
		return;
	}

	memcpy(m_title,title,len);	
}

//
//
//
void ID3v1Tag::SetArtist(char *artist)
{
	memset(m_artist,0,sizeof(m_artist));	// clear artist

	unsigned int len=(unsigned int)strlen(artist);
	if(len>sizeof(m_artist))
	{
		len=sizeof(m_artist);
		memcpy(m_artist,artist,len);
		m_artist[len] = '\0';
		return;
	}

	memcpy(m_artist,artist,len);
	
}

//
//
//
void ID3v1Tag::SetAlbum(char *album)
{
	memset(m_album,0,sizeof(m_album));	// clear album

	unsigned int len=(unsigned int)strlen(album);
	if(len>sizeof(m_album))
	{
		len=sizeof(m_album);
		memcpy(m_album,album,len);
		m_album[len] = '\0';
		return;
	}
	
	memcpy(m_album,album,len);
}

//
//
//
void ID3v1Tag::SetYear(char *year)
{
	memset(m_year,0,sizeof(m_year));	// clear year

	unsigned int len=(unsigned int)strlen(year);
	if(len>sizeof(m_year))
	{
		len=sizeof(m_year);
		memcpy(m_year,year,len);
		m_year[len] = '\0';
		return;
	}

	memcpy(m_year,year,len);
}

//
//
//
void ID3v1Tag::SetComment(char *comment)
{
	memset(m_comment,0,sizeof(m_comment));	// clear comment

	unsigned int len=(unsigned int)strlen(comment);
	if(len>sizeof(m_comment))
	{
		len=sizeof(m_comment);
		memcpy(m_comment,comment,len);
		m_comment[len] = '\0';
		return;
	}

	memcpy(m_comment,comment,len);
}

//
//
//
void ID3v1Tag::SetGenre(unsigned char genre)
{
	m_genre[0]=genre;
}
