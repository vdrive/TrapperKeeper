#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "SoulSeekClientDlg.h"
#include "ConnectionManager.h"
#include "ParentComInterface.h"
#include "PacketAssembler.h"

class SoulSeekClientDll :	public Dll
{
public:
	SoulSeekClientDll(void);
	~SoulSeekClientDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void WriteToTextFile(string s);

	//Objects
	SoulSeekClientDlg m_dlg;
	ConnectionManager m_connectionManager;
	ParentComInterface* p_parentCom;
	PacketAssembler m_pa;
	

protected:
	//SyncherDest m_syncher;
};
