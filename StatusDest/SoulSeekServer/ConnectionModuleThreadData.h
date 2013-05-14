// ConnectionModuleThreadData.h

#pragma once
#include <queue>
using namespace std;

#include "WSocket.h"

class CCriticalSection;

class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();
	~ConnectionModuleThreadData();
	void Clear();

	CCriticalSection *p_critical_section;
	WSAEVENT m_reserved_events[4];

	queue<SOCKET> *m_pqNewParentConnections;
	queue<char *> *m_pqNewParentIPs;
};