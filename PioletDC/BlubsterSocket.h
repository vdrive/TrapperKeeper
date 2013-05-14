// BlubsterSocket.h

#ifndef BLUBSTER_SOCKET_H
#define BLUBSTER_SOCKET_H

//#include "UAsyncSocket.h"
#include "UEventSocket.h"
#include "BlubsterHeader.h"
#include "BlubsterHost.h"
#include "QueryID.h"
#include "VendorCount.h"
#include "ProjectKeywords.h"
#include "SupplyProject.h"
#include "ProjectSupplyQuery.h"

class CBlubsterDlg;
class ConnectionModule;

//class BlubsterSocket : public UAsyncSocket
class BlubsterSocket : public UEventSocket
{
public:
	BlubsterSocket();
	~BlubsterSocket();
//	void InitParent(CBlubsterDlg *dlg);
	void InitHwnd(HWND hwnd);
	void InitParent(ConnectionModule* parent);
	void TimerHasFired();

	void OnReceive(int error_code);

	void SendPing(unsigned int ip,unsigned short int port=41170,unsigned char ttl=0);
	void SendPong(unsigned int ip,unsigned short int port=41170,unsigned char ttl=0);

	void PerformQuery(const char *query, unsigned int counter);
	void UpdateProjectKeywords(vector<ProjectKeywords>& keywords);
	void UpdateSupply(vector<SupplyProject>& supply_projects);
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	vector<VendorCount>* ReturnVendorCountsPointer();

private:
	// Private Member Functions
	void Log(char *buf);
	void IncrementCounters(unsigned int ping,unsigned int pong,unsigned int query,unsigned int query_hit,unsigned int unknown,unsigned int error_pong);

	void SendPings(unsigned char ttl=0);
	void ForwardQuery(unsigned int src_ip,unsigned int counter,unsigned char ttl,char *query);

	void AddHost(unsigned int ip,unsigned short int port,unsigned int num_connections, const char* vendor);
	void LoadHostCache();

	void AddVendor(CString vendor_name);
	void ProcessQuery(int ip, string query, unsigned char ttl);
	bool IsStringValid(const char *ptr);
	void ExtractKeywordsFromQuery(vector<string> *keywords, const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
	void ProcessQueryHit(unsigned int counter, int ip, string nickname, string filename, UINT file_length_in_bytes, int bitrate, int sampling_rate,int stereo,
						int song_length_in_seconds, char* md5);
	int CheckQueryHitRequiredKeywords(ProjectSupplyQuery *psq,char *lfilename,unsigned int file_size);
	void ReadMaximumHostsDataFromRegistry();



	// Private Data Members
//	CBlubsterDlg *p_dlg;
	HWND m_hwnd;
	unsigned int m_host_cache_counter;

	unsigned int m_ping_timer;
	vector<BlubsterHost> v_hosts;
	vector<BlubsterHost> v_host_cache;
	vector<QueryID> v_query_cache;
	vector<QueryID> v_query_hit_cache;
	vector<VendorCount> v_vendor_counts;
	ConnectionModule* p_connection_module;
	vector<ProjectKeywords> v_keywords;
	vector<SupplyProject> v_supply_projects;

	UINT m_max_host;
	UINT m_max_host_cache;
public:
	void UpdateHostLimit(UINT max_host);
	void UpdateHostCacheLimit(UINT max_host_cache);
};

#endif // BLUBSTER_SOCKET_H