#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "DCScannerDlg.h"
#include "projectmanager.h"

class DCScannerDll :	public Dll
{
public:
	DCScannerDll(void);
	~DCScannerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	DCScannerDlg m_dlg;
	ProjectManager m_project_manager;
};
