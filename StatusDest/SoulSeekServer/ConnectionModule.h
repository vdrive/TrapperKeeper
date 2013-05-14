// ConnectionModule.h

#pragma once
#include <queue>
using namespace std;

#include "WSocket.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "SearchRequest.h"

class ConnectionManager;

class ConnectionModule
{
public:
	ConnectionModule();
	~ConnectionModule();
	void InitParent(ConnectionManager *manager);
	inline int GetIdleSocketCount(void) { return m_nSocketsAvail; }

	void AddParentConnection(SOCKET socket, char* ip);
	void LogMsg(char *pMsg);
	void ReceivedConnection(unsigned int token);
	void ProcessSearchRequest(SearchRequest* sr);
	void IncrementSocketsAvail();
	void DecrementSocketsAvail();

	// Messages
	void InitThreadData(WPARAM wparam,LPARAM lparam);

private:
	ConnectionManager *p_manager;
	ConnectionModuleMessageWnd m_wnd;
	ConnectionModuleThreadData m_thread_data;

	queue<SOCKET> m_qWaitingSockets;
	queue<char *> m_qWaitingIPs;
	int m_nSocketsAvail;
};