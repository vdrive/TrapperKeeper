#pragma once
#include "manager.h"
#include "..\FastTrackGift\PacketBuffer.h"
#include "osrng.h"	// for CryptoPP
#include "QueryHit.h"
#include "FileSizeAndHash.h"

class WinMxDcDll;
class KeywordManager;
class FileSharingManager :	public Manager
{
public:
	FileSharingManager(void);
	~FileSharingManager(void);
	PacketBuffer* GetShare();
	void InitParent(WinMxDcDll* parent);
	void DoneRetreivingSpoofEntries();
	void ReadSupplyFromFiles();
	void TerminateThread();
	void SetKeywordManager(KeywordManager* manager);
	void UpdateFileSharingManagerSupplyProjects();
	void KeywordsUpdated();
	void GetSpoofEntriesSizes(UINT& spoof_size, UINT& spoof_index, UINT& round, UINT& hash_size);
	void ResetSpoofsCount();
	UINT GetFileSize(const char* filename);

private:
	vector<QueryHit> v_share_files;		//storing all supply info

	hash_set<FileSizeAndHash,FileSizeAndHashHash> hs_hashes; //storing all the supply hashes and sizes
	UINT m_last_share_sent_index;
	UINT m_round;

	CRITICAL_SECTION m_critical_section;
	CryptoPP::AutoSeededRandomPool m_rng;
	KeywordManager* p_keyword_manager;

#ifdef WINMX_SWARMER
	void ConstructHashTable();
#endif

	 //return a buffer contains this file share info in WinMx format, caller is responsible for freeing the memory
	PacketBuffer* GetShareFile(QueryHit supply, bool same_hash = false);
};
