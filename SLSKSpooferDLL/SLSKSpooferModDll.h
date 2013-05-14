#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include ".\SLSKSpooferDlg.h"
//#include "SLSKComInterface.h"
#include "ProjectManager.h"

class SLSKSpooferMod :	public Dll
{
public:
	static HINSTANCE m_hinst;	//stores the HINSTANCE of DllLoader.dll
	static AppID m_app_id;			//your application ID and info

	SLSKSpooferMod(void);
	~SLSKSpooferMod(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
protected:
	//ProjectManager m_project_manager;
	CSLSKSpooferDlg m_dlg;
	//static SLSKController m_app;
};
