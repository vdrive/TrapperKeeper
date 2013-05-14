#pragma once

#include "dll.h"
//#include "../DllLoader/AppID.h"
//#include "SamplePlugInDlg.h"
#include "BTScraperDlg.h"
#include "BTSComInterface.h"
#include "btdatacollector.h"

class BTScraperMod :	public Dll
{
public:
	BTScraperMod(void);
	~BTScraperMod(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	HWND m_dlg_hwnd;
	void Collect();

	BTSComInterface* p_com_interface; //Com Interface
protected:
	BTScraperDlg m_dlg;
	BTDataCollector c;
private:
	bool b_started;
};
