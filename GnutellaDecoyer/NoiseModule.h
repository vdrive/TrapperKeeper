// NoiseModule.h

#ifndef NOISE_MODULE_H
#define NOISE_MODULE_H

#include "NoiseModuleMessageWnd.h"

class NoiseManager;
class CCriticalSection;
class NoiseModuleThreadData;
class ConnectionData;
class FileSharingManager;
class NoiseModule
{
public:
	NoiseModule(CCriticalSection *connection_data_critical_section,vector<ConnectionData> *connection_data,
					NoiseManager *manager, FileSharingManager* fsm,bool sharing_enabled);
	~NoiseModule();
	void InitParent(NoiseManager *manager, FileSharingManager* fsm);
	void InitIndex(unsigned int index);

	unsigned int IdleSocketCount();
	void AcceptConnection(SOCKET hSocket);
	
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	void StatusReady(WPARAM wparam,LPARAM lparam);
	void DisableSharing();
	void EnableSharing();

	HWND m_hwnd;
	CCriticalSection *p_connection_data_critical_section;
	vector<ConnectionData> *p_connection_data;

	FileSharingManager* p_fs_manager;
private:
	NoiseManager *p_manager;
	NoiseModuleMessageWnd m_wnd;
	unsigned int m_index;
	CWinThread* m_thread;

	CCriticalSection *p_critical_section;
	NoiseModuleThreadData *p_thread_data;
	
	unsigned int m_idle_socket_count;
	bool m_sharing_enabled;

	vector<SOCKET> v_cached_incoming_connection_socket_handles;

};

#endif // NOISE_MODULE_H