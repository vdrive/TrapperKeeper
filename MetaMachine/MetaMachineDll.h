#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "MetaMachineDlg.h"
#include "MetaTCP.h"
#include "MetaSystem.h"
#include "MetaCom.h"

class MetaMachineDll :	public Dll
{
public:
	MetaMachineDll(void);
	~MetaMachineDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	MetaCom m_com;
	static CMetaMachineDlg sm_meta_dialog;
	TCPReference *mp_tcp;  //the first and probably (but not necessarily) the last reference.
	MetaSystemReference *mp_system;  //the first and probably (but not necessarily) the last reference.
public:
	static CMetaMachineDlg* GetDlg(void);
};
