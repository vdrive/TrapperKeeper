#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "AresSupernodeDlg.h"
#include "AresSupernodeSystem.h"

class AresSupernodeDll :	public Dll
{
public:
	AresSupernodeDll(void);
	~AresSupernodeDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	CAresSupernodeDlg m_dlg;
	AresSupernodeSystemRef *mp_system;
};
