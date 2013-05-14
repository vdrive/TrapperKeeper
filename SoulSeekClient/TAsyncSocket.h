// TAsyncSocket.h

#ifndef TASYNC_SOCKET_H
#define TASYNC_SOCKET_H

#include "TSocket.h"
#include "WAsyncSocket.h"

class TAsyncSocket : public TSocket, public WAsyncSocket
{
public:
	virtual int Create(unsigned short int port=0,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual int Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	virtual int Listen();

	virtual void OnSend(int error_code);
};

#endif // TASYNC_SOCKET_H