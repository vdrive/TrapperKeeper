#pragma once
#include "..\sampleplugin\dll.h"
#include "GnutellaDecoyerDlg.h"
#include "ConnectionManager.h"
#include "SyncherDest.h"
#include "FileSharingManager.h"
#include "NoiseManager.h"
#include "NoiseModuleThreadStatusData.h"

#include <pdh.h>

class GnutellaDecoyerDll :
	public Dll
{
public:
	GnutellaDecoyerDll(void);
	~GnutellaDecoyerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void EnableVendorCounts(BOOL enable);
	int AlterModuleCounts(int min,int max);
	void OnTimer(UINT nIDEvent);
	void ReportConnectionStatus(ConnectionModuleStatusData& status);
	void Log(const char* log);
	void VendorCountsReady(vector<VendorCount> &vendor_counts);
	void ShowSocketStatus(const char* msg);
	void FileSharingManagerUpdateSharedFilesDone();
	void InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	void RebuildSharingDB();
	int GetModCount();
	void DisableSharing();
	void EnableSharing();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);

	CGnutellaDecoyerDlg m_dlg;
	int m_min_module_count;
	int m_max_module_count;
	void DecoySupplySynched();
	void NewShareFileList(vector<QueryHitResult> share_files,hash_set<FileSizeAndHash,FileSizeAndHashHash> hashes);
private:
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void CheckModuleConnections();

	NoiseManager m_noise_manager;
	FileSharingManager m_file_sharing_manager;
	SyncherDest m_decoy_syncher;
	SyncherDest m_host_cache_syncher;
	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;
	ConnectionManager m_connection_manager;
	vector<ConnectionModuleStatusData> v_mod_status;
};
