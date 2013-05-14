#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SpoofingSystem.h"
//#include "MetaSpooferTCP.h"
#include "MetaMachineSpooferDlg.h"
#include "..\tksyncher\tksyncherinterface.h"

class MetaMachineSpooferDll :	public Dll
{
public:
	MetaMachineSpooferDll(void);
	~MetaMachineSpooferDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	

private:
	static CMetaMachineSpooferDlg sm_meta_dialog;
	TKSyncherInterface m_syncher_interface;
	//SpooferTCPReference *mp_tcp;  //the first and probably (but not necessarily) the last reference.
	MetaSpooferReference *mp_system;  //the first and probably (but not necessarily) the last reference.
	//CSamplePlugInDlg m_dlg;

public:
	static CMetaMachineSpooferDlg* GetDlg() {return &sm_meta_dialog;}
};
