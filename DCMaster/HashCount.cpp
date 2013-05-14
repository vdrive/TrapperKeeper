#include "StdAfx.h"
#include "hashcount.h"

HashCount::HashCount(void)
{
	m_track=0;
	m_hash_count=0;
	m_expiration_date = CTime(3000,1,1,0,0,0);
}

HashCount::~HashCount(void)
{
}
