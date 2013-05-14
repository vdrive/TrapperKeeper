// ID3v1Tag.cpp

#include "stdafx.h"
#include "ID3v1Tag.h"

#define MAXGENRE   147
char *GENRE_TYPE [MAXGENRE+2] = {
   "Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge",
   "Hip-Hop","Jazz","Metal","New Age","Oldies","Other","Pop","R&B",
   "Rap","Reggae","Rock","Techno","Industrial","Alternative","Ska",
   "Death Metal","Pranks","Soundtrack","Euro-Techno","Ambient",
   "Trip-Hop","Vocal","Jazz+Funk","Fusion","Trance","Classical",
   "Instrumental","Acid","House","Game","Sound Clip","Gospel","Noise",
   "Alt. Rock","Bass","Soul","Punk","Space","Meditative",
   "Instrumental Pop","Instrumental Rock","Ethnic","Gothic",
   "Darkwave","Techno-Industrial","Electronic","Pop-Folk","Eurodance",
   "Dream","Southern Rock","Comedy","Cult","Gangsta Rap","Top 40",
   "Christian Rap","Pop/Funk","Jungle","Native American","Cabaret",
   "New Wave","Psychedelic","Rave","Showtunes","Trailer","Lo-Fi",
   "Tribal","Acid Punk","Acid Jazz","Polka","Retro","Musical",
   "Rock & Roll","Hard Rock","Folk","Folk/Rock","National Folk",
   "Swing","Fast-Fusion","Bebob","Latin","Revival","Celtic",
   "Bluegrass","Avantgarde","Gothic Rock","Progressive Rock",
   "Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band",
   "Chorus","Easy Listening","Acoustic","Humour","Speech","Chanson",
   "Opera","Chamber Music","Sonata","Symphony","Booty Bass","Primus",
   "Porn Groove","Satire","Slow Jam","Club","Tango","Samba",
   "Folklore","Ballad","Power Ballad","Rhythmic Soul","Freestyle",
   "Duet","Punk Rock","Drum Solo","A Cappella","Euro-House",
   "Dance Hall","Goa","Drum & Bass","Club-House","Hardcore","Terror",
   "Indie","BritPop","Negerpunk","Polsk Punk","Beat",
   "Christian Gangsta Rap","Heavy Metal","Black Metal","Crossover",
   "Contemporary Christian","Christian Rock","Merengue","Salsa",
   "Thrash Metal","Anime","JPop","Synthpop",""
};

//
//
//
ID3v1Tag::ID3v1Tag()
{
	Clear();
}

//
//
//
void ID3v1Tag::Clear()
{
	memset(this,0,128);
}

//
// 1 - read in tag, 0 - there is no tag in the file, -1 - error opening file
//
int ID3v1Tag::ExtractTag(const char *filename)
{
	CFile file;
	if(file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return -1;
	}
	if(file.GetLength() < 128)
	{
		file.Close();
		return 0;
	}

	file.Seek(0-128,CFile::end);

	file.Read(this,128);

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

//
//
//
string ID3v1Tag::ReturnTitle()
{
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	memcpy(buf,m_title,sizeof(m_title));

	string ret=buf;
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

	string ret=buf;
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

	string ret=buf;
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

	string ret=buf;
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

	string ret=buf;
	return ret;
}

//
//
//
string ID3v1Tag::ReturnGenre()
{
	string ret;
	char buf[1024+1];
	memset(buf,0,sizeof(buf));

	//_itoa(m_genre[0],buf,10);

	//string ret=buf;
	if(m_genre[0] <= MAXGENRE)
		string ret=GENRE_TYPE[m_genre[0]];
	return ret;
}