#include "StdAfx.h"
#include "hashpart.h"

HashPart::HashPart(byte* hash)
{
	memcpy(m_hash,hash,16);
}

HashPart::~HashPart(void)
{
}
