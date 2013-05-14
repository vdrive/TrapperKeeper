#pragma once
#include "UEventSocket.h"
#include "..\DCMaster\ProjectKeywords.h"
#include "UserGroupHashes.h"
#include "DonkeyMD4.h"
#include "SearchSet.h"
#include "PublishQueue.h"
#include "UserHash.h"
#include "KeywordHash.h"
#include "PacketBuffer.h"

class ConnectionModule;
class OvernetUDPSocket :
	public UEventSocket
{
public:
	OvernetUDPSocket(void);
	~OvernetUDPSocket(void);
	void InitHwnd(HWND hwnd);
	void InitParent(ConnectionModule* parent);
	void UpdateProjectKeywords(vector<ProjectKeywords>& keywords);
	void OnReceive(int error_code);
	void OneSecondTick();
	UINT ReturnNumberOfPeers();
	void ClearStatus();
	//void BuildSpoofs();

private:
	DonkeyFile m_df;
	ConnectionModule* p_connection_module;
	HWND m_hwnd;
	//unsigned char m_my_id[16];
	unsigned short int m_my_udp_port; //udp port
	unsigned short int m_my_tcp_port; //tcp port
	UINT m_my_ip;
	char m_my_ip_str[16];
	UINT m_seconds;
	UINT m_hash_search_index;
	//UINT m_num_unique_hashes_to_be_published;
	//UINT m_num_total_hashes_to_be_published;
	//UINT m_num_hashes_not_yet_published;
	//UINT m_keywords_published;
	//UINT m_publish_ack_received;
	UINT m_publicize_ack_received;
	UINT m_next_reset_time;
	//bool m_completed_searching_one_round;
	UINT m_queries_received;
	UINT m_publish_received;
	UINT m_spoofs_sent;
	unsigned int m_overnet_dc_ip;
	unsigned int m_overnet_dc2_ip;
	UINT m_number_overnet_users;


	vector<UserGroupHashes> v_group_hashes; //user hashes sorted by the first byte i.e., v_group_hashes[0] = 0x00 firstbyte, v_group_hashes[1] = 0x01...etc
	vector<ProjectKeywords>v_all_project_keywords;
	hash_set<UserHash,UserHashHash> hs_my_user_hashes;
	hash_set<KeywordHash,KeywordHashHash> hs_keyword_hashes;
	//hash_set<SearchSet,SearchSetHash> hs_hashes;
	//hash_set<PublishQueue,PublishQueueHash> hs_publish_queues;


	void GetMyIP();
	void SendConnect(const char* ip, unsigned short int port, byte* hash);
	void SendConnect(UINT ip, unsigned short int port, byte* hash);
	void SendSearch(byte* id, UINT ip, unsigned short int port, byte search_type);
	//void ExtractKeywordsFromSupply(vector<string> *keywords,const char *query);
	int GetHashDistince(byte* hash1, byte* hash2);
	CString GetExtension(const char* filename);
	void GetArtistKeywords(const char* artist, vector<CString>* artist_keywords);
	void UpdateStatus();
	//void SendPublishKeywordHash(UINT ip, unsigned short int port,const MetaData& meta);
	//UINT ReturnNumberOfPublishQueues();
	//UINT ReturnNumberOfRemainingKeywordHashesToBePublished();
	//void LogStatusToFile();
	void SendPublicizeAck(UINT ip, unsigned short int port);
	void SendPublicizeMyself(UINT ip, unsigned short int port, byte* hash);
	void BuildKeywordHashList();
	void FindMyNearestUserHash(byte* hash, vector<UserHash>& hashes);
	void GetHashFromWord(byte* hash, char* word);
	void PartialRandomizeHash(byte* hash, int start);
	int MatchingTrack(const char* query, vector<vector<ProjectKeywords>::iterator>& projects, UINT &project_id, bool& audio_project);
	void MatchingTracks(vector<vector<ProjectKeywords>::iterator>& projects, UINT &project_id, bool& audio_project,vector<int>& tracks);

	void ClearNonAlphaNumeric(char *buf,int buf_len);
	void ExtractKeywordsFromQuery(vector<string> *keywords,const char *query);
	void RemoveArtistNames(vector<string>& keywords, CString artist_name);
	void SendSearchNext(UINT ip, unsigned short int port,byte* hash, byte* user_hash);
	void SendSearchEnd(UINT ip, unsigned short int port,byte* hash, unsigned short int result_sent, unsigned short int total_result);
	void SendPublishAck(UINT ip, unsigned short int port,byte* hash);
	void SetNextResetTime();
	void SendSpoofResult(UINT ip, unsigned short int port, byte* hash, PacketBuffer* packet);
	void SendIPSpoofDataPacket(unsigned int src_ip,unsigned short int src_port,unsigned int dest_ip,unsigned short int dest_port,
											 byte* buf, UINT buf_len);
	void SendSearchNext(UINT ip, unsigned short int port,byte* hash, hash_set<OvernetPeer,OvernetPeerHash>& user_hashes);

};
