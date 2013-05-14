#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include "BTIp.h"
#include "BTIPList.h"

#include "BTIPGathererCom.h"
#include "TrackerConnection.h"
#include "TorrentFileData.h"
#include "BTIPList.h"

struct BTIPThreadData
{
	TorrentFileData m_data;	
	BTIPGathererCom * p_com;
	string source_name;
};

class BTIPGathererDll :	public Dll
{
public:
	BTIPGathererDll(void);
	~BTIPGathererDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	BTIPList GetPeerList(TorrentFileData tdata);
	void SendIPData(char * source_name, BTIPList iplist);
	void SendSourceIPData(BTIPList iplist);

private:
	//CSamplePlugInDlg m_dlg;
	BTIPGathererCom * p_com;
	TrackerConnection m_tracker;

	string m_source_name;

};
