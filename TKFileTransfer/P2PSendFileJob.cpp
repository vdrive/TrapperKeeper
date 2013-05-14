#include "StdAfx.h"
#include "p2psendfilejob.h"

P2PSendFileJob::P2PSendFileJob(const char* hash, UINT file_size, const char* dest,UINT file_part)
{
	m_file_hash=hash;
	m_file_size=file_size;
	m_dest=dest;
	m_file_part=file_part;
}

P2PSendFileJob::~P2PSendFileJob(void)
{
}
