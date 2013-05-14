// UAsyncSocket.h

#ifndef UASYNC_SOCKET_H
#define UASYNC_SOCKET_H

#include "USocket.h"
#include "WAsyncSocket.h"

class UAsyncSocket : public USocket, public WAsyncSocket
{
public:
	UAsyncSocket();
	~UAsyncSocket();

	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual void Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	// Virtual Functions
	virtual bool OnReceive(int error_code);
};

#endif // UASYNC_SOCKET_H