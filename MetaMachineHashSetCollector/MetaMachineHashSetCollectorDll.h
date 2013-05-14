#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
//#include "SamplePlugInDlg.h"

class MetaMachineHashSetCollectorDll :	public Dll
{
public:
	MetaMachineHashSetCollectorDll(void);
	~MetaMachineHashSetCollectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:

	TCPReference *mp_tcp;  //the first and probably (but not necessarily) the last reference.
	MetaSystemReference *mp_system;  //the first and probably (but not necessarily) the last reference.
//	CSamplePlugInDlg m_dlg;
};
