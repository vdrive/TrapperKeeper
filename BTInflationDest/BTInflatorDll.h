#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include "BTInflatorCom.h"
#include "torrent.h"
#include "InflationSocketManager.h"
#include "InflationSocket.h"
#include "BTInflationDlg.h"



class BTInflatorDll :	public Dll
{
public:
	BTInflatorDll(void);
	~BTInflatorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void TimerFired(UINT nIDEvent);

	void ServiceTorrent(Torrent * the_torrent);
	//void ServiceNewTorrent(Torrent * the_torrent);
	void TorrentFailure(string info_hash, string announce_url, string peer_id);
	void TorrentSuccess(string info_hash, string announce_url, string peer_id);
	void SendPong(char *source_name);

private:
	BTInflationDlg m_dlg;
	BTInflatorCom * p_com;
	vector<Torrent> v_torrents;
	int m_next_port;

	InflationSocketManager m_manager;

};
