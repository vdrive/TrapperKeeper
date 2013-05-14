#pragma once
#include "threadedobject.h"
#include "SyncherfileTransfer.h"
#include "TKSyncherInterface.h"
#include <mmsystem.h>
#include <afxmt.h>
#include "Vector.h"
#include "Dll.h"
#include "Timer.h"

class SyncherFileTransfer;
#define MAXFILETRANSFERS 4  //4 simulataneous file transfers can be taking place to one destination from this source alone.  The FileTransferService has its own throttling mechanisms and this cannot hurt it.

//this class encapsulates the set of interface objects that care about different sources
//It is deriving from ThreadedObject, which is derived from Object just like Source is.  This requires an extra cast to remove ambiguity for the compiler.
class SyncherSubscriber : public Object   //extend source to duplicate its functionality
{
	class ReceivedFileJob : public Object{
	public:
		string m_local_path;
		string m_remote_path;
		ReceivedFileJob(const char* local_path, const char* remote_path){
			m_local_path=local_path;
			m_remote_path=remote_path;
		}
	};
	/*
	class DeleteFileJob : public Object{
	public:
		string m_local_path;
		bool mb_directory;
		Timer m_time_request;
		DeleteFileJob(const char* local_path,bool is_directory){
			m_local_path=local_path;
			mb_directory=is_directory;
		}
	};*/
	class GetFileJob : public Object{
	public:
		string m_local_path;
		Timer m_created;
		GetFileJob(const char *local_path){
			m_local_path=local_path;
		}
	};
private:
	bool mb_valid;  //if this subscriber received a file or something changes, then this subscriber is no longer valid and Validate() needs to be called.
	bool mb_source_changed;
	unsigned long m_last_server_ping;
	Dll* p_dll;
	Timer m_last_validate;
	CCriticalSection m_data_lock;

	//we will track 3 state maps for reference and management
	TKSyncherMap m_source_map;  //the file map that the source has.
	//TKSyncherMap m_ref_source_map;  //the file map that the source has, except with the paths changed
	TKSyncherMap m_current_map;  //the file map of files that we actually have available.

	string m_source_name;
	string m_source_ip;
	vector <UINT> mv_apps;  //a vector of app id's that are subscribed to this source, if the size of this reaches zero, then the object should be destroyed.
	//unsigned long m_last_transfer;
	Vector mv_get_jobs;
	//Vector mv_delete_jobs;
	Vector mv_received_jobs;

	Vector mv_desired_files;  //files we wish to have, but don't.

	Vector mv_subscriber_children;  //i.e.  Universal.Music where Universal would be the parent, and Universal.Music the child.
	
	SyncherFileTransfer *p_file_transfer;

	SyncherSubscriber *p_parent;

	int m_current_transfers;
public:
	inline const char* GetSourceName(){return m_source_name.c_str();}
	inline const char* GetSourceIP(){return m_source_ip.c_str();}

	//an inline function that compares the sha broadcasted from a source to our known source sha
	inline bool CheckSHA1(const char* sha1,const char* source_ip){
		bool stat=false;
		m_last_server_ping=timeGetTime();
		CSingleLock lock(&m_data_lock,TRUE);
		m_source_ip=source_ip;
		const char *source_sha1=m_source_map.m_sha1.c_str();
		if(stricmp(sha1,source_sha1)!=0){
			stat=true; //if its changed, then wait until we receive the current map before we delete unused files
		}
		else{
			RemoveDebris(m_current_map.m_directory_name.c_str());
			mb_valid=false;  //lets make it do an update to request any files if need be.  This should be unnecessary, but since their might be an issue with dests not requesting files, this would definitely fix it.
		}
		return stat;
	}

	SyncherSubscriber(Dll *dll,const char* source,UINT app);
	~SyncherSubscriber(void);
	void AddRef(UINT app);
	UINT DeRef(UINT app);
	void ReceivedFile(const char* local_path , const char* remote_path);
	bool Validate(void);
	void SetSourceMap(TKSyncherMap &map,const char *source_ip);
	void FileDownloadFailed(const char* local_path);
	void LoadState(void);
	void SaveState(void);
private:
	bool IsGettingFile(const char* file);
	void UpdateSubscribers(void);
	void UpdateState();
	bool UpdateStateEx(TKSyncherMap &m_current_map,TKSyncherMap &actual_map);
	bool CleanMap(TKSyncherMap& cur_map);
	void ChangeNames(TKSyncherMap &map);
	//void ProcessDeleteJobs(void);

public:
	void Resynch(void);
private:
	//void DeleteFiles(TKSyncherMap &map);
public:
	bool DoesFileExistAlready(const char* local_path, const char* _sha1);
	void RemoveDebris(const char* directory);
	void Invalidate(void);
	void SetParent(SyncherSubscriber* parent);
	SyncherSubscriber* IsSource(const char* source_name);
	bool IsEqual(void);
	void FillTreeCtrl(CTreeCtrl& tree,HTREEITEM *parent);
	void EnumGetFiles(vector <string> &files);
	UINT CountTotalGets(void);
	SyncherSubscriber* GetParent(void);
	void PurgeChild(SyncherSubscriber* p_child,bool b_notify);
	void PurgeAll(void);
	int GetCurrentTransferWeight(void);
};

