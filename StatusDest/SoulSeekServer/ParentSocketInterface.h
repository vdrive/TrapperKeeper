#pragma once
#include "TEventSocket.h"

class ConnectionSockets;
class ParentSocketInterface : public TEventSocket
{
public:
	ParentSocketInterface(void);
	~ParentSocketInterface(void);

	void InitParent(ConnectionSockets *parent);

	void OnConnect(int error_code);
	bool OnReceive(int error_code);
	void OnClose(int error_code);
	int Attach(SOCKET socket, char* theIP );
	int Close(void);

	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

private:
	ConnectionSockets *p_parent;
	bool m_bGotHeader;
	unsigned int m_socketToken;
	char* ip;
	bool connected;
};
