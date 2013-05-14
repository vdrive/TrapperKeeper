#pragma once
#include "manager.h"
#include "SupernodeHost.h"
#include "ConnectionModuleStatusData.h"
#include "ConnectionModule.h"
#include "osrng.h"	// for CryptoPP
#include "PeerCacheSocket.h"

#ifdef WINMX_DC
class SupplyManager;
#endif
class KeywordManager;
class FileSharingManager;
class ConnectionManager : public Manager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);
	void SetKeywordManager(KeywordManager *keyword_manager);
	void SetFileSharingManager(FileSharingManager *file_sharing_manager);
	void AddModule();
	void ReportStatus(ConnectionModuleStatusData &status);
	void ReportHosts(vector<SupernodeHost> &hosts);
	unsigned int ReturnModCount();
	void KillModules();
	void LimitModuleCount(int count);
	void OnFiveMinuteTimer();	// 5 minute timer to write out the host cache to file
	void TimerHasFired();	// 1 sec timer
	void LogMsg(const char* msg);
	//void Search(const char* search);
	void ProjectSupplyUpdated();
	unsigned short PerformProjectSupplyQuery(char *project, unsigned int project_id);
	void KeywordsUpdated();
	vector<ProjectKeywords>* ReturnProjectKeywordsPointer();
#ifdef WINMX_DC
	vector<SupplyProject> *ReturnProjectSupplyPointer();
	void SetSupplyManager(SupplyManager *supply_manager);
#endif
	void ReConnectAll();
	bool ReadyToSearch();
	void StopSearchingProject(const char* project_name);


	FileSharingManager *p_file_sharing_manager;
	bool m_vendor_counts_enabled;

private:
	// Private Data Members
	vector<ConnectionModule *> v_mods;
	hash_set<SupernodeHost,SupernodeHostHash> hs_host_cache; //storing the main supernode list, hashed by ip
	unsigned int m_vendor_counts_timer_counter;
	KeywordManager *p_keyword_manager;
#ifdef WINMX_DC
	SupplyManager *p_supply_manager;
#endif
	int m_search_id;
	PeerCacheSocket m_peer_cache_socket;
	CryptoPP::AutoSeededRandomPool m_rng;

	// Private Member Functions
	void WriteOutHostCache();
	void ReadInHostCache();

};
