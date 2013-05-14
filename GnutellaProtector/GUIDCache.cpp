// GUIDCache.cpp

#include "stdafx.h"
#include "GUIDCache.h"

CCriticalSection GUIDCache::m_critical_section;
vector<GUIDCacheItem> GUIDCache::v_guid_items;

//
// -1 - not seen before, don't ignore : 0 - seen before, but don't ignore : 1 - seen before and ignore
// Only ignore a query once we've responded to it 5 times
//
int GUIDCache::Ignore(GUID guid)
{
	int i;
	int ret=-1;	// not seen before, don't ignore

	CSingleLock singleLock(&GUIDCache::m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		// See if the guid is in the vector (starting at the end)
		vector<GUIDCacheItem>::iterator item_iter=v_guid_items.end()-1;
		for(i=(int)GUIDCache::v_guid_items.size()-1;i>=0;i--)
		{
			if(GUIDCache::v_guid_items[i].m_guid==guid)
			{
				ret=0;	// seen before, but don't ignore
				v_guid_items[i].m_count++;	// increment the number of times we've seen this guid

				// Check to see if it has reached the limit
				if(v_guid_items[i].m_count>5)
				{
					ret=1;	// seen before, and ignore
				}

				// Remove the old entry, and push it back at the head of the queue (since we just saw it)
				GUIDCacheItem item=v_guid_items[i];	// make a copy
				v_guid_items.erase(item_iter);		// remove old one
				v_guid_items.push_back(item);		// push it back

				break;
			}

			item_iter--;
		}

		// If not found, then add it to the vector
		if(ret==-1)
		{
			GUIDCacheItem item;
			item.m_guid=guid;
			item.m_count++;
			GUIDCache::v_guid_items.push_back(item);

			// Limit the size of the GUID cache
			while(GUIDCache::v_guid_items.size()>1000)
			{
				GUIDCache::v_guid_items.erase(GUIDCache::v_guid_items.begin());
			}
		}

		singleLock.Unlock();
	}

	return ret;
}