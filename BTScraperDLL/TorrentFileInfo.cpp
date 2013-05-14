#include "torrentfileinfo.h"

CTorrentFileInfo::CTorrentFileInfo(void)
{
	ZeroMemory(m_sha1, 20*sizeof(unsigned short));
	ZeroMemory(m_ed2k, 16*sizeof(unsigned short));
}

CTorrentFileInfo::~CTorrentFileInfo(void)
{
}


CTorrentFileInfo::CTorrentFileInfo(const CTorrentFileInfo & info)
{
	operator=(info);
}

CTorrentFileInfo & CTorrentFileInfo::operator=(const CTorrentFileInfo & info)
{	
	m_length = info.m_length;
	memcpy(m_ed2k, info.m_ed2k, 20*sizeof(unsigned short));
	m_md5sum = info.m_md5sum;
	m_path = info.m_path;
	m_path_utf8 = info.m_path_utf8;
	memcpy(m_sha1, info.m_sha1, 20*sizeof(unsigned short));
	return *this;
}

int CTorrentFileInfo::GetLength()
{
	return m_length;
}

unsigned short * CTorrentFileInfo::GetEd2k()
{
	return m_ed2k;
}

string CTorrentFileInfo::GetMd5sum()
{
	return m_md5sum;
}
string CTorrentFileInfo::GetPath()
{
	return m_path;
}
string CTorrentFileInfo::GetPathUTF8()
{
	return m_path_utf8;
}
unsigned short * CTorrentFileInfo::GetSha1()
{
	return m_sha1;
}
void CTorrentFileInfo::SetLength(int len)
{
	m_length = len;
}


void CTorrentFileInfo::SetMd5sum(string sum)
{
	m_md5sum = sum;
}
void CTorrentFileInfo::SetPath(string path)
{
	m_path = path;
}
void CTorrentFileInfo::SetPathUTF8(string path)
{
	m_path_utf8 = path;
}
