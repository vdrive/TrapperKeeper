#pragma once
#include "SupernodeHost.h"
#include "ConnectionModuleStatusData.h"
#include "ConnectionModule.h"
#include "Manager.h"
//#include <set>
//using namespace std;

class ConnectionManager: public Manager
{
private:
	//	SLSKaccountInfo account_info;
	// Private Data Members
	CString m_user_name;
	vector<ConnectionModule *> server_mods;
	vector<ConnectionModule *> parent_mods;
	vector<ConnectionModule *> peer_mods;
	int m_parent_mods;
	int m_peer_mods;
	int m_server_mods;
	int m_modnumber;
	
	hash_set<SupernodeHost,SupernodeHostHash> hs_host_cache; //storing the main supernode list, hashed by ip
	unsigned int m_vendor_counts_timer_counter;
	//vector<VendorCount> v_vendor_counts;
	//KeywordManager *p_keyword_manager;
	//SupplyManager *p_supply_manager;
	//unsigned short m_search_id;
	bool m_connect_to_supernode;

	// Private Member Functions
	//void WriteOutHostCache();
	//void ReadInHostCache();

	//CryptoPP::AutoSeededRandomPool m_rng;
public:
	ConnectionManager(void);
	~ConnectionManager(void);
	void CalcServerConnections(void);
	void CalcParentConnections(void);
	void CalcPeerConnections(void);
	void WriteToLog(const char* log);
	//void SetKeywordManager(KeywordManager *keyword_manager);
	//void SetSupplyManager(SupplyManager *supply_manager);
	//void SetFileSharingManager(FileSharingManager *file_sharing_manager);
	void AddModule(int state);
	//void ReportStatus(ConnectionModuleStatusData &status);
	void ConnectNewHosts(vector<SupernodeHost> &con, int state);
	void ConnectNewHosts(SLSKtask *t);
//	int ConnectNewHosts(vector<SupernodeHost> &con);
	//void ReportHosts(vector<SupernodeHost> &hosts);
	unsigned int ReturnModCount(int state);
	void KillModules();
	void KillModules(int state);
	void LimitModuleCount(int state, int count);
	//void InitAccountInfo(SLSKaccountInfo &ai);
	//void OnHeartbeat();	// 1 minute timer to write out the host cache to file
	//void TimerHasFired();	// 1 sec timer
	//void VendorCountsReady(vector<VendorCount> *vendor_counts);
	void AddParent(SupernodeHost *newparent);
	void LogMsg(const char* msg);
	void ServerMsg(const char* msg);
	void ParentMsg(const char* msg);
	void PeerMsg(const char* msg);
	void SetUserName(CString *pUn);
	void setParentConnecting(const char* msg);
	void setPeerConnecting(const char* msg);
	vector<SupernodeHost> server_ips;
	vector<SupernodeHost> parent_ips;
	vector<SLSKtask *> tasks;
	void SetNewInformation(void);
	//void Search(const char* search);
	//void ProjectSupplyUpdated();
	//unsigned short PerformProjectSupplyQuery(char *project, unsigned int project_id);
	//void KeywordsUpdated();
	//vector<ProjectKeywords>* ReturnProjectKeywordsPointer();
	//vector<SupplyProject> *ReturnProjectSupplyPointer();
	void ReConnectAll(int state);
	void SendTask(SLSKtask *t, int state);
	//bool ReadyToSearch();

//	FileSharingManager *p_file_sharing_manager;
	bool m_vendor_counts_enabled;
};