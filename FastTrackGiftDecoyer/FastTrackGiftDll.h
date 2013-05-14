#pragma once
#include "..\sampleplugin\dll.h"
#include "FastTrackGiftDlg.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include <pdh.h>
#include "ConnectionManager.h"
#include "NoiseManager.h"
#include "NoiseModuleThreadStatusData.h"
#include "FileSharingManager.h"
#include "KazaaControllerDestSyncher.h"

class FastTrackGiftDll :
	public Dll
{
public:
	FastTrackGiftDll(void);
	~FastTrackGiftDll(void);
//	void DataReceived(char *source_name, void *data, int data_length);
	int GetMinModuleCount();
	void RemoveAllModules();
	void ManualPauseSearching(bool pause);
	void ApplyAutoSearchingThreshold(int pause_threshold, int resume_threshold);
	void FileSharingManagerSupplyRetrievalFailed();
	void SendSupplyRetrievalProgress(int progress);
	void EnableVendorCounts(BOOL enable);
	void FileSharingManagerUpdateSharedFilesDone();
	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);
	void DisableSharing();
	void EnableSharing();
	void RebuildSharingDB();


private:
	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;
	int m_min_module_count;
	int m_max_module_count;
	vector<ConnectionModuleStatusData> v_mod_status;
	ConnectionManager m_connection_manager;
	KazaaControllerDestSyncher m_syncher;
	int m_auto_pause_threshold;
	int m_auto_resume_threshold;
	char m_dc_master[32];


	NoiseManager m_noise_manager;
	FileSharingManager m_file_sharing_manager;

	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void CheckModuleConnections();

public:
	CFastTrackGiftDlg m_dlg;

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void Log(const char* log);
	void ReportConnectionStatus(ConnectionModuleStatusData& status);
	void VendorCountsReady(vector<VendorCount> &vendor_counts);
	void OnTimer(UINT nIDEvent);
	int AlterModuleCounts(int min,int max);
	void ShowSocketStatus(const char* msg);
	int GetModCount();
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);
	void InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ReConnectToAllSupernodes();
};
