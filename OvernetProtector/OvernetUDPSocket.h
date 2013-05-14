#pragma once
#include "UEventSocket.h"
#include "..\DCMaster\ProjectKeywords.h"
#include "ProjectSupplyQuery.h"
#include "UserGroupHashes.h"
#include "DonkeyMD4.h"

/*
#include "OvernetPeer.h"
#include "SearchSet.h"
#include "HashManager.h"
#include "PublishQueue.h"
*/

class ConnectionModule;
class OvernetUDPSocket :
	public UEventSocket
{
public:
	OvernetUDPSocket(void);
	~OvernetUDPSocket(void);
	void InitHwnd(HWND hwnd);
	void InitParent(ConnectionModule* parent);
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	void UpdateProjectKeywords(vector<ProjectKeywords>& keywords);
	void OnReceive(int error_code);
	void OneSecondTick();
	UINT ReturnNumberOfPeers();

/*
	UINT ReturnNumberOfHashes(UINT& hashes_not_published, UINT& total_hashes,UINT& search_index);
	UINT ReturnHashManagerNumberOfHashes();
	UINT ReturnNumberOfPublishQueues();
	UINT ReturnNumberOfRemainingKeywordHashesToBePublished();
	void RePublicizeAndRePublish();
	int AlterPublishRate(int multi);
	hash_set<SearchSet,SearchSetHash>* ReturnHashesPtr();
*/

	UINT m_publicize_sent,m_publicize_received,m_publicize_acks_received,m_files_published,m_keywords_published,m_round;
	CTime m_last_round_start_time;
	UINT m_publish_ack_received;

private:
	DonkeyFile m_df;
	ConnectionModule* p_connection_module;
	HWND m_hwnd;
	unsigned char m_my_id[16];
	unsigned short int m_my_udp_port; //udp port
	unsigned short int m_my_tcp_port; //tcp port
	UINT m_my_ip;
	char m_my_ip_str[16];
	UINT m_seconds;
	UINT m_hash_search_index;
	int m_publish_rate;
	CTime m_last_publish_ack_received;
	bool m_publish_file_hashes_completed;

	vector<UserGroupHashes> v_group_hashes; //user hashes sorted by the first byte i.e., v_group_hashes[0] = 0x00 firstbyte, v_group_hashes[1] = 0x01...etc
	//hash_set<PublishQueue,PublishQueueHash> hs_publish_queues;
	//hash_set<SearchSet,SearchSetHash> hs_hashes;
	//DonkeyFile m_df;
	//vector<CString>v_file_server_ips;
	vector<ProjectKeywords>v_all_project_keywords;
	//HashManager m_hash_manager;

	void GetMyIP();
	
	void SendConnect(const char* ip, unsigned short int port);
	void SendConnect(UINT ip, unsigned short int port);
	void SendSearchInfo(UINT ip, unsigned short int port, ProjectSupplyQuery& psq, unsigned short int start_pos=0);
	void SendSearch(byte* id, UINT ip, unsigned short int port, byte search_type);
	int CheckRequiredKeywords(ProjectSupplyQuery& psq,const char *lfilename);
	void ExtractKeywordsFromSupply(vector<string> *keywords,const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
	int CheckQueryHitRequiredKeywords(ProjectSupplyQuery& psq, const char *lfilename);
	int GetHashDistince(byte* hash1, byte* hash2);
	bool IsOneOfUs(UINT ip);





	/*
	void SendIPQuery(UINT ip, unsigned short int port);
	void SendPublicizeMyself(UINT ip, unsigned short int port);
	void SendPublicizeAck(UINT ip, unsigned short int port);
	void SendPublishFileHash(UINT ip, unsigned short int port, byte* hash, const char* src_ip, unsigned short src_port, byte* user_hash);
	void SendPublishKeywordHash(UINT ip, unsigned short int port,const MetaData& meta);
	void ReadInHashes();
	void ReadInProjectKeywords();
	void GetArtistKeywords(const char* artist, vector<CString>* artist_keywords);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
	void IPSpoof(unsigned int src_ip,unsigned short int src_port,unsigned int dest_ip,unsigned short int dest_port,
							 byte* buf, UINT buf_len);
*/


};
