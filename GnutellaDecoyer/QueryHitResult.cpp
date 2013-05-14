// QueryHitResult.cpp

#include "stdafx.h"
#include "QueryHitResult.h"
#include "sha.h"
#include "sha1.h"
using namespace CryptoPP;
//
//
//
QueryHitResult::QueryHitResult()
{
	Clear();
}

//
//
//
QueryHitResult::~QueryHitResult()
{
	Clear();
}

//
//
//
void QueryHitResult::Clear()
{
	m_file_index=0;
	m_file_size=0;
	m_file_name.erase();
	m_info.erase();
	m_bitrate=0;
	m_artist.erase();
	m_album.erase();
	m_title.erase();
	m_path.erase();
	m_comment.erase();
	m_type=0;
	m_duration=0;
}

//
//
//
int QueryHitResult::GetBufferLength()
{
	int len=0;

	// File Index and File Size
	len+=sizeof(unsigned int)+sizeof(unsigned int);
	len+=(int)m_file_name.size()+1;	// +1 for NULL
	len+=(int)m_info.size()+1;		// +1 for NULL

	return len;
}

//
//
//
void QueryHitResult::WriteToBuffer(char *buf)
{
	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	*((unsigned int *)ptr)=m_file_index;
	ptr+=sizeof(unsigned int);
	*((unsigned int *)ptr)=m_file_size;
	ptr+=sizeof(unsigned int);

	strcpy(ptr,m_file_name.c_str());
	ptr+=m_file_name.size()+1;

	strcpy(ptr,m_info.c_str());
	ptr+=m_info.size()+1;
}

//
//
//
bool QueryHitResult::CalculateHash()
{
	CFile file_in;
	CString filename = m_path.c_str();
	filename += m_file_name.c_str();
	if(file_in.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
		return false;
	else
	{
		
		//byte* buf = new byte[file_in.GetLength()];
		//file_in.Read(buf, file_in.GetLength());
		//SHA sha;
		//byte digest[20];
		//memset(&digest,0,sizeof(digest));
		//sha.CalculateDigest((unsigned char*)digest,buf,file_in.GetLength());
		//m_info = (const char*)Sha1toBase32(digest);	
		//delete [] buf;
		//file_in.Close();
		
		
		byte digest[20];
		memset(&digest,0,sizeof(digest));
		SHA sha;
		byte* buf = new byte[1024*1024];
		ZeroMemory(buf,1024*1024);
		UINT byte_read=file_in.Read(buf, 1024*1024);
		while(byte_read)
		{
			sha.Update(buf,byte_read);
			ZeroMemory(buf,1024*1024);
			byte_read=file_in.Read(buf, 1024*1024);
		}
		sha.Final(digest);
		file_in.Close();
		m_info = (const char*)Sha1toBase32(digest);	
		delete [] buf;
	}
	ExtractKeywordsFromQuery(m_file_name.c_str());
	ExtractKeywordsFromQuery(m_album.c_str());
	ExtractKeywordsFromQuery(m_artist.c_str());
	ExtractKeywordsFromQuery(m_title.c_str());
	ExtractKeywordsFromQuery(m_comment.c_str());
	return true;
}

//
//
//
int QueryHitResult::GetDiskStoringBufferLength()
{
	int len=0;

	// File Index and File Size
	len+=(int)m_album.size()+1;
	len+=(int)m_artist.size()+1;
	len+=sizeof(m_bitrate);
	len+=sizeof(m_duration);
	len+=sizeof(m_file_index);
	len+=(int)m_file_name.size()+1;	// +1 for NULL
	len+=sizeof(m_file_size);
	len+=(int)m_info.size()+1;		// +1 for NULL
	len+=(int)m_path.size()+1;
	len+=(int)m_title.size()+1;
	len+=(int)m_comment.size()+1;
	len+=sizeof(m_type);
	len+=sizeof(unsigned int);	// # of keywords
	for(int i=0;i<(int)v_keywords.size();i++)
	{
		len+=(int)strlen(v_keywords[i].c_str())+1;	// +1 for NULL
	}

	return len;
}

//
//
//
void QueryHitResult::WriteToDiskStoringBuffer(char* buf)
{
	char *ptr=buf;

	memset(ptr,0,GetDiskStoringBufferLength());
	strcpy(ptr,m_album.c_str());
	ptr+=m_album.size()+1;
	strcpy(ptr,m_artist.c_str());
	ptr+=m_artist.size()+1;
	*((int *)ptr)=m_bitrate;
	ptr+=sizeof(int);
	*((int *)ptr)=m_duration;
	ptr+=sizeof(int);
	*((unsigned int *)ptr)=m_file_index;
	ptr+=sizeof(unsigned int);
	strcpy(ptr,m_file_name.c_str());
	ptr+=m_file_name.size()+1;
	*((unsigned int *)ptr)=m_file_size;
	ptr+=sizeof(unsigned int);
	strcpy(ptr,m_info.c_str());
	ptr+=m_info.size()+1;
	strcpy(ptr,m_path.c_str());
	ptr+=m_path.size()+1;
	strcpy(ptr,m_comment.c_str());
	ptr+=m_comment.size()+1;
	strcpy(ptr,m_title.c_str());
	ptr+=m_title.size()+1;
	*((int *)ptr)=m_type;
	ptr+=sizeof(int);

	*((unsigned int *)ptr)=(int)v_keywords.size();
	ptr+=sizeof(unsigned int);

	for(int i=0;i<(int)v_keywords.size();i++)
	{
		strcpy(ptr,v_keywords[i].c_str());
		ptr+=strlen(v_keywords[i].c_str())+1;	// +1 for NULL
	}
}

//
//
//
int QueryHitResult::ReadFromDiskStoringBuffer(char* buf)
{
	Clear();
	char *ptr=buf;
	m_album=ptr;
	ptr+=m_album.size()+1;
	m_artist=ptr;
	ptr+=m_artist.size()+1;
	m_bitrate=*((int*)ptr);
	ptr+=sizeof(m_bitrate);
	m_duration=*((int*)ptr);
	ptr+=sizeof(m_duration);
	m_file_index=*((unsigned int *)ptr);
	ptr+=sizeof(m_file_index);
	m_file_name=ptr;
	ptr+=m_file_name.size()+1;
	m_file_size=*((unsigned int *)ptr);
	ptr+=sizeof(m_file_size);
	m_info=ptr;
	ptr+=m_info.size()+1;
	m_path=ptr;
	ptr+=m_path.size()+1;
	m_comment=ptr;
	ptr+=m_comment.size()+1;
	m_title=ptr;
	ptr+=m_title.size()+1;
	m_type=*((int *)ptr);
	ptr+=sizeof(m_type);

	int len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(UINT i=0;i<len;i++)
	{
		string keyword=ptr;
		v_keywords.push_back(keyword);
		ptr+=keyword.size()+1;
	}
	return GetDiskStoringBufferLength();
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void QueryHitResult::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')) ||
			(buf[i]=='\'') ||(buf[i]=='-'))// || (buf[i]=='.') )
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}

//
//
//
void QueryHitResult::ExtractKeywordsFromQuery(const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	cstring.MakeLower();
	strcpy(lbuf,cstring);

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<v_keywords.size();i++)
		{
			if(strcmp(v_keywords[i].c_str(),ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			v_keywords.push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}