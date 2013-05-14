#include "StdAfx.h"
#include "kadfilekey.h"
#include "KadUtilityFunctions.h"

KadFileKey::KadFileKey(const char* name)
{
	m_name=name;
	byte output[16];
	KadUtilityFunctions::HashData((byte*)m_name.c_str(),(UINT)strlen(name),output);
	m_key.setValueBE(output);
}

KadFileKey::~KadFileKey(void)
{
}
