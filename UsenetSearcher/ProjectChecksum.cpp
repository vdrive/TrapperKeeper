// ProjectChecksum.cpp

#include "stdafx.h"
#include "ProjectChecksum.h"
#include "sha.h"

//
//
//
ProjectChecksum::ProjectChecksum()
{
	Clear();
}

//
//
//
ProjectChecksum::~ProjectChecksum()
{
	Clear();
}

//
//
//
void ProjectChecksum::Clear()
{
	m_project_name.erase();
	memset(m_sha1,0,sizeof(m_sha1));
}

//
//
//
int ProjectChecksum::GetBufferLength()
{
	int len=0;
	len+=(int)m_project_name.size()+1;
	len+=sizeof(m_sha1);
	return len;
}

//
//
//
int ProjectChecksum::WriteToBuffer(char *buf)
{
	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	strcpy(ptr,m_project_name.c_str());
	ptr+=m_project_name.size()+1;

	memcpy(ptr,m_sha1,sizeof(m_sha1));
	ptr+=sizeof(m_sha1);

	return GetBufferLength();
}

//
//
//
int ProjectChecksum::ReadFromBuffer(char *buf)
{
	char *ptr=buf;

	Clear();

	m_project_name=ptr;
	ptr+=strlen(ptr)+1;

	memcpy(m_sha1,ptr,sizeof(m_sha1));
	ptr+=sizeof(m_sha1);

	return GetBufferLength();
}


//
// just checks the sha1, not the project name string
//
bool ProjectChecksum::operator ==(ProjectChecksum &checksum)
{
	if(memcmp(m_sha1,checksum.m_sha1,sizeof(m_sha1))==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
// just checks the sha1, not the project name string
//
bool ProjectChecksum::operator !=(ProjectChecksum &checksum)
{
	if(memcmp(m_sha1,checksum.m_sha1,sizeof(m_sha1))==0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//
//
//
void ProjectChecksum::Checksum(char *buf,int buf_len)
{
	CryptoPP::SHA sha;
	sha.Update((const unsigned char *)buf,buf_len);
	sha.Final((unsigned char *)m_sha1);
}

//
//
//
char *ProjectChecksum::Checksum()
{
	return &m_sha1[0];
}
