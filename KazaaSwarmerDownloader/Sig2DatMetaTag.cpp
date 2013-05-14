#include "StdAfx.h"
#include "sig2datmetatag.h"

Sig2DatMetaTag::Sig2DatMetaTag(void)
{

	m_tagId = 3;
	m_tagLength = 20;
}

Sig2DatMetaTag::~Sig2DatMetaTag(void)
{
}


int Sig2DatMetaTag::GetSize()
{
	int size;
	size = (m_tagLength + (sizeof(DWORD) *2));
	return size;

}