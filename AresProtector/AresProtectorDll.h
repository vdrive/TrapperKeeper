#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "AresDialog.h"
#include "AresProtectionSystem.h"

class AresProtectorDll :	public Dll
{
public:
	AresProtectorDll(void);
	~AresProtectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	AresProtectionSystemReference *mp_ref;
	CAresDialog m_dlg;
};
