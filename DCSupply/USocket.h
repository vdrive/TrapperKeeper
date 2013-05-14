// USocket.h

#ifndef USOCKET_H
#define USOCKET_H

#include "WSocket.h"

class USocket : virtual public WSocket
{
public:
	USocket();

	int SendTo(void *data,unsigned int len,unsigned int ip,unsigned short int port,unsigned int *num_sent=NULL);
	int ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short int *port,unsigned int *num_read);

protected:


};

#endif // USOCKET_H