// ID3v2Tag.cpp

#include "stdafx.h"
#include "ID3v2Tag.h"
#include "ID3v2TagFrameHeader.h"

//
//
//
ID3v2Tag::ID3v2Tag()
{
	Clear();
}

//
//
//
void ID3v2Tag::Clear()
{
//	memset(this,0,sizeof(ID3v2Tag));

	m_hdr.Clear();

	m_artist.erase();
	m_album.erase();
	m_title.erase();
	m_track.erase();

	m_comment.erase();
	m_year.erase();
}
/*
//
//
//
bool ID3v2Tag::ExtractTag(char *filename)
{
	if(p_data!=NULL)
	{
		delete [] p_data;
		p_data=NULL;
	}
	
	CFile file;
	if(file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return false;
	}

	// Check to see that the beinning
	if(m_hdr.ExtractTagHeader(&file)!=1)
	{
		return false;
	}

	if(m_hdr.IsExtendedHeaderPresent())
	{
		MessageBox(NULL,"Extended Header Present","Not implemented yet",MB_OK);
		return false;
	}

	m_len=atoi(m_hdr.GetTagLength().c_str());

	p_data=new unsigned char[m_len];

	file.Read(p_data,m_len);

	file.Close();

	return ProcessTag();
}
*/
//
//
//
bool ID3v2Tag::ExtractTag(CFile *file)
{
/*
	if(p_data!=NULL)
	{
		delete [] p_data;
		p_data=NULL;
	}
*/
	// Check to see that the beinning
	if(m_hdr.ExtractTagHeader(file)!=1)
	{
		return false;
	}

	if(m_hdr.IsExtendedHeaderPresent())
	{
		//MessageBox(NULL,"Extended Header Present in ID3v2Tag::ExtractTag()","Not implemented yet",MB_OK);
		return false;
	}

	unsigned int len=atoi(m_hdr.GetTagLength().c_str());
	len-=sizeof(ID3v2TagHeader);

	unsigned char *data=new unsigned char[len];

	file->Read(data,len);

	bool ret=ProcessTag(data,len);

	delete [] data;

	return ret;
}

//
//
//
bool ID3v2Tag::ProcessTag(unsigned char *data,unsigned int len)
{
	char buf[1024];
	ID3v2TagFrameHeader frame_hdr;

	unsigned char *ptr=data;

	// Only look for specific things
	while((int)(ptr-data)<(int)len)
	{
		memset(buf,0,sizeof(buf));	// reset buf
		
		if(*ptr==0)
		{
			break;
		}
			
		frame_hdr.ExtractFrameHeaderPointer(ptr);
		ptr+=sizeof(ID3v2TagFrameHeader);

		if(memcmp(frame_hdr.ReturnFrameIDPointer(),"TPE1",4)==0)			// Lead performer(s)/Soloist(s)
		{
			// Check to see if it is ascii or unicode
			if(*ptr==0x00)
			{
				memcpy(buf,ptr+1,frame_hdr.ReturnFrameSize()-1);
			}
			else if(*ptr==0x01)
			{
				WideCharToMultiByte(CP_ACP,0,(LPCWSTR)(ptr+3),(frame_hdr.ReturnFrameSize()-3)/2,buf,sizeof(buf),NULL,NULL);
			}
			
			CString cstring=buf;
			cstring.TrimLeft();
			cstring.TrimRight();
			m_artist=cstring;
		}
		else if(memcmp(frame_hdr.ReturnFrameIDPointer(),"TALB",4)==0)	// Album/Movie/Show title
		{
			// Check to see if it is ascii or unicode
			if(*ptr==0x00)
			{
				memcpy(buf,ptr+1,frame_hdr.ReturnFrameSize()-1);
			}
			else if(*ptr==0x01)
			{
				WideCharToMultiByte(CP_ACP,0,(LPCWSTR)(ptr+3),(frame_hdr.ReturnFrameSize()-3)/2,buf,sizeof(buf),NULL,NULL);
			}
			
			CString cstring=buf;
			cstring.TrimLeft();
			cstring.TrimRight();
			m_album=cstring;
		}
		else if(memcmp(frame_hdr.ReturnFrameIDPointer(),"TIT2",4)==0)	// Title/songname/content description
		{
			// Check to see if it is ascii or unicode
			if(*ptr==0x00)
			{
				memcpy(buf,ptr+1,frame_hdr.ReturnFrameSize()-1);
			}
			else if(*ptr==0x01)
			{
				WideCharToMultiByte(CP_ACP,0,(LPCWSTR)(ptr+3),(frame_hdr.ReturnFrameSize()-3)/2,buf,sizeof(buf),NULL,NULL);
			}
			
			CString cstring=buf;
			cstring.TrimLeft();
			cstring.TrimRight();
			m_title=cstring;
		}
		else								// Unknown
		{
		}

		ptr+=frame_hdr.ReturnFrameSize();
	}

	return true;
}

//
//
//
unsigned int ID3v2Tag::GetTagLength()
{
	return atoi(m_hdr.GetTagLength().c_str());
}

//
//
//
int ID3v2Tag::WriteTagToFile(CFile *file)
{
	// Determine the buffer length and create the buffer
	unsigned int buf_len=0;
	
	buf_len+=sizeof(ID3v2TagHeader);

	if(m_artist.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=(int)m_artist.size();
	}
	if(m_album.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=(int)m_album.size();	
	}
	if(m_title.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=(int)m_title.size();
	}
	if(m_track.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=(int)m_track.size();
	}
	if(m_comment.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=3;	// language
		buf_len++;	// short descr (just a null)
		buf_len+=(int)m_comment.size();
	}
	if(m_year.size()!=0)
	{
		buf_len+=sizeof(ID3v2TagFrameHeader)+1;		// +1 for the byte to designate ASCII not UNICODE characters
		buf_len+=(int)m_year.size();
	}

	// Check to see if there is no info available to put into the header.  If so, then don't write it.
	if(buf_len==sizeof(ID3v2TagHeader))
	{
		return 0;
	}

	unsigned char *buf=new unsigned char[buf_len];

	m_hdr.Init();
	m_hdr.SetTagSize(buf_len-sizeof(ID3v2TagHeader));

	// Fill in the buffer
	unsigned char *ptr=&buf[0];
	memcpy(ptr,&m_hdr,sizeof(ID3v2TagHeader));
	ptr+=sizeof(ID3v2TagHeader);

	ID3v2TagFrameHeader frame_hdr;
	
	if(m_artist.size()!=0)
	{
		memcpy(&frame_hdr,"TPE1",4);
		frame_hdr.SetFrameSize((int)m_artist.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_artist.c_str(),m_artist.size());
		ptr+=m_artist.size();
	}

	if(m_album.size()!=0)
	{
		memcpy(&frame_hdr,"TALB",4);
		frame_hdr.SetFrameSize((int)m_album.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_album.c_str(),m_album.size());
		ptr+=m_album.size();
	}

	if(m_title.size()!=0)
	{
		memcpy(&frame_hdr,"TIT2",4);
		frame_hdr.SetFrameSize((int)m_title.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_title.c_str(),m_title.size());
		ptr+=m_title.size();
	}
	if(m_track.size()!=0)
	{
		memcpy(&frame_hdr,"TRCK",4);
		frame_hdr.SetFrameSize((int)m_track.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_track.c_str(),m_track.size());
		ptr+=m_track.size();
	}
	if(m_comment.size()!=0)
	{
		memcpy(&frame_hdr,"COMM",4);
		frame_hdr.SetFrameSize((int)m_comment.size()+1+3+1);	// +1 for ASCII character designation +3 lang +1 short desc null

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,"ENG",3);		// language
		ptr+=3;

		*ptr=0x00;	// no short content descr
		ptr++;
		
		memcpy(ptr,m_comment.c_str(),m_comment.size());
		ptr+=m_comment.size();
	}
	if(m_year.size()!=0)
	{
		memcpy(&frame_hdr,"TYER",4);
		frame_hdr.SetFrameSize((int)m_year.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_year.c_str(),m_year.size());
		ptr+=m_year.size();
	}

	delete [] buf;
/* 

	*&* TODO *&*

	if(m_track.size()!=0)
	{
		memcpy(&frame_hdr,"????",4);
		frame_hdr.SetFrameSize(m_track.size()+1);	// +1 for ASCII character designation

		memcpy(ptr,&frame_hdr,sizeof(ID3v2TagFrameHeader));
		ptr+=sizeof(ID3v2TagFrameHeader);

		*ptr=0x00;	// ASCII
		ptr++;

		memcpy(ptr,m_track.c_str(),m_track.size());
		ptr+=m_track.size();
	}
*/
	try
	{
		file->Write(buf,buf_len);
	}
	catch(CException *e)
	{
		e->Delete();
		return -1;
	}

	return 1;
}

//
//
//
string ID3v2Tag::ReturnTitle()
{
	return m_title;
}

//
//
//
string ID3v2Tag::ReturnArtist()
{
	return m_artist;
}

//
//
//
string ID3v2Tag::ReturnAlbum()
{
	return m_album;
}

//
//
//
string ID3v2Tag::ReturnTrack()
{
	return m_track;
}

//
//
//
void ID3v2Tag::SetTitle(char *title)
{
	CString cstring=title;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_title=cstring;
}

//
//
//
void ID3v2Tag::SetArtist(char *artist)
{
	CString cstring=artist;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_artist=cstring;
}

//
//
//
void ID3v2Tag::SetAlbum(char *album)
{
	CString cstring=album;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_album=cstring;
}

//
//
//
void ID3v2Tag::SetTrack(char *track)
{
	CString cstring=track;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_track=cstring;
}

//
//
//
void ID3v2Tag::SetComment(char *comment)
{
	CString cstring=comment;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_comment=cstring;
}

//
//
//
void ID3v2Tag::SetYear(char *year)
{
	CString cstring=year;
	cstring.TrimLeft();
	cstring.TrimRight();

	m_year=cstring;
}