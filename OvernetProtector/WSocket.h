// WSocket.h

#ifndef WSOCKET_H
#define WSOCKET_H

#include "winsock2.h"	// also add ws2_32.lib to project library includes

class WSocket
{
public:
	WSocket();
	~WSocket();

	static int Startup();
	static void Cleanup();
	static int GetLastError();
	int Listen();
	bool Attach(SOCKET hSocket);
	SOCKET Detach();

	bool IsSocket();

	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	virtual int Close();

protected:
	SOCKET Accept();
	SOCKET m_hSocket;
	int m_socket_type;	// SOCK_STREAM or SOCK_DGRAM
};

#endif // WSOCKET_H