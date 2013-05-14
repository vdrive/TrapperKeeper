#include "StdAfx.h"
#include "swarm.h"

Swarm::Swarm(const char* file_name, int file_size,byte *hash,byte* hash_set)
{
	m_hash_set=hash_set;
	memcpy(m_hash,hash,16);
	m_shash=EncodeBase16(hash,16);
	m_file_name=file_name;
	m_file_size=file_size;
}

Swarm::~Swarm(void)
{
	if(m_hash_set)
		delete[] m_hash_set;
}
