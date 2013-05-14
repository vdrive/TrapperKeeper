#pragma once
#include <afxsock.h>		// MFC socket extensions
#include "..\tkcom\threadedobject.h"
#include "KadUDPBootstrapper.h"
#include "KadUDPPublisher.h"

class KadUDPListener : public ThreadedObject
{
private:
	CAsyncSocket m_rcv_socket;
	KadUDPBootstrapper m_boot_strapper;
	KadUDPPublisher m_publisher;
public:
	KadUDPListener(void);
	~KadUDPListener(void);
	
	UINT Run(void);
	void StartKadUDPSystem(void);
	void StopKadUDPSystem(void);
	void ProcessBootstrapResponse(byte* p_data, UINT data_length);
	KadUDPPublisher* GetPublisher(void);
};
