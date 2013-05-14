#pragma once
#include "TEventSocket.h"
//#include "FastTrackCrypt.h"
//#include "PacketBuffer.h"
#include "SupernodeHost.h"
#include "SLSKSocketStatusData.h"
//#include "ProjectSupplyQuery.h"
//#include "FastTrackMeta.h"
#include "SLSKtask.h"
#include "buffer.h"
//#include "CSLSKSpooferDlg.h"


#define MAX_MSG_LENGTH				1024*1024*10 //10 MB
#define MIN_USERS_ON_CONNECT		1000000 //we want to connect to at least 1 million users network
//#define FST_NETWORK_NAME "KaZaA"
//#define FST_MAX_SEARCH_RESULTS		200

/* session states */
/*typedef enum
{
	SessNew,
	SessConnecting,
	SessHandshaking,
	SessWaitingNetName,
	SessEstablished,
	SessDisconnected
} FSTSessionState;
*/
/* session messages sent to callback */
/*typedef enum
{
	// our own stuff /
	SessMsgConnected = 0x01FF,		// tcp connection established //
	SessMsgEstablished = 0x02FF,	// encryption initialized and we received
									 * a valid network name
									 //
	SessMsgDisconnected = 0x03FF,	// tcp connection closed //
	
	/* FastTrack messages //
	SessMsgNodeList = 0x00,
	SessMsgUserInfo = 0x02,
	SessMsgQuery = 0x06,
	SessMsgQueryReply = 0x07,
	SessMsgQueryEnd = 0x08,
	SessMsgNetworkStats = 0x09,
	SessMsgNetworkName = 0x1d,
	SessMsgPushRequest = 0x0d,
	SessMsgExternalIp = 0x2c,      /* supernode tells us our outside ip //
	SessMsgShareFile = 0x22,
	SessMsgUnshareFile = 0x05,
	SessMsgProtocolVersion = 0x26  /* we think this is the protocol version // 

} FSTSessionMsg;
*/

struct w_keyword
{
public:
	int m_project_id;
	CString m_keyword;
	int m_weight;
};

/*struct track
{
public:
	int m_track_number;
	CString m_track_name;
};
*/
struct project_info
{
public:
	int m_project_id;
	CString m_artist;
	CString m_album;
	vector<track> tracks;
};

struct spoof_info
{
public:
	int m_project_id,
		m_track_number,
		m_track_length,
		m_bit_rate,
		m_track_size;
};

class ConnectionSockets;
class SLSKSocket :	public TEventSocket
{
public:
	SLSKSocket(void);
	~SLSKSocket(void);
	void InitParent(ConnectionSockets* parent);
	void InitTasks(vector<SLSKtask> t1, vector<SLSKtask> t2);
	int Connect(SupernodeHost host);
	int Connect(CString un, SupernodeHost host);
	int Connect(CString un, SLSKtask *t);
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	void reply(buffer &reply_buffer);
	void GetPeerIP(SLSKtask *task);
	bool IsConnected();
	bool m_logged_in;
	CTime m_last_parent_gotten;
	void IncrementDemandCounter(char* artist);
	int m_state_connecting;
	bool m_main_server;
	void SendCantConnect(SLSKtask *t);

	int Close(int error_code=0);
	int ListeningPort;
	//string ReturnRemoteIPAddressString(void);
	//void SendPing();
	//void Search(ProjectSupplyQuery& psq);
	void TimerHasFired();
	unsigned int ReturnHostIP();
	
	//string ReturnRemoteVendor();
	//void KeywordsUpdated();
	//void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

protected:
	void SocketDataSent(unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SocketDataReceived(char *data,unsigned int len);

private:

	vector<SLSKtask *> tasks;
	vector<buffer *> spoofs;
	
	//SLSKaccountInfo account_info;
//	buffer buf;
//	unsigned char m_buf_array[4096];
//	unsigned int m_buf_offset;
	int buffer_total_size;
	//CSLSKSpooferDlg dlg;
	ConnectionSockets* p_sockets;

	CString connection_ip;
	int port;

	bool m_first_query;
	static int m_peer_connections;
	static int m_parent_connections;
	static int m_server_connections;
	static int m_parent_connecting;
	static int m_peer_connecting;
	bool m_connected_parent;
	bool m_spoofed;
	//static int numofconnections;
	//FSTSessionState m_state;
	//FSTCipher* p_in_cipher;
	//FSTCipher* p_out_cipher;
	//unsigned int m_in_xinu;
	//unsigned int m_out_xinu;
	//bool m_received_header;
	int m_message_type;
	int m_message_length;
	//unsigned short m_search_id;
	CTime m_time_last_parent_sent;
	CTime m_time_sent_handshake;
	SLSKSocketStatusData m_status_data;
	//CryptoPP::AutoSeededRandomPool m_rng;
	bool m_ready_to_send_shares;
	int m_num_shares_sent;
//	vector<SLSKtask> st;
//	vector<SLSKtask> pt;
	bool send73, sendparent;
	char* names[100];
	
	CString m_user_name;
	CString m_password;
	int m_version, m_number_of_folders, m_number_of_files;




	/*void ResetData();
	unsigned int GetEncryptionType(unsigned int seed, unsigned int enc_type);
	bool GetIncomingSeedAndEncryptionType(unsigned char* data, int len);
	int DoHandshake(unsigned char * data);
	void GetRemoteServerName(unsigned char* data, UINT data_len);
	void SendMyNetworkName(void);
	void SessionGreetSupernode();*/
	unsigned int GetIntIPFromStringIP(const char* ip);
	void GetIPStringFromIPInt(int ip_int, char* ip_str);
	//static int loggingin;
	/*void SendSessionMessage(PacketBuffer& buf, FSTSessionMsg msg_type);
	void SendPong();
	void ReceivedQueryReply(PacketBuffer& buf, FSTSessionMsg msg_type);
	char * HashEncode64(unsigned char* hash);
	char * Base64Encode(const unsigned char *data, int src_len);
	void SendShare(void);
	int CheckQueryHitRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename,unsigned int file_size);
	int CheckRequiredKeywords(hash_set<ProjectSupplyQuery>::iterator &psq,const char *lfilename, UINT filesize);
	void ExtractKeywordsFromSupply(vector<string> *keywords,const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);

*/

	//int PacketStrlen(unsigned char *data, unsigned int data_len, unsigned char termbyte);
	//unsigned int GetDynamicInt(unsigned char* data, unsigned int& ptr_moved);

};