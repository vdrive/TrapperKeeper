#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include <afxmt.h>

class AresSNSpoofCache : public ThreadedObject
{
public:

	AresSNSpoofCache();
	~AresSNSpoofCache(void);
protected:

	CCriticalSection m_lock;
	Vector mv_spoofs;
	Vector mv_keywords;

	UINT Run(void);
public:
	bool IsReady(void);
	void GetMatchingSpoofs(int search_type,vector<string>& v_keywords, Vector& v_final_spoofs);
	UINT GetLoadedDecoys(void);
};
