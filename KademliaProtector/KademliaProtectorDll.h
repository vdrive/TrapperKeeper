#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "KademliaProtectorDialog.h"
#include "KademliaProtectorSystem.h"

class KademliaProtectorDll :	public Dll
{
public:
	KademliaProtectorDll(void);
	~KademliaProtectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	KademliaProtectorDialog m_dlg;
	KademliaProtectorSystemReference *mp_system;
};
