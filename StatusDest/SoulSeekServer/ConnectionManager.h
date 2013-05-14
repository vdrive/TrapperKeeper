// ConnectionManager.h

#pragma once
#include "winsock2.h"
#include "SearchRequest.h"
#include <vector>
using namespace std;

class SoulSeekServerDll;
class ConnectionModule;

class ConnectionManager
{
public:
	ConnectionManager();
	~ConnectionManager();
	void InitParent(SoulSeekServerDll *dlg);
	void AddParentConnection(SOCKET sockets, char* ip);
	void LogMsg(char *pMsg);
	void ReceivedConnection(unsigned int token);
	void ProcessSearchRequest(SearchRequest* sr);
	void CloseAllModules();
	void UpdateParentSockets(int iCParents,int iTParents,int iPMods);

private:
	void AddModule();

private:
	SoulSeekServerDll *p_parent;
	vector<ConnectionModule *> v_mods;
};