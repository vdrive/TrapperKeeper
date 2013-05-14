#pragma once
#include <afxsock.h>		// MFC socket extensions
#include "..\tkcom\threadedobject.h"
#include "KadContact.h"

class KadUDPBootstrapper : public ThreadedObject
{
private:
	CAsyncSocket *mp_socket;

public:
	KadUDPBootstrapper(void);
	~KadUDPBootstrapper(void);
	UINT Run(void);
	void SetSocket(CAsyncSocket* p_socket);
	void BootStrap(KadContact* p_contact,UINT my_ip,byte *my_hash);
};
