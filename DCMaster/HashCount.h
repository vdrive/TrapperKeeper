#pragma once
#include "projectdatastructure.h"

class HashCount
{
public:
	HashCount(void);
	~HashCount(void);

	int m_track;
	int m_hash_count;
	CTime m_expiration_date;
};
