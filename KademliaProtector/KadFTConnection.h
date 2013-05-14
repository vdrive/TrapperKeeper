#pragma once
#include "Buffer2000.h"
//#include "MetaSpooferTCP.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\object.h"
#include "..\tkcom\Timer.h"
#include "zlib.h"
#include "SocketEventListener.h"
#include "PeerSocket.h"

class KadFTConnection :  public SocketEventListener, public Object
{
protected:

	CCriticalSection m_lock;

	UINT m_emule_version;
	unsigned short m_listening_port;
	bool mb_compressed_poisoner;
	bool mb_no_swarms_allowed;
	bool mb_overnet;
	PeerSocket *mp_tcp_socket;

	Timer m_created;  //an instance of when this was first created
	Timer m_idle;  //an instance of when this was first created
	Timer m_last_feed;
	Buffer2000 m_buffer;
	string m_my_user_name;

	bool mb_overnet_special_status;

	bool mb_eserver;
	bool mb_can_send;
	int m_feed_pos;
	Buffer2000 m_feed_packet1,m_feed_packet2,m_feed_packet3;
	bool mb_feed;
	bool mb_abort;

	bool mb_emule_compatible;

	byte m_my_hash[16];
	UINT m_start1;
	UINT m_start2;
	UINT m_start3;
	UINT m_end1;
	UINT m_end2;
	UINT m_end3;
	UINT m_send_queue_index;
	UINT m_send_delay;
	string m_ip;

	string m_my_ip;
	unsigned short m_port;
	unsigned short m_client_port;

	vector <string> v_send_queue_file_names;
	vector <int> v_send_queue_start_offset;
	vector <int> v_send_queue_end_offset;

	bool mb_needs_send;
	Timer m_last_send;  //for throttling

	Vector mv_send_buffers;

protected:
	void CheckBuffer(void);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnSend(CAsyncSocket* src);
	void OnConnect(CAsyncSocket* src);
	void OnClose(CAsyncSocket* src);
	void Feed(void);
	void GrantSlot(void);
	void RespondFileStatus(byte *hash);
	void RespondFileRequest(byte *hash);
	void RespondSendParts(byte *hash);
	void ReleaseSlot(void);
	void RespondHello(void);
	void RespondHashParts(byte *hash);
	void CompressData(byte* data, UINT data_len, Buffer2000 &compressed_data);
	void CheckSendBuffer(void);
	void RespondDontHaveFile(byte *hash);
	void RespondOvernet0x66(byte *hash);
	void ProcessEmulePacket(Buffer2000& packet);
	void SendData(const byte* p_data, UINT data_length);	

public:
	KadFTConnection(CAsyncSocket* socket,unsigned short listening_port, byte *my_hash,string my_user_name);
	~KadFTConnection(void);
	void Update(void);

	void Shutdown(void);
	bool IsOld(void);

	unsigned short GetListeningPort(void);

private:
	const char* IsNoSwarmAddress(const char* address);

public:
	const char* GetIP(void);
};
