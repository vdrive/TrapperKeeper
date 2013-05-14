#pragma once
#include "manager.h"
#include "GnutellaHost.h"
#include "ConnectionModuleStatusData.h"
#include "GWebCache.h"
#include "DBInterface.h"
#include "HashTable.h"
#include "DataBaseInfo.h"
#include "NetAddress.h"

class ConnectionModule;
class KeywordManager;
class SupplyManager;
class ProjectKeywords;
class SupplyProject;
class VendorCount;
class DBManager;
class ConnectionManager :public Manager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);

	void InitKeywordManager(KeywordManager *keyword_manager);
	void InitSupplyManager(SupplyManager *supply_manager);
	void InitDBManager(DBManager *db_manager);

	void AddModule();
	void ReportStatus(ConnectionModuleStatusData &status);
	void ReportHosts(vector<GnutellaHost> &hosts);

	unsigned int ReturnModCount();

	void KeywordsUpdated();
	vector<ProjectKeywords> *ReturnProjectKeywordsPointer();
	void ProjectSupplyUpdated(char *project_name);
	vector<SupplyProject> *ReturnProjectSupplyPointer();

	void KillModules();

	void LimitModuleCount(int count);

	void OnHeartbeat();	// 1 minute timer to write out the host cache to file

	void TimerHasFired();	// 1 sec timer
	void VendorCountsReady(vector<VendorCount> *vendor_counts,int compression_on_counts);
	void ConnectToHost(CString host);
	void ReadInDecoyHashes(DataBaseInfo& db_info);

	// Public Data Members
	char *p_compressed_qrp_table;
	unsigned int m_compressed_qrp_table_len;
	SupplyManager *p_supply_manager;
	bool m_vendor_counts_enabled;
	vector<HashTable> v_decoy_hashes;
	vector<NetAddress> v_decoyers;
private:
	// Private Data Members
	vector<ConnectionModule *> v_mods;
	vector<GnutellaHost> v_host_cache;
	vector<GnutellaHost> v_high_priority_host_cache;

	KeywordManager *p_keyword_manager;
	DBManager *p_db_manager;
//	DBInterface m_db_interface;

	GWebCache m_web_cache;

	unsigned int m_vendor_counts_timer_counter;
	vector<VendorCount> v_vendor_counts;

	// Private Member Functions
	void CreateQRPTable();
	void WriteOutHostCache();
	void ReadInHostCache();
	bool FilterHost(const char* host);
	void ReadInBearShareIPs();
	void ExtractKeywordsFromQuery(vector<string> *keywords,const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);


};
