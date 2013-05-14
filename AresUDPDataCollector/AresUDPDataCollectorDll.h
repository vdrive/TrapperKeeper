#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "AresUDPDataCollectorSystem.h"
//#include "SamplePlugInDlg.h"

class AresUDPDataCollectorDll :	public Dll
{
public:
	AresUDPDataCollectorDll(void);
	~AresUDPDataCollectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	SystemRef *mp_system;
//	CSamplePlugInDlg m_dlg;
};
