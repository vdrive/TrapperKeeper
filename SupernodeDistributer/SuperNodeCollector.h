#pragma once
#include "IPAddress.h"

class SuperNodeCollector
{
public:
	SuperNodeCollector(void);
	~SuperNodeCollector(void);
	void ImportSupernodeList(vector<IPAddress>& list);
	void KillAllKazaaProcesses();
	void LaunchKazaa(const char* kazaa_path, const char* run_in);

private:
public:
	void DisconnectKazaa(void);
	void ConnectKazaa(void);
	void ReconnectKazaa(void);
};
