#pragma once
#include "TEventSocket.h"
#include "FastTrackCrypt.h"
#include "PacketBuffer.h"
#include "SupernodeHost.h"
#include "FastTrackSocketStatusData.h"
#include "FastTrackMeta.h"

#define MAX_MSG_LENGTH				1024*1024*10 //10 MB
#define MIN_USERS_ON_CONNECT		1000000 //we want to connect to at least 1 million users network
#define FST_NETWORK_NAME "KaZaA"
#define FST_MAX_SEARCH_RESULTS		200

/* session states */
typedef enum
{
	SessNew,
	SessConnecting,
	SessHandshaking,
	SessWaitingNetName,
	SessEstablished,
	SessDisconnected
} FSTSessionState;

/* session messages sent to callback */
typedef enum
{
	/* our own stuff */
	SessMsgConnected = 0x01FF,		/* tcp connection established */
	SessMsgEstablished = 0x02FF,	/* encryption initialized and we received
									 * a valid network name
									 */
	SessMsgDisconnected = 0x03FF,	/* tcp connection closed */
	
	/* FastTrack messages */
	SessMsgNodeList = 0x00,
	SessMsgUserInfo = 0x02,
	SessMsgQuery = 0x06,
	SessMsgQueryReply = 0x07,
	SessMsgQueryEnd = 0x08,
	SessMsgNetworkStats = 0x09,
	SessMsgNetworkName = 0x1d,
	SessMsgPushRequest = 0x0d,
	SessMsgExternalIp = 0x2c,      /* supernode tells us our outside ip */
	SessMsgShareFile = 0x22,
	SessMsgUnshareFile = 0x05,
	SessMsgProtocolVersion = 0x26  /* we think this is the protocol version */ 

} FSTSessionMsg;

class ConnectionSockets;
class FastTrackSocket :	public TEventSocket
{
public:
	FastTrackSocket(void);
	~FastTrackSocket(void);
	void InitParent(ConnectionSockets* parent);
	int Connect(SupernodeHost host);
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	bool IsConnected();
	int Close();
	void SendPing();
	void TimerHasFired();
	unsigned int ReturnHostIP();
	string ReturnRemoteVendor();

protected:
	void SocketDataSent(unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SocketDataReceived(char *data,unsigned int len);

private:

	ConnectionSockets* p_sockets;
	FSTSessionState m_state;
	FSTCipher* p_in_cipher;
	FSTCipher* p_out_cipher;
	unsigned int m_in_xinu;
	unsigned int m_out_xinu;
	bool m_received_header;
	int m_message_type;
	int m_message_length;
	//unsigned short m_search_id;
	CTime m_time_last_ping_sent;
	FastTrackSocketStatusData m_status_data;
	//CryptoPP::AutoSeededRandomPool m_rng;
	bool m_ready_to_send_shares;
	int m_num_shares_sent;

	void ResetData();
	unsigned int GetEncryptionType(unsigned int seed, unsigned int enc_type);
	bool GetIncomingSeedAndEncryptionType(unsigned char* data, int len);
	int DoHandshake(unsigned char * data);
	void GetRemoteServerName(unsigned char* data, UINT data_len);
	void SendMyNetworkName(void);
	void SessionGreetSupernode();
	unsigned int GetIntIPFromStringIP(const char* ip);
	void GetIPStringFromIPInt(int ip_int, char* ip_str);
	void SendSessionMessage(PacketBuffer& buf, FSTSessionMsg msg_type);
	void SendPong();
	char * HashEncode64(unsigned char* hash);
	char * Base64Encode(const unsigned char *data, int src_len);
	void SendShare(void);

	//int PacketStrlen(unsigned char *data, unsigned int data_len, unsigned char termbyte);
	//unsigned int GetDynamicInt(unsigned char* data, unsigned int& ptr_moved);

};