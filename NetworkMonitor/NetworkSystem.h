#pragma once
#include "..\TKCom\Vector.h"
#include "..\TKCom\ThreadedObject.h"
#include "CommunicationSystem.h"
#include "Rack.h"
#include <afxmt.h>

class NetworkSystem : public ThreadedObject
{
public:
	//Vector mv_racks;
	Vector mv_networks;
	Vector mv_racks;
	Vector mv_processes;
	Vector mv_dlls;
	NetworkSystem(void);
	~NetworkSystem(void);

	CommunicationSystem m_com_system;
	void Shutdown(void);

	UINT Run();

	float m_percent_alive;
	float m_percent_not_crashed;

	int m_alive;
	int m_dead;
	int m_crashed;



private:
	void ScanInNetworks(void);
	vector <CTime> mv_on_time;
	
	CCriticalSection m_on_time_vector_lock;

	bool mb_processes_changed;
	bool mb_dlls_changed;

public:
	void Init(void);
private:
	void ScanInRacks(void);
	void OnReceive(void);

public:
	void EnumerateOnTime(int range,vector <bool> &vb_on);
	void MarkOnTime(void);
	void SaveHistory(void);
	void LoadHistory(void);
	Rack* GetRack(const char* ip);
	
	void FillTreeWithProcessInfo(CTreeCtrl& tree);
	void FillTreeWithDllInfo(CTreeCtrl& tree);
	Network* GetNetwork(const char* name);
	void RemoteControlRack(const char* ip);
	void OnRestartDeadRacks(void);
	void OnRestartSelectedNetwork(Network* network);
	void OnRemoteCrashedSelectedNetwork(Network* network);
	void OnRemoteDeadSelectedNetwork(Network* network);
	void OnRestartDeadSelectedNetwork(Network* network);
	void OnRestartCrashedSelectedNetwork(Network* network);
	void ExportDeadRacks(void);
	void WriteToFile(HANDLE file, const char* str);
	void ExportCrashedRacks(void);
};
