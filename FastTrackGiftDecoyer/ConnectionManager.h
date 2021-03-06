#pragma once
#include "manager.h"
#include "SupernodeHost.h"
#include "ConnectionModuleStatusData.h"
#include "ConnectionModule.h"
#include "osrng.h"	// for CryptoPP

class VendorCount;
class FileSharingManager;
class ConnectionManager : public Manager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);
	void SetFileSharingManager(FileSharingManager *file_sharing_manager);
	void AddModule();
	void ReportStatus(ConnectionModuleStatusData &status);
	void ReportHosts(vector<SupernodeHost> &hosts);
	unsigned int ReturnModCount();
	void KillModules();
	void LimitModuleCount(int count);
	void OnHeartbeat();	// 1 minute timer to write out the host cache to file
	void TimerHasFired();	// 1 sec timer
	void VendorCountsReady(vector<VendorCount> *vendor_counts);
	void LogMsg(const char* msg);
	void ReConnectAll();

	FileSharingManager *p_file_sharing_manager;
	bool m_vendor_counts_enabled;

private:
	// Private Data Members
	vector<ConnectionModule *> v_mods;
	hash_set<SupernodeHost,SupernodeHostHash> hs_host_cache; //storing the main supernode list, hashed by ip
	unsigned int m_vendor_counts_timer_counter;
	vector<VendorCount> v_vendor_counts;
	unsigned short m_search_id;
	bool m_connect_to_supernode;

	// Private Member Functions
	void WriteOutHostCache();
	void ReadInHostCache();

	CryptoPP::AutoSeededRandomPool m_rng;
};
