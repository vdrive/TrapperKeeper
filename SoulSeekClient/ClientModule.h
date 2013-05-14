// ConnectionModule.h

#pragma once
#include <queue>
using namespace std;

#include "WSocket.h"
#include "ClientModuleMessageWnd.h"
#include "ClientModuleThreadData.h"
#include "PacketIO.h"

class ConnectionManager;
class CCriticalSection;

class ClientModule
{
public:
	ClientModule(int id);
	~ClientModule();
	void InitParent(ConnectionManager *manager);
	inline int GetIdleSocketCount(void) { 
		if(m_shuttingdown) return 0;
		return m_nSocketsAvail;
	}

	void SetEvent(int event);
	void OpenAnotherConnection();
	bool SendPacket(PacketWriter* packet);
	void CloseConnection();
	void Shutdown();
	void DeleteThread();
	void LogMsg(char *pMsg);
	void IncrementSocketsAvail();
	void DecrementSocketsAvail();
	void WriteToFile(char* s);

	// Messages
	void InitThreadData(WPARAM wparam,LPARAM lparam);

private:
	char m_id[10];
	ConnectionManager *p_manager;
	//ClientModuleMessageWnd m_wnd;
	//ClientModuleThreadData m_thread_data;

	CCriticalSection *p_critical_section;
	ClientModuleThreadData *p_thread_data;
	CWinThread *p_thread;

	ClientModuleMessageWnd m_wnd;

	bool m_shuttingdown;

	queue<PacketWriter *> m_qWaitingPacketsToSend;
	int m_nSocketsAvail;
};