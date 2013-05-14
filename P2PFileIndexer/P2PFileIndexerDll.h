#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "P2PDialog.h"
//#include "SamplePlugInDlg.h"

class P2PFileIndexerDll :	public Dll
{
public:
	P2PFileIndexerDll(void);
	~P2PFileIndexerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	P2PDialog m_dlg;
	//CSamplePlugInDlg m_dlg;
};
