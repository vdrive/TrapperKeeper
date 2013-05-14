#include "stdafx.h"
#include "SupplyEntry.h"


//
//
//
SupplyEntry::SupplyEntry()
{
	p_filename=NULL;
	p_nickname=NULL;
	p_md5=NULL;

	Clear();
}

//
//
//
SupplyEntry::~SupplyEntry()
{
	if(p_filename!=NULL)
	{
		delete [] p_filename;
		p_filename=NULL;
	}
	if(p_nickname!=NULL)
	{
		delete [] p_nickname;
		p_nickname=NULL;
	}
	if(p_md5!=NULL)
	{
		delete [] p_md5;
		p_md5=NULL;
	}
}

//
//
//
SupplyEntry::SupplyEntry(const SupplyEntry &entry)
{
	p_filename=NULL;
	p_nickname=NULL;
	p_md5=NULL;
	if(entry.p_filename != NULL)
	{
		p_filename=new char[strlen(entry.p_filename)+1];
		strcpy(p_filename,entry.p_filename);
	}
	if(entry.p_nickname != NULL)
	{
		p_nickname=new char[strlen(entry.p_nickname)+1];
		strcpy(p_nickname,entry.p_nickname);
	}
	if(entry.p_md5 != NULL)
	{
		p_md5=new char[strlen(entry.p_md5)+1];
		strcpy(p_md5,entry.p_md5);
	}
	m_file_length = entry.m_file_length;
	m_bit_rate = entry.m_bit_rate;
	m_sampling_rate = entry.m_sampling_rate;
	m_stereo = entry.m_stereo;
	m_song_length = entry.m_song_length;
	m_track = entry.m_track;
	m_popularity = entry.m_popularity;
	
}

//
//
//
SupplyEntry& SupplyEntry::operator=(const SupplyEntry &entry)
{
	if(p_filename!=NULL)
	{
		delete [] p_filename;
		p_filename=NULL;
	}
	if(entry.p_filename != NULL)
	{
		p_filename=new char[strlen(entry.p_filename)+1];
		strcpy(p_filename,entry.p_filename);
	}

	if(p_nickname!=NULL)
	{
		delete [] p_nickname;
		p_nickname=NULL;
	}
	if(entry.p_nickname != NULL)
	{
		p_nickname=new char[strlen(entry.p_nickname)+1];
		strcpy(p_nickname,entry.p_nickname);
	}

	if(p_md5!=NULL)
	{
		delete [] p_md5;
		p_md5=NULL;
	}
	if(entry.p_md5 != NULL)
	{
		p_md5=new char[strlen(entry.p_md5)+1];
		strcpy(p_md5,entry.p_md5);
	}


	m_file_length = entry.m_file_length;
	m_bit_rate = entry.m_bit_rate;
	m_sampling_rate = entry.m_sampling_rate;
	m_stereo = entry.m_stereo;
	m_song_length = entry.m_song_length;
	m_track = entry.m_track;
	m_popularity = entry.m_popularity;

	return *this;
}

//
//
//
void SupplyEntry::Clear()
{
	if(p_filename!=NULL)
	{
		delete [] p_filename;
		p_filename=NULL;
	}
	if(p_nickname!=NULL)
	{
		delete [] p_nickname;
		p_nickname=NULL;
	}
	if(p_md5!=NULL)
	{
		delete [] p_md5;
		p_md5=NULL;
	}
	m_file_length = 0;
	m_bit_rate = 0;
	m_sampling_rate = 0;
	m_stereo = 0;
	m_song_length = 0;
	m_track = 0;
	m_popularity = 0;

}
	
//
//
//
int SupplyEntry::GetBufferLength()
{
	int count = 0;

	if(p_filename != NULL)
		count += (int)strlen(Filename())+1;
	if(p_nickname != NULL)
		count += (int)strlen(Nickname())+1;
	if(p_md5 != NULL)
		count += (int)strlen(MD5())+1;

	count += sizeof(m_file_length);
	count += sizeof(m_bit_rate);
	count += sizeof(m_sampling_rate);
	count += sizeof(m_stereo);
	count += sizeof(m_song_length);
	count += sizeof(m_track);
	count += sizeof(m_popularity);

	return count;
}

	
///
//
//
int SupplyEntry::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	if(p_filename!=NULL)
	{
		strcpy(ptr,Filename());
		ptr+=strlen(Filename())+1;
	}
	if(p_nickname != NULL)
	{
		strcpy(ptr,Nickname());
		ptr+=strlen(Nickname())+1;
	}
	if(p_md5 != NULL)
	{
		strcpy(ptr,MD5());
		ptr+=strlen(MD5())+1;
	}

	*((unsigned int *)ptr)=m_file_length;
	ptr+=sizeof(m_file_length);

	*((int *)ptr)=m_bit_rate;
	ptr+=sizeof(m_bit_rate);

	*((int *)ptr)=m_sampling_rate;
	ptr+=sizeof(int);

	*((int *)ptr)=m_stereo;
	ptr+=sizeof(int);

	*((int *)ptr)=m_song_length;
	ptr+=sizeof(int);

	*((int *)ptr)=m_track;
	ptr+=sizeof(int);

	*((int *)ptr)=m_popularity;
	ptr+=sizeof(int);

	return GetBufferLength();
}

	
//
//
//
int SupplyEntry::ReadFromBuffer(char *buf)
{
	int num_entries = 0;

	Clear();

	char *ptr=buf;

	
	Filename(ptr);
	if(Filename()!=NULL)
		ptr+=strlen(Filename())+1;

	Nickname(ptr);
	if(Nickname()!=NULL)
		ptr+=strlen(Nickname())+1;

	MD5(ptr);
	if(MD5()!=NULL)
		ptr+=strlen(MD5())+1;

	m_file_length=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_bit_rate=*((int *)ptr);
	ptr+=sizeof(int);

	m_sampling_rate=*((int *)ptr);
	ptr+=sizeof(int);

	m_stereo=*((int *)ptr);
	ptr+=sizeof(int);

	m_song_length=*((int *)ptr);
	ptr+=sizeof(int);

	m_track=*((int *)ptr);
	ptr+=sizeof(int);

	m_popularity=*((int *)ptr);
	ptr+=sizeof(int);

	return GetBufferLength();
}

//
//
//
void SupplyEntry::Filename(char *filename)
{
	if(p_filename!=NULL)
	{
		delete [] p_filename;
		p_filename=NULL;
	}

	p_filename=new char[strlen(filename)+1];
	strcpy(p_filename,filename);
}

//
//
//
char *SupplyEntry::Filename()
{
	return p_filename;
}

//
//
//
void SupplyEntry::Nickname(char* nickname)
{
	if(p_nickname!=NULL)
	{
		delete [] p_nickname;
		p_nickname=NULL;
	}

	p_nickname=new char[strlen(nickname)+1];
	strcpy(p_nickname,nickname);
}

//
//
//
char *SupplyEntry::Nickname()
{
	return p_nickname;
}

//
//
//
void SupplyEntry::MD5(char* md5)
{
	if(p_md5!=NULL)
	{
		delete [] p_md5;
		p_md5=NULL;
	}

	p_md5=new char[strlen(md5)+1];
	strcpy(p_md5,md5);
}

//
//
//
char *SupplyEntry::MD5()
{
	return p_md5;
}