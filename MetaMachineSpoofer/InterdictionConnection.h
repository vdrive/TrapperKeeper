#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\Buffer2000.h"
//#include "TKTCP.h"
#include "..\tkcom\PeerSocket.h"

class InterdictionConnection : public Object , public SocketEventListener
{
	enum INTERDICTIONSTATE {INIT=1,FILE_REQUEST,FILE_STATUS_REQUEST,REQUEST_SLOT,REQUEST_PARTS};
	INTERDICTIONSTATE m_state;
	Timer m_idle_timer;
	string m_project;
	Buffer2000 m_init_buffer;
	//UINT m_tcp_handle;
	bool mb_awaiting_callback;
	UINT m_kill_after_idle_time;
	UINT m_amount_data_sent;
	UINT m_total_downloaded;;
	UINT m_amount_data_requested;
	UINT m_my_id;
	string m_ip;
	unsigned short m_port;
	UINT m_my_version;
	byte m_target_hash[16];
	string m_str_target_hash;
	//string m_ip;
	//unsigned int m_port;
	//TKTCPConnection m_tcp_connection;
	string m_my_user_name;
	unsigned short m_my_port;
	byte m_my_hash[16];
	vector <int> mv_has_parts;
	
	Buffer2000 m_buffer;
	bool mb_abort;
	bool mb_ready_to_download;
	PeerSocket *mp_tcp_socket;

	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnSend(CAsyncSocket* src);
	void OnConnect(CAsyncSocket* src);
	void OnClose(CAsyncSocket* src);

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

public:
	InterdictionConnection(const char *my_name,byte *my_hash, unsigned short my_port,const char *target_hash,const char* target_ip, unsigned short target_port,const char* project);
	~InterdictionConnection(void);

	void Update();
	void CheckBuffer();
	bool IsDead(void);
	void AskForFileStatus(void);
	void RequestParts(void);
	void CheckIfHasFile(void);
	void OnConnectFailed(CAsyncSocket* src);
	void AskForSlot(void);
	void Init(void);
	bool IsConnection(const char* ip);
	void ReleaseSlot(void);
	void Shutdown(void);
	void RespondHello(void);
	bool IsAwaitingCallBack(void);
	void ReestablishedConnection(CAsyncSocket *new_con);
	const char* GetIP(void);
	void RespondDontHaveFile(void);
};
