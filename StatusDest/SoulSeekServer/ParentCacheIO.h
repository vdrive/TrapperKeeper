#pragma once
#include "Client.h"
#include <vector>
using namespace std;

class SoulSeekServerDll;

class ParentCacheIO
{
public:
	ParentCacheIO(void);
	~ParentCacheIO(void);
	void InitParent(SoulSeekServerDll *parent);
	void Populate(void);
	unsigned int AddParent(char* username, char* ip, int port, int socketStatus);
	void ReceivedConnection(unsigned int token);
	void FailedConnection(unsigned int token);
	int RemoveFailedConnections(void); // Returns the number of failed connections
	int RemoveBadServers(void); // Removes old servers that we couldnt connect to several times
	unsigned int GetNextToken(void);
	void WriteFile(void);
	void ConnectToParents(void);

private:
	SoulSeekServerDll *p_parent;
	char fileLocation[MAX_PATH+1];
	int maxServers;
	unsigned int m_nextToken;
	vector<Client *> parents;
};
