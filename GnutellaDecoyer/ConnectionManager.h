#pragma once
#include "manager.h"
#include "GnutellaHost.h"
#include "ConnectionModuleStatusData.h"
#include "GWebCache.h"
#include "QueryHitResult.h"
#include <afxmt.h>	// for CCriticalSection

class ConnectionModule;
class VendorCount;
class ConnectionManager :public Manager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);

	void AddModule();
	void ReportStatus(ConnectionModuleStatusData &status);
	void ReportHosts(vector<GnutellaHost> &hosts);

	unsigned int ReturnModCount();

	void KillModules();

	void LimitModuleCount(int count);

	void OnHeartbeat();	// 1 minute timer to write out the host cache to file

	void TimerHasFired();	// 1 sec timer
	void VendorCountsReady(vector<VendorCount> *vendor_counts);
	void ConnectToHost(CString host);
	void NewShareFileList(vector<QueryHitResult>& share_files);
	vector<QueryHitResult> GetShareFiles();

	// Public Data Members
	char *p_compressed_qrp_table;
	unsigned int m_compressed_qrp_table_len;
	bool m_vendor_counts_enabled;
private:
	// Private Data Members
	vector<ConnectionModule *> v_mods;
	vector<GnutellaHost> v_host_cache;
	vector<GnutellaHost> v_high_priority_host_cache;

	GWebCache m_web_cache;

	unsigned int m_vendor_counts_timer_counter;
	vector<VendorCount> v_vendor_counts;
	vector<QueryHitResult> v_share_files;		//storing all supply info

	// Private Member Functions
	void CreateQRPTable();
	void WriteOutHostCache();
	void ReadInHostCache();
	bool FilterHost(const char* host);
	void ReadInBearShareIPs();

	CCriticalSection m_share_list_critical_section;

};
