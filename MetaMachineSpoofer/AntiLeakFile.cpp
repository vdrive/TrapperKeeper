#include "StdAfx.h"
#include "antileakfile.h"

AntiLeakFile::AntiLeakFile(const char* file_name,const char* str_hash,byte* hash,UINT size)
{
	memcpy(m_hash,hash,16);
	m_str_hash=str_hash;
	m_file_name=file_name;
	m_size=size;
}

AntiLeakFile::~AntiLeakFile(void)
{
}
