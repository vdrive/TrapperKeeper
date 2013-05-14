#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "NetworkMonitorDialog.h"
#include "NetworkSystem.h"

class NetworkMonitorDll :	public Dll
{
public:
	NetworkMonitorDll(void);
	~NetworkMonitorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	CNetworkMonitorDialog m_dlg;
	NetworkSystem m_network_system;
};
