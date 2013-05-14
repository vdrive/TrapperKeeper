// ListeningSocket.h

#ifndef LISTENING_SOCKET_H
#define LISTENING_SOCKET_H

#include "TAsyncSocket.h"

class NoiseManager;

class ListeningSocket : public TAsyncSocket
{
public:
	void InitParent(NoiseManager *manager);
	void OnAccept(int error_code);
	void OnClose(int error_code);

private:
	NoiseManager *p_manager;
};

#endif // LISTENING_SOCKET_H