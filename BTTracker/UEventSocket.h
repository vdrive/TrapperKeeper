// UEventSocket.h

#ifndef UEVENT_SOCKET_H
#define UEVENT_SOCKET_H

#include "USocket.h"
#include "WEventSocket.h"

class UEventSocket : public USocket, public WEventSocket
{
public:
	UEventSocket();
	~UEventSocket();

	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual void Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	// Virtual Functions
	virtual bool OnReceive(int error_code);
};

#endif // UEVENT_SOCKET_H
