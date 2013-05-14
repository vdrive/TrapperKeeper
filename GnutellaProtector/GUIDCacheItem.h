// GUIDCacheItem.h
#pragma once

class GUIDCacheItem
{
public:
	GUIDCacheItem();
	~GUIDCacheItem();
	void Clear();

	GUID m_guid;
	unsigned int m_count;
};