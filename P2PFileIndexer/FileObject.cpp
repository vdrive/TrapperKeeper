#include "StdAfx.h"
#include "fileobject.h"

FileObject::FileObject(const char* hash, UINT size)
{
	m_hash=hash;
	m_size=size;
}

FileObject::~FileObject(void)
{
}

bool FileObject::IsFile(const char* hash, UINT size)
{
	int test_size=(int)size;
	if(stricmp(hash,m_hash.c_str())==0 && (m_size==size || test_size==-1))
		return true;
	else
		return false;
}

const char* FileObject::GetHash(void)
{
	return m_hash.c_str();
}
