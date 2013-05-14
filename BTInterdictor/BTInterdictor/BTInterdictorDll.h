#pragma once
#include "dll.h"
#include "AppID.h"
#include "BTinterdictorDlg.h"
//#include "SyncherDest.h"

class BTinterdictorDll :	public Dll
{
public:
	BTinterdictorDll(void);
	~BTinterdictorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	//void SupplySynched(const char* source_ip);

private:
	CBTinterdictorDlg m_dlg;
	//SyncherDest m_syncher;
};
