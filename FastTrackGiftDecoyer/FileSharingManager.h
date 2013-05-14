#pragma once
#include "manager.h"
#include "FastTrackFileInfo.h"
#include "PacketBuffer.h"
#include <afxmt.h>	// for CCriticalSection
#include "osrng.h"	// for CryptoPP
#include "DataBaseInfo.h"
#include "FileSharingManagerThreadData.h"
#include "FileSizeAndHash.h"
#include "UploadInfo.h"

class FastTrackGiftDll;
class KeywordManager;
class FileSharingManager :	public Manager
{
public:
	FileSharingManager(void);
	~FileSharingManager(void);
	PacketBuffer* GetShare();
	void InitParent(FastTrackGiftDll* parent, HWND hwnd);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnOneMinuteTimer();
	void TerminateThread();
	void GetSpoofEntriesSizes(UINT& spoof_size, UINT& temp_spoof_size, UINT& spoof_index, UINT& round, UINT& hash_size);
	void ResetSpoofsCount();
	FileSizeAndHash GetFileSize(byte* hash);
	void RebuildSharedFilesList(void);
	void FileSharingManagerUpdateSharedFilesDone();
	void DisableSharing();
	void StoreUploadLog(UploadInfo& info);


private:
	vector<FastTrackFileInfo> v_share_files;		//storing all supply info
	vector<FastTrackFileInfo> v_temp_share_files;
	hash_set<FileSizeAndHash,FileSizeAndHashHash> hs_hashes; //storing all the supply hashes and sizes
	UINT m_last_share_sent_index;
	UINT m_round;
	HWND m_dlg_hwnd;

	CCriticalSection m_critical_section;
	CryptoPP::AutoSeededRandomPool m_rng;
	CCriticalSection* p_critical_section;
	CCriticalSection m_upload_log_critical_section;

	CWinThread* m_thread;
	FileSharingManagerThreadData* p_thread_data;
	UINT m_minute_counter;
	bool m_rebuilding_db;
	vector<UploadInfo> v_upload_logs;
	
	void ConstructHashTable();
	//void ExploreFolder(char *folder, vector<FastTrackFileInfo> &v_share_files);
	//void ExtractMetaData(vector<FastTrackFileInfo> &v_share_files);
	//bool IsAudio(const char* filename);
	//bool IsVideo(const char* filename);
	//void GetID3TagInfo(vector<FastTrackFileInfo>::iterator* file);
	void SaveSharingDB();
	bool ReadSharingDB();
	CString GetTitleFromFilename(const char* filename);

};
