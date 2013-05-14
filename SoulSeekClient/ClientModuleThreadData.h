// ConnectionModuleThreadData.h

#pragma once
#include <queue>
using namespace std;

#include "PacketIO.h"
#include "WSocket.h"

//class CCriticalSection;

class ClientModuleThreadData
{
public:
	ClientModuleThreadData();
	~ClientModuleThreadData();
	void Clear();

	//CCriticalSection *p_critical_section;
	WSAEVENT m_reserved_events[4];

	//HANDLE m_reserved_events[4];

	queue<PacketWriter *>* p_qPacketsToSend;
};