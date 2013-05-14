#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
//#include "SamplePlugInDlg.h"
#include "AresDataCollectorSystem.h"
#include "AresDataCollectorDialog.h"

class AresDataCollectorDll :	public Dll
{
public:
	AresDataCollectorDll(void);
	~AresDataCollectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	AresDataCollectorSystemReference *mp_system;
	CAresDataCollectorDialog m_dlg;
};
