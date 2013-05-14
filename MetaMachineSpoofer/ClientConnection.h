#pragma once
#include "Buffer2000.h"
//#include "MetaSpooferTCP.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Timer.h"
#include "zlib.h"
#include "SocketEventListener.h"
#include "PeerSocket.h"

class ClientConnection : public ThreadedObject, public SocketEventListener
{
	class SendQueue : public Object
	{
		UINT tcp_handle;
		vector <string> v_file_names;
	};
protected:
	CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
	{
		CString base16_buff;
		static byte base16_alphabet[] = "0123456789ABCDEF";

		for(unsigned int i = 0; i < bufLen; i++) {
			base16_buff += base16_alphabet[buffer[i] >> 4];
			base16_buff += base16_alphabet[buffer[i] & 0xf];
		}

		return base16_buff;
	}

	unsigned short m_listening_port;
	bool mb_compressed_poisoner;
	bool mb_no_swarms_allowed;
	PeerSocket *mp_tcp_socket;
	//UINT m_tcp_handle;
	Timer m_created;  //an instance of when this was first created
	Timer m_idle;  //an instance of when this was first created
	Timer m_last_feed;
	Buffer2000 m_buffer;
	string m_my_user_name;

	bool mb_eserver;
	bool mb_can_send;
	bool mb_is_declared_source;  //a flag to specify whether we've reported this ip as a new source for the IP they are trying to get.
	int m_feed_pos;
	Buffer2000 m_feed_packet1,m_feed_packet2,m_feed_packet3;
	bool mb_feed;
	//int m_my_port;
	enum CLIENTSTATE {CLIENT_START=1,CLIENT_HELLO,CLIENT_HELLO_WAIT,CLIENT_FILE_REQUEST,CLIENT_FILE_REQUEST_WAIT,CLIENT_FILE_STATUS,CLIENT_FILE_STATUS_WAIT,CLIENT_HASH_PARTS,CLIENT_HASH_PARTS_WAIT,CLIENT_SEND_PARTS,CLIENT_SEND_PARTS_WAIT};
	CLIENTSTATE m_state;
	bool mb_abort;
	//string m_ip;
	//Buffer2000 m_send_buffer;

	bool mb_emule_compatible;

	byte m_desired_hash[16];
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
	unsigned short m_port;
	unsigned short m_client_port;

	int m_throttle_speed;

	Vector mv_send_buffers;

	vector <string> v_send_queue_file_names;
	vector <int> v_send_queue_start_offset;
	vector <int> v_send_queue_end_offset;

	bool mb_needs_send;
	Timer m_last_send;  //for throttling

public:
	ClientConnection(CAsyncSocket* socket,unsigned short listening_port, byte *my_hash,string my_user_name);
	~ClientConnection(void);

protected:
	void CheckBuffer(void);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnSend(CAsyncSocket* src);
	void OnConnect(CAsyncSocket* src);
	void OnClose(CAsyncSocket* src);
	UINT Run();
public:
	void Update(void);
	void RespondHello(void);
	void RespondHashParts(void);
	//bool IsConnection(TKTCPConnection &con);
	void Shutdown(void);
	bool IsOld(void);
private:
	void ClearDesiredHash(void);
	//void ProcessSendQueue(void);

public:
	void RespondFileStatus(void);
	void RespondFileRequest(void);
	void RespondSendParts(void);
	void ReleaseSlot(void);
	//void AddToSendQueue(byte * buffer, UINT buffer_length,int start_offset, int end_offset);
	//void CleanSendQueue(void);
	
	//void CompressPacket(Buffer2000& packet);
	void CompressData(byte* data, UINT data_len, Buffer2000 &compressed_data);
	void Feed(void);
	void GrantSlot(void);
	unsigned short GetListeningPort(void);
	void CheckSendBuffer(void);
	void RespondDontHaveFile(void);
private:
	const char* IsNoSwarmAddress(const char* address);
	void ProcessEmulePacket(Buffer2000& packet);
public:
	
private:
	void SendData(const byte* p_data, UINT data_length);
};
