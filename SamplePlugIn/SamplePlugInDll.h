#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include "SyncherDest.h"

class SamplePlugInDll :	public Dll
{
public:
	SamplePlugInDll(void);
	~SamplePlugInDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void SupplySynched(const char* source_ip);

private:
	CSamplePlugInDlg m_dlg;
	SyncherDest m_syncher;
};
