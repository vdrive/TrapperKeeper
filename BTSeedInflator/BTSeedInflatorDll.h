#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include "BTSeedInflatorCom.h"
#include "TorrentFileData.h"
#include "BTIP.h"
#include "BTIPList.h"
#include "TrackerConnection.h"
#include "InflationThread.h"


struct BTIPThreadData
{
	TorrentFileData m_data;	
	BTSeedInflatorCom * p_com;
	string source_name;
};

class BTSeedInflatorDll :	public Dll
{
public:
	BTSeedInflatorDll(void);
	~BTSeedInflatorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

private:
	//CSamplePlugInDlg m_dlg;
	BTSeedInflatorCom * p_com;
	vector<TorrentFileData> v_torrents;
	int m_num_connections;
	vector<InflationThread*> v_threads;
};
