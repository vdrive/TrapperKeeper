#include "StdAfx.h"
#include "fileentry.h"

FileEntry::FileEntry(void)
{

	m_track = 0;
	m_size = 0;
	m_count = 0;
	memset(m_project,0,sizeof(m_project));
	memset(m_filename,0,sizeof(m_filename));
	memset(m_hash,0,sizeof(m_hash));
	memset(m_timestamp,0,sizeof(m_timestamp));
}

FileEntry::~FileEntry(void)
{
}

bool FileEntry::operator >(FileEntry &entry)
{
	if (m_count > entry.m_count)
	{
		return 1;
	}
	else return 0;
}