#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Timer.h"
#include <afxmt.h>

class AresProjectInfoCache : public ThreadedObject
{
private:
	Vector mv_projects;
	CCriticalSection m_lock;
	bool mb_ready;
public:
	AresProjectInfoCache(void);
	~AresProjectInfoCache(void);
	UINT Run();
	void GetProjects(Vector& v);
	bool IsReady(void);
	string GetArtist(const char* project);
	bool IsProjectActive(const char* project);
};
