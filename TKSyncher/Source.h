#pragma once
#include "object.h"
#include "TKSyncherInterface.h"
#include <afxmt.h>

class Source :
	public Object, public TKSyncherMap
{
	friend class CSyncherDialog;
private:
	string m_name;
	bool mb_delete;
	bool mb_updated;
	volatile int mb_cancel_update;  //set to true in the off chance that someone wants to shut down trapper keeper.
							//this allows it to abort a possibly intensive source directory sha1 scan.
	
public:
	bool mb_updating;
	CCriticalSection m_source_lock;
	Source(char* name,char *dir);
	~Source(void);
	bool Update(void);
	vector <string> mv_destinations;
protected:
	virtual void UpdateEngine(TKSyncherMap &map);
	void AddElement(TKSyncherMap& map, const char *full_file_name,const char *single_name);
public:
	inline const char* GetSourceName(){return m_name.c_str();};
	static string CalculateDirectorySHA1(TKSyncherMap &dir);
	bool CalculateFileSHA1(const char* file_name,string &hash);
	static void DeleteFilesAndDirectories(TKSyncherMap &map);
	bool HasBeenUpdatedOnce(void);
	void Die(void);
	bool IsValid(void);
	bool IsDestroyed(void);
	void SaveState(void);
	void LoadState(void);
	void CancelUpdate(void);
};
