#pragma once
#include "..\AresProtector\AresHost.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\threadedobject.h"
#include <afxmt.h>

class AresDCHostCache : public ThreadedObject
{
private:
	bool mb_has_saved_once;
	Vector mv_hosts;
	Vector mv_new_hosts;
	int m_host_offset;
	CCriticalSection m_lock;
	void ReadInHostFile(void);
	void Update(void);
	void WriteOutHostsFile(void);



public:
	AresDCHostCache(void);
	~AresDCHostCache(void);
	
	AresHost* GetNextHost(void);
	void AddHosts(Vector& v_hosts);

	inline unsigned int Size(){return mv_hosts.Size();}	
	
	
	UINT Run(void);
};
