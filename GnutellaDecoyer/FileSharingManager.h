#pragma once
#include "manager.h"
#include <afxmt.h>	// for CCriticalSection
#include "osrng.h"	// for CryptoPP
#include "FileSharingManagerThreadData.h"
#include "QueryHitResult.h"
#include "FileSizeAndHash.h"

class FileSharingManager :	public Manager
{
public:
	FileSharingManager(void);
	~FileSharingManager(void);
	void InitParent(GnutellaDecoyerDll* parent, HWND hwnd);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnOneMinuteTimer();
	void TerminateThread();
	void ConstructHashTable(void);
	void FileSharingManagerUpdateSharedFilesDone();
	FileSizeAndHash GetFileSize(const char* hash);
	void RebuildSharedFilesList(void);
	void GetSpoofEntriesSizes(UINT& spoof_size, UINT& temp_spoof_size, UINT& hash_size);
	void StoreUploadLog(UploadInfo& info);

private:
	vector<UploadInfo> v_upload_logs;
	vector<QueryHitResult> v_share_files;		//storing all supply info
	vector<QueryHitResult> v_temp_share_files;
	hash_set<FileSizeAndHash,FileSizeAndHashHash> hs_hashes; //storing all the supply hashes and sizes
	HWND m_dlg_hwnd;

	CCriticalSection m_upload_log_critical_section;
	CCriticalSection m_critical_section;
	CryptoPP::AutoSeededRandomPool m_rng;
	CCriticalSection* p_critical_section;

	CWinThread* m_thread;
	FileSharingManagerThreadData* p_thread_data;
	UINT m_minute_counter;
	bool m_rebuilding_db;
	
	void ConstructSharedFileList();
	void SaveSharingDB();
	bool ReadSharingDB();
};
