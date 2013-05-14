//AUTHOR:  Norman Heath
//LAST MODIFIED:  3.17.2003
//PURPOSES:  Manages the synching of files and directories between a source and destination(s).

//FUNCTIONALITY
//1.  If files are deleted from the source, a continous attempt will be made to delete them from the destination(s)
//2.  If files are added to the appropriate source directory, a continous attempt will be made to mirror only the necessary files onto the destination until successful.
//3.  Extraneous files placed in the synching directories on the destination will not be tampered with.
//4.  Destinations will serialize their state and restore it when they are restarted.
//5.  Sources will serialize their destination set.

//TODO:
//1.  Broadcast sha1's only for a particular source that has changed.  (currently it does it for all sources every x seconds, or if any source has changed)

#pragma once
#include "threadedobject.h"
#include "vector.h"
#include <afxmt.h>
#include "SyncherComLink.h"
#include "LoadBalanceSystem.h"
#include "Dll.h"
class SyncherComLink;

class SyncherService :
	public ThreadedObject
{
	class RescanJob : public Object{
	private:
		string source_name;
	public:
		RescanJob(const char* _source_name){source_name=_source_name;}
		inline const char* GetSource(){return source_name.c_str();}
	};
public:
	CCriticalSection m_source_lock;
	CCriticalSection m_source_search_lock;
	CCriticalSection m_subscriber_lock;
	Vector mv_sources;
	Vector mv_subscribers;
	Vector mv_rescan_jobs;

	SyncherService(void);
	~SyncherService(void);
	virtual UINT Run();
	void StartSyncherService();
	void StopSyncherService(void);
	bool ReceivedComData(char* source_ip,byte* data, UINT size);
	bool ReceivedDllData(UINT from_app,byte* data);

private:
	Dll *p_dll;
	SyncherComLink *p_com_link;
	LoadBalanceSystem m_load_balancer;
	bool mb_directories_created;
	bool mb_needs_source_scan;
	void UpdateSources(void);
	void RestartComputer(void);
	void CleanDirectory(const char* directory);

public:
	void SetComLink(SyncherComLink* com_link);
	void RescanSources(void);
	Dll* GetDLL(void);
	void SetDLL(Dll* dll);
	LoadBalanceSystem* GetLoadBalancer(void);
	void CheckForAutoClean(void);
	UINT GetLargestDbbFileInDirectory(const char* directory);
};
