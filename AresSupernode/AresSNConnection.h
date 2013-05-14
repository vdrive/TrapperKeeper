#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Buffer2000.h"
#include "..\tkcom\HashObject.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\tkcom\LinkedList.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\PeerSocket.h"
#include "..\tkcom\SocketEventListener.h"

class AresSNConnection : public SocketEventListener, public Object
{

private:
	UINT m_num_sn_peers;
	UINT m_num_leaf_peers;

	bool mb_clean;

	Vector mv_files_shared;
	Vector mv_processor_job;
	//Vector mv_shares_to_upload;

	CTime m_last_file_upload;

	unsigned short int m_fss;	// four-shit-short
	unsigned char m_fsb;		// four-shit-byte

	UINT m_id;
	//Vector mv_hosts;

	bool mb_fully_connected;
	bool mb_connecting;
	bool mb_has_connected;
	bool mb_connected;
	bool mb_marked_for_death;
	bool mb_name_sent;
	bool mb_0x38_client;
	bool mb_first_ping_sent;
	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;
	Timer m_last_ping_sent;

//	byte m_spoof_hashes[NUM_SPOOF_HASHES][20];  //precalculated set of hashes to spoof
//	int m_num_results[NUM_SPOOF_HASHES];  //precalculated number of results for each hash
//	int m_total_results;


	UINT m_con_handle;  //only used if we are using event based sockets
#ifndef TKSOCKETSYSTEM
	PeerSocket *mp_ps_con;
#endif


	UINT m_build_number;
	Timer m_last_purge;
	//PeerSocket *mp_ps_con;

	bool mb_handled_compressed;  //debug variable
	unsigned int m_remote_ip;
	string m_str_ip;
	unsigned short m_my_port;
	unsigned short m_port;
	string m_status;

	unsigned int m_state;

	Buffer2000 m_receive_buffer;

	bool mb_dead;

	//a vector of events
	LinkedList m_event_list;
	int m_num_shares;

	void ProcessPacket(AresPacket& packet);

	char m_user_name[12];

	void Init(const char *host_ip,unsigned short port);	

protected:



public:
	AresSNConnection(const char* host_ip,int port,unsigned short my_port);
	AresSNConnection(AresHost* host,unsigned short my_port);
	~AresSNConnection(void);

//#ifdef TKSOCKETSYSTEM
	/*
	void OnClose();
	void OnSend();
	void OnReceive(byte *data,UINT length);
	void OnConnectFailed();
	void OnConnect();
	*/
//#else
	//**************
	void OnClose(CAsyncSocket* src);
	void OnSend(CAsyncSocket* src);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnConnect(CAsyncSocket* src);
	void OnConnectFailed(CAsyncSocket* src);
	//**************
//#endif

	inline const char* GetStatus(){return m_status.c_str();}
	inline const char* GetIP(){return m_str_ip.c_str();}
	inline unsigned short GetPort(){return m_port;}
	inline CTimeSpan GetAliveTime(){return CTime::GetCurrentTime()-m_create_time;}

	inline bool IsClean(){return mb_clean;}

	void Update(void);
	void Connect(void);
	bool IsConnected(void);
	void Kill(void);
	CString GetFormattedAgeString(void);
	
	LinkedList* GetEventList(void);

	inline bool IsDead(void){
		return mb_dead;
	}

	inline bool IsDying(void){
		return mb_marked_for_death || mb_dead;
	}

	inline UINT GetID(void)
	{
		return m_id;
	}

	void CleanUp(void);
	void RecordReceiveEvent(AresPacket* packet);
	void RecordSendEvent(AresPacket* packet);
	
private:
	void PurgeEvents(void);

protected:
	void SendData(byte* data, UINT length);
public:
	UINT GetConHandle(void);
	bool IsFullyConnected(void);
	void StartMessage0x62(byte* eighty);
	void FinishMessage0x62(void);
	void ReceivedQuery(AresPacket& packet);
	void SendMessage0x68(void);
	void InitPeerCounts(void);
	void SendMessage0x67(void);
	void HandleCompressedPacket(AresPacket& packet);
	void SendMessage0x03(void);
	void RespondToTestQuery(int search_type,unsigned short query_id, unsigned short leaf_id, const char* query_str);
	void ForwardQuery(Buffer2000* query);
	void TestQuery(int search_type,unsigned short query_id, unsigned short leaf_id,const char* query_str, vector <string>&v_keywords);
};
