#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "KazaaDBManagerDialog.h"
#include "..\tksyncher\tksyncherinterface.h"

class KazaaDBManagerDll :	public Dll, public TKSyncherInterface
{
public:
	KazaaDBManagerDll(void);
	~KazaaDBManagerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	KazaaDBManagerDialog m_dlg;
	void MapFinishedChanging(const char *source_ip);
};
