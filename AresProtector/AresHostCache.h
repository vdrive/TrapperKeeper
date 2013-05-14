#pragma once
#include "AresHost.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\threadedobject.h"
#include <afxmt.h>

class AresHostCache : public ThreadedObject
{
private:
	bool mb_has_saved_once;
	Vector mv_hosts;
	Vector mv_new_hosts;
	Vector mv_md_hosts;
	int m_host_offset;

	bool mb_ready;
	CCriticalSection m_lock;
	CCriticalSection m_md_host_lock;
	//void ReadInHostFile(void);
	void Update(void);
	//void WriteOutHostsFile(void);
public:
	AresHostCache(void);
	~AresHostCache(void);
	
	AresHost* GetNextHost(void);
	void AddHosts(Vector& v_hosts);

	inline unsigned int Size(){return mv_hosts.Size();}
	
	
	UINT Run(void);
	void StartSystem(void);
	void StopSystem(void);
	bool IsReady(void);
};
