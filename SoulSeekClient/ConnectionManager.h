// ConnectionManager.h

#pragma once
#include "PacketIO.h"
#include "winsock2.h"
#include <vector>
using namespace std;

class SoulSeekClientDll;
class ClientModule;

class ConnectionManager
{
public:
	ConnectionManager();
	~ConnectionManager();
	void InitParent(SoulSeekClientDll *dlg);
	void LogMsg(char *pMsg);
	void CloseClientConnection();
	void OpenAnotherClientConnection();
	void RoutePacket(PacketWriter* packet);
	void CloseAllModules();
	void DeleteClient(ClientModule* cmod);
	void WriteToFile(string s);

private:
	void AddClientModule(int id);
	int m_socketNumberCounter;

private:
	SoulSeekClientDll *p_parent;
	vector<ClientModule *> v_clientMods;
};