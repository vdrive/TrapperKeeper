#pragma once
#include "Dll.h"
#include "../DllLoader/AppID.h"
#include "DCSupplyDlg.h"
#include "ProjectManager.h"
#include "ProjectKeywordsVector.h"

class DCSupplyDll :	public Dll
{
public:
	DCSupplyDll(void);
	~DCSupplyDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	DCSupplyDlg m_dlg;
	ProjectManager m_project_manager;	//project manager that keeps all the projects up to date
};
