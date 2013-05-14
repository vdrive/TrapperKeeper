#pragma once
#include "TEventSocket.h"
#include "PacketIO.h"

class ClientSockets;
class ClientSocketInterface : public TEventSocket
{
public:
	ClientSocketInterface(void);
	~ClientSocketInterface(void);

	void InitParent(ClientSockets *parent);

	void OnConnect(int error_code);
	bool OnReceive(int error_code);
	void OnClose(int error_code);
	int Close();

	void SendPacket(PacketWriter* packet);

	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	
	CTime m_connectTime;

private:
	bool m_closedCalled;
	ClientSockets *p_parent;
	bool m_bGotHeader;
	PacketWriter *p_packet;
	//char *p_username;
	//char *p_ip;
	int packetSize;
};
