#pragma once
#include "tasyncsocket.h"

class InflationSocketManager;
class InflationSocket :
	public TAsyncSocket
{
public:
	InflationSocket(void);
	InflationSocket(string announce_url, string info_hash, string peer_id, int port, bool is_seed);
	~InflationSocket(void);

	void InitParent(InflationSocketManager * p_parent);

	void InitConnection(string announce_url, string info_hash, string peer_id, int port, bool is_seed, string key_id);
	
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	int Connect(char * host, unsigned short port);
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);

	void ParseHttpString(char * input, char * host, char * url, int * port);
	void SetFirstConnect();
	
	
//	char * GetPeerMessage(string announce_url, string info_hash, string peer_id, int port, bool is_seed);
	char * GetPeerMessage();
	char * GetKeyID();
	string ReturnPeerId();
	char * ConvertToHash(string hash);

	bool IsTimedout();

	string m_info_hash;
	string m_announce_url;
	string m_peer_id;
	string m_key_id;


private:
	
	InflationSocketManager * p_parent;

	string testleak;

	int m_port;
	bool m_is_seed;
	bool m_is_first_connect;
	
	CTime m_creation_time;
};
