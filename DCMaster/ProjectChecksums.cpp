//ProjectChecksums.h

#include "stdafx.h"
#include "ProjectChecksums.h"

	
//
//
//
ProjectChecksums::ProjectChecksums()
{
	Clear();
}

	
//
//
//
ProjectChecksums::~ProjectChecksums()
{
	Clear();
}

	
//
//
//
void ProjectChecksums::Clear()
{
	v_checksums.clear();
}
	
//
//
//
int ProjectChecksums::GetBufferLength()
{
	int i;
	int len=0;

	len+=sizeof(unsigned int);	// number of checksums
	for(i=0;i<(int)v_checksums.size();i++)
	{
		len+=v_checksums[i].GetBufferLength();
	}

	return len;
}

	
//
//
//
int ProjectChecksums::WriteToBuffer(char *buf)
{
	int i;

	// Clear the buffer
	memset(buf,0,GetBufferLength());
	
	char *ptr=buf;

	*((int *)ptr)=(int)v_checksums.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_checksums.size();i++)
	{
		ptr+=v_checksums[i].WriteToBuffer(ptr);
	}

	return GetBufferLength();
}
	
//
//
//
int ProjectChecksums::ReadFromBuffer(char *buf)
{
	int i;
	int len;
	
	Clear();

	char *ptr=buf;

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		ProjectChecksum checksum;
		ptr+=checksum.ReadFromBuffer(ptr);
		v_checksums.push_back(checksum);
	}

	return GetBufferLength();
}

//
//
//
//
// just checks the sha1, not the project name string
//
bool ProjectChecksums::operator !=(ProjectChecksums &checksums)
{
	if(v_checksums.size()!=checksums.v_checksums.size())
		return true;

	for(UINT i=0; i<v_checksums.size();i++)
	{
		if(v_checksums[i] != checksums.v_checksums[i])
			return true;
	}
	return false;
}

