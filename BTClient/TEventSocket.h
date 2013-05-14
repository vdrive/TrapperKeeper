// TEventSocket.h

#ifndef TEVENT_SOCKET_H
#define TEVENT_SOCKET_H

#include "TSocket.h"
#include "WEventSocket.h"

class TEventSocket : public TSocket, public WEventSocket
{
public:
	virtual int Create(unsigned short int port=0,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual void Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	virtual int Listen();

	virtual void OnSend(int error_code);
};

#endif // TEVENT_SOCKET_H