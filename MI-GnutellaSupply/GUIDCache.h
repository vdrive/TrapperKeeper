// GUIDCache.h
#pragma once

#include <afxmt.h>	// for CCriticalSection
#include "GUIDCacheItem.h"

class GUIDCache
{
public:
	GUIDCache(){};
	~GUIDCache(){};
	static int Ignore(GUID guid);	// -1 - not seen before, don't ignore : 0 - seen before, but don't ignore : 1 - seen before and ignore

	static CCriticalSection m_critical_section;
	static vector<GUIDCacheItem> v_guid_items;
};