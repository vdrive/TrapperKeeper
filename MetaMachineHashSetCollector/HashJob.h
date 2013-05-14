#pragma once
#include "..\tkcom\object.h"

class HashJob :
	public Object
{
public:
	string m_hash;
	byte m_byte_hash[20];
	vector <string> mv_ips;
	HashJob(void);
	~HashJob(void);
	void ClearDuplicates(void);
};
