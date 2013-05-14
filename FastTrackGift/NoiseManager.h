// NoiseManager.h

#ifndef NOISE_MANAGER_H
#define NOISE_MANAGER_H

#include "Manager.h"
#include "ListeningSocket.h"
#include "NoiseModuleThreadStatusData.h"
//#include "ConnectionData.h"
#include <afxmt.h>	// for CCriticalSection

class NoiseModule;
class FileSharingManager;
class NoiseManager : public Manager
{
public:
	NoiseManager();
	~NoiseManager();
	void KillModules();
	void StartServer();

	void AcceptedConnection(SOCKET hSocket);

	void ReportStatus(vector<NoiseModuleThreadStatusData> &status);
	
	unsigned int GetModCount();

	void ClearIdleModules();

	UINT m_total_num_received_connections;
	void OnCloseListeningPort(int error_code);
	void SetFileSharingManager(FileSharingManager* manager);

private:
	vector<NoiseModule *> v_mods;
	ListeningSocket m_listening_socket;

	unsigned int m_listening_port;
	unsigned int m_listening_port_retry_count;

	CCriticalSection m_connection_data_critical_section;
//	vector<ConnectionData> v_connection_data;
	FileSharingManager* p_file_sharing_manager;
};

#endif // NOISE_MANAGER_H