#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "GnutellaFileDownloaderDlg.h"
//#include "SyncherDest.h"

class GnutellaFileDownloaderDll :	public Dll
{
public:
	GnutellaFileDownloaderDll(void);
	~GnutellaFileDownloaderDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void SupplySynched(const char* source_ip);

private:
	GnutellaFileDownloaderDlg m_dlg;
//	SyncherDest m_syncher;
};
