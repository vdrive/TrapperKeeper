#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"

class AresDCInterdictionServer : private ThreadedObject
{
	private:

	CAsyncSocket m_rcv_socket;
	CAsyncSocket m_send_socket;

public:
	AresDCInterdictionServer(void);
	~AresDCInterdictionServer(void);

	UINT Run();

	void StopSystem(void);
	void StartSystem(void);
	void RespondTargets(const char* ip);
};
