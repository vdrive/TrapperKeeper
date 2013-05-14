#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTDatabaseInterface.h"
#include "NameServerInterface.h"
#include "BTTorrentSourceCom.h"
#include "BTTorrentSrcDlg.h"



class BTTorrentSourceDll :	public Dll
{
public:
	BTTorrentSourceDll(void);
	~BTTorrentSourceDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void SendTorrentData();
	void SendLowSeedTorrentData(vector<TorrentFileData> torrent_data);
	void SendRequestedTorrentData(TorrentFileData tdata, char * dest_name);
	void SendRequestedTorrentIPs(BTIPList vips, char * dest_name);
	void SendHashIDList();
	void SendStopTorrentID(int torrent_id);
	void SendStopTorrent(TorrentFileData tdata);

	void RemoveInflatedTorrent(unsigned int torrent_id);
	void PingAllDestinations();

	void AddNewLowSeedTorrent(TorrentFileData tdata);

	void TimerFired(UINT nIDEvent);
private:
	BTTorrentSrcDlg m_dlg;

	BTDatabaseInterface m_db;
	NameServerInterface m_name_server;
	BTTorrentSourceCom * p_com;

	vector<TorrentFileData>  m_torrent_data;
	vector<TorrentFileData>  m_low_seed_data;

	vector<string> v_send_list;
};
