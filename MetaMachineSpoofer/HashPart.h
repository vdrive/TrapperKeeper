#pragma once
#include "..\tkcom\object.h"

class HashPart : public Object
{
public:
	byte m_hash[16];
	HashPart(byte* hash);
	~HashPart(void);
};
