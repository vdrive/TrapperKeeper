#include "stdafx.h"
#include "SupplyEntry.h"
//#include "sha.h"

//
//
//
SupplyEntry::SupplyEntry()
{
	p_filename=NULL;
	p_sha1=NULL;
	//p_spoof_sha1=NULL;

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
	if(p_sha1!=NULL)
	{
		delete [] p_sha1;
		p_sha1=NULL;
	}
	/*if(p_spoof_sha1!=NULL)
	{
		delete [] p_spoof_sha1;
		p_spoof_sha1=NULL;
	}*/
}

//
//
//
SupplyEntry::SupplyEntry(const SupplyEntry &entry)
{
	p_filename=NULL;
	p_sha1=NULL;
	//p_spoof_sha1=NULL;
	if(entry.p_filename != NULL)
	{
		p_filename=new char[strlen(entry.p_filename)+1];
		strcpy(p_filename,entry.p_filename);
	}
	if(entry.p_sha1 != NULL)
	{
		p_sha1=new char[strlen(entry.p_sha1)+1];
		strcpy(p_sha1,entry.p_sha1);
	}
	//if(entry.p_spoof_sha1 != NULL)
	//{
	//	p_spoof_sha1=new char[strlen(entry.p_spoof_sha1)+1];
	//	strcpy(p_spoof_sha1,entry.p_spoof_sha1);
	//}
	m_file_length = entry.m_file_length;
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

	if(p_sha1!=NULL)
	{
		delete [] p_sha1;
		p_sha1=NULL;
	}
	if(entry.p_sha1 != NULL)
	{
		p_sha1=new char[strlen(entry.p_sha1)+1];
		strcpy(p_sha1,entry.p_sha1);
	}
	//if(entry.p_spoof_sha1 != NULL)
	//{
	//	p_spoof_sha1=new char[strlen(entry.p_spoof_sha1)+1];
	//	strcpy(p_spoof_sha1,entry.p_spoof_sha1);
	//}
	m_file_length = entry.m_file_length;
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
	if(p_sha1!=NULL)
	{
		delete [] p_sha1;
		p_sha1=NULL;
	}
	//if(p_spoof_sha1!=NULL)
	//{
	//	delete [] p_spoof_sha1;
	//	p_spoof_sha1=NULL;
	//}
	m_file_length = 0;
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
	if(p_sha1 != NULL)
		count += (int)strlen(SHA1())+1;

	count += sizeof(m_file_length);
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
	if(p_sha1 != NULL)
	{
		strcpy(ptr,SHA1());
		ptr+=strlen(SHA1())+1;
	}

	*((unsigned int *)ptr)=m_file_length;
	ptr+=sizeof(m_file_length);

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

	SHA1(ptr);
	if(SHA1()!=NULL)
		ptr+=strlen(SHA1())+1;

	m_file_length=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

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
void SupplyEntry::SHA1(char* sha1)
{
	if(p_sha1!=NULL)
	{
		delete [] p_sha1;
		p_sha1=NULL;
	}

	p_sha1=new char[strlen(sha1)+1];
	strcpy(p_sha1,sha1);

	//if(p_spoof_sha1!=NULL)
	//{
	//	delete [] p_spoof_sha1;
	//	p_spoof_sha1=NULL;
	//}
	//CryptoPP::SHA sha;
	//char new_sha1[20];
	//memset(&new_sha1,0,sizeof(new_sha1));
	//sha.Update((const unsigned char *)sha1,strlen(sha1));
	//sha.Final((unsigned char *)new_sha1);
	//CString new_sha1_str;
	//for(int j=0;j<16;j++)
	//	new_sha1_str.AppendFormat("%.2X",(byte)new_sha1[j]);
	//p_spoof_sha1=new char[new_sha1_str.GetLength()+1];
	//strcpy(p_spoof_sha1,new_sha1_str);
}

//
//
//
char *SupplyEntry::SHA1()
{
	return p_sha1;
}

////
////
////
//char* SupplyEntry::SpoofSHA1()
//{
//	return p_spoof_sha1;
//}
