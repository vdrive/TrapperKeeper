// GUIDCacheItem.cpp

#include "stdafx.h"
#include "GUIDCacheItem.h"

//
//
//
GUIDCacheItem::GUIDCacheItem()
{
	Clear();
}

//
//
//
GUIDCacheItem::~GUIDCacheItem()
{
}

//
//
//
void GUIDCacheItem::Clear()
{
	memset(&m_guid,0,sizeof(GUID));
	m_count=0;
}