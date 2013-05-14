#pragma once
#include "..\TKCom\Vector.h"
#include "..\TKCom\ThreadedObject.h"
#include "..\TKCom\Buffer2000.h"
#include "ReceivedMessage.h"

class CommunicationSystem : public ThreadedObject
{
public:
	Vector mv_received_messages;
	CommunicationSystem(void);
	~CommunicationSystem(void);

	CAsyncSocket m_rcv_socket;
	CAsyncSocket m_send_socket;
	UINT Run();
	void Shutdown(void);
	void Send(const char* dest, UINT header);
	ReceivedMessage* GetNextReceivedMessage(void);
	void PopMessages(void);
	int HasReceived(void);
	void SendPing(const char* dest_ip);
	void SendDllRequest(const char* dest_ip);
	void SendProcessRequest(const char* dest_ip);
};
