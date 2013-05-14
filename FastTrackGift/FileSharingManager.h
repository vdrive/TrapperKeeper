#pragma once
#include "manager.h"
#include "FastTrackFileInfo.h"
#include "PacketBuffer.h"
#include <afxmt.h>	// for CCriticalSection
#include "osrng.h"	// for CryptoPP
#include "DataBaseInfo.h"
#include "FileSharingManagerThreadData.h"
#include "FileSizeAndHash.h"

class FastTrackGiftDll;
class KeywordManager;
class FileSharingManager :	public Manager
{
public:
	FileSharingManager(void);
	~FileSharingManager(void);
	PacketBuffer* GetShare();
	void InitParent(FastTrackGiftDll* parent);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void DoneRetreivingSpoofEntries();
	void OnOneMinuteTimer();
	//void WriteProjectSupplyToFile();
	void ReadSupplyFromFiles();
	void TerminateThread();
	void SetKeywordManager(KeywordManager* manager);
	void UpdateFileSharingManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	void KeywordsUpdated();
	void GetSpoofEntriesSizes(UINT& spoof_size, UINT& temp_spoof_size, UINT& spoof_index, UINT& round, UINT& hash_size);
	void ResetSpoofsCount();
	UINT GetFileSize(byte* hash);
	void FileSharingManagerSupplyRetrievalFailed();


private:
	vector<FastTrackFileInfo> v_share_files;		//storing all supply info
	vector<FastTrackFileInfo> v_temp_share_files;
	hash_set<FileSizeAndHash,FileSizeAndHashHash> hs_hashes; //storing all the supply hashes and sizes
	UINT m_last_share_sent_index;
	UINT m_round;
	vector<CString> v_file_exts;
	int m_file_extension_replacing_chance;

	CCriticalSection m_critical_section;
	CryptoPP::AutoSeededRandomPool m_rng;
	CCriticalSection* p_critical_section;
	bool m_retreiving_spoof_entries;
	//bool m_retrieve_spoof_entries_again;
	CWinThread* m_thread;
	UINT m_minute_counter;
	DataBaseInfo m_raw_db_info;
	FileSharingManagerThreadData* p_thread_data;
	KeywordManager* p_keyword_manager;
	int m_next_retrieving_new_supplies_hour;
	int m_next_retrieving_new_supplies_minute;

	bool RetreiveNewSpoofEntries();
	void DeleteAllSpoofFiles();
	bool IsDecoyingEnabled(UINT project_id);
	void ConstructHashTable();
	void ReadFileExtensionReplacement();




/*
	PacketBuffer* GetPayCheckSpoof(void);
	vector<CString> v_spoof_filenames;
	unsigned short GetHashChecksum (unsigned char *hash);
*/
};
