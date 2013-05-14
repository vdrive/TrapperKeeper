#pragma once
#include "c:\cvs\mdproj\trapperkeeper\sampleplugin\dll.h"
#include "AutoUpdaterSourceDlg.h"
#include "AutoUpdaterSourceCom.h"


class AutoUpdaterSourceDll :
	public Dll
{
public:
	AutoUpdaterSourceDll(void);
	~AutoUpdaterSourceDll(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void PopulateCurrentDllListBox();

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	CAutoUpdaterSourceDlg m_dlg;
private:
	
	AutoUpdaterSourceCom m_AutoUpdaterSourceCom;
};
