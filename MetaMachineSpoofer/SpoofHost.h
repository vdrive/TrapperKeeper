#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\ThreadedObject.h"
#include "Buffer2000.h"
//#include "..\tkcom\TKTCP.h"
#include <afxmt.h>
#include "..\tkcom\PeerSocket.h"
#include "SearchTask.h"



//#define LOGIPS
//This class encapsulates a connection to a server.
//This class also encapsulates an state based automaton.
class SpoofHost :	public Object, public SocketEventListener
{
private:
	class HashObject : public Object{
	public:
		byte m_hash[16];	
		HashObject(byte *hash){
			memcpy(m_hash,hash,16);
		}
	};

	byte ConvertCharToInt(char ch)
	{
		switch(ch){
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'A': return 10;
			case 'a': return 10;
			case 'B': return 11;
			case 'b': return 11;
			case 'C': return 12;
			case 'c': return 12;
			case 'D': return 13;
			case 'd': return 13;
			case 'E': return 14;
			case 'e': return 14;
			case 'F': return 15;
			case 'f': return 15;
		}
		return 0;
	}

	//state definitions
	//S_INIT = In the process of initing
	//S_INIT_RESPONSE = Waiting for a response from the server
	//S_READY = Ready to do a search
	//S_SEARCH = We are in the process of doing a search
	//S_SEARCH_RESPONSE = We are in the process of waiting for a search response
	//S_HASH_STROBE = We are in the process of probing hash sources from the server
	//S_HASH_STROBE_RESPONSE = We are in the process of waiting for a result from the server
	//S_PAUSE = basically a pause state for debugging
	enum SERVERSTATE {S_INIT=1 , S_INIT_RESPONSE , S_READY , S_READY_UPLOAD , S_READY_ASK_FOR_SERVERLIST ,S_GET_SOURCES};

	//string m_ip;
	//unsigned short m_port;  //the port that this instance of a donkey client is listening on
	unsigned short m_my_port;
	Timer m_age;
	//UINT m_server_handle;  //handle to the socket that this instance of a donkey client is listening on

	//TKTCPConnection m_tcp_connection;
	//TKTCPConnection m_firewall_connection;
	//UINT m_tcp_handle;  //a connection reference handle for use with MetaTCP
	//UINT m_tcp_firewall_check; //a connection reference to the connection we will receive to check if we have a firewall going.
	Buffer2000 m_firewall_buffer; //data from the firewall connection
	Buffer2000 m_tcp_buffer_pre; //data from the normal connection
	Buffer2000 m_tcp_buffer_post; //data from the normal connection
	//byte m_my_hash[16];  //a hash representing this client, so we report the same one each time

	int m_retry;  //times we have tried to do whatever it is we are trying to do.
	int m_uploaded_files;
	byte m_my_hash[16];
	string m_my_user_name;
	
	int m_search_count;  //denotes how many times we've requested more results.
	Buffer2000 m_send_buffer;
	int m_emule_version;

	CTime m_connected;
	SERVERSTATE m_state;  //current state of the machine
	Timer m_state_time;  //time we have been in the current state

	CCriticalSection m_firewall_buffer_lock;
	CCriticalSection m_tcp_buffer_lock;
	Timer m_infinite_loop_timer;  //a check

	Timer m_search_timer;
	UINT m_search_delay;


	bool b_new_normal_data;
	bool b_new_firewall_data;

	UINT respoof_time;
	Timer down_time;  //time we wish to remain down so that every rack isn't always connected to this server.  that would look wierd to see these huge contiguous ip ranges connected to you.
	Timer m_restart_delay;
	bool b_down;  //down or up;   flag signaling whether we wish to remain disconnected for now.

	UINT m_connection_delay;
	UINT m_file_limit;
	Timer m_created;
	Timer m_get_sources_timer;

	int m_dead_time;

	int m_get_sources_delay;

	SearchTask m_search_task;

	static CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen);

	//Timer m_get_sources_timer;

public:

	bool mb_active_server;  //a flag for this server to be active or not

	SpoofHost(const char* ip,unsigned short port, unsigned short my_port,byte *my_hash, string my_user_name,UINT connection_delay);
	~SpoofHost(void);
	bool IsServer(const char* ip);
	bool IsServer(const char* ip, unsigned short listen_port);
	
	void NewConnection(PeerSocket *src);
	//void NewData(TKTCPConnection &con, Buffer2000 &data);

	void Reset(bool b_change_state=true);
	//void LostConnection(TKTCPConnection &con);

private:
	void GenerateHash(const char* file_name,byte* buffer,bool b_total_random,int seed,int &file_size);
	void ProcessMainBuffer(void);
	void ProcessFirewallBuffer(void);
	void Init(void);
	void ChangeState(SERVERSTATE new_state);
	bool HasFullResponse(byte code);
	
	string m_ip;
	string m_bind_ip;
	unsigned short m_port;
	unsigned int m_expiration_time;
	PeerSocket *mp_main_connection;
	PeerSocket *mp_firewall_connection;
	Buffer2000 m_init_buffer;

public:
	void Update(void);

	inline const char* GetIP() { return m_ip.c_str(); }
	inline unsigned short GetPort() { return m_port; }
	inline unsigned short GetBasePort() { return m_my_port; }
	inline byte* GetMyHash() { return m_my_hash; }
	const char* GetMyUserName() { return m_my_user_name.c_str(); }
	const char* GetBindIP() { return m_bind_ip.c_str(); }

	void ClearUnwantedPackets(void);
public:
	CTime ConnectionTime(void);
	//bool IsThreadLockedUp(void);
	
	void UploadFiles(void);
private:
	//byte ConvertCharToInt(char ch);
public:
	void AskForServerInfo(void);
	bool IsConnected(void);
	int GetUploadedFiles(void);
	int GetState(void);
protected:
	void AddFileToUploadList(Buffer2000 &packet,byte* hash, int size , string name);
	void OnClose(CAsyncSocket* src);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnSend(CAsyncSocket* src);
	void OnConnect(CAsyncSocket* src);
public:
	int AddSpoofFiles(Buffer2000& packet, Vector &v_spoof_files, int cur_num_files, int file_limit);
	
	void CompressPacket(Buffer2000 & packet);
	void DecompressPacket(Buffer2000& packet);
	void GetSourcesForHash(void);
	void ProcessHashQueryResponse(Buffer2000 &data);
	void UpdateState(void);
	void CheckSendBuffer(void);
	void SetBindIP(const char* bind_ip);
	bool IsOld(void);
	void ProcessServerInfoResponse(Buffer2000 &data);
	void DoSearch(void);
	void ProcessSearchResults(Buffer2000& data);
};
