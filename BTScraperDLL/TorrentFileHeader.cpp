#include "StdAfx.h"
#include "torrentfileheader.h"

TorrentFileHeader::TorrentFileHeader(void)
{
}

TorrentFileHeader::~TorrentFileHeader(void)
{
}

int TorrentFileHeader::GetSize()
{
	int size = 0;
	size += sizeof(int)*2; // m_len, m_num
	return size;
}

int TorrentFileHeader::GetAnnounceLength()
{
	return m_len;
}

int TorrentFileHeader::GetNumPieces()
{
	return m_num;
}

void TorrentFileHeader::SetAnnounceLength(int len)
{
	m_len = len;
}

void TorrentFileHeader::SetNumPieces(int num)
{
	m_num = num;
}
