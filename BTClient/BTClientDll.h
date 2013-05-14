#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTCLientDlg.h"
#include "ClientConnection.h"
#include "..\BTScraperDll\PeerList.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "..\BTScraperDll\TorrentFileHeader.h"
#include "BTClientComInterface.h"
#include "ConnectionManager.h"
#include "ClientData.h"

class BTClientDll :	public Dll
{
public:
	BTClientDll(void);
	~BTClientDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, UINT data_length);

	void SpawnProjectSession(TorrentFile * torrent);
	void DisplayInfo(char * info);
	void ReadTorrents();  //For manual torrents
	void ExploreFolder(char * folder, vector<string> * filenames);

	void ClientDataReported(ClientData * cdata);
	void ModuleDataReported(ModuleData * cdata);
	void TimerFired(UINT nIDEvent);
	void WriteStatsToFile();

	void StatusRequestReceived(char *source_name);
	void RemoveModule(int modnum);



private:
	BTClientDlg m_dlg;
	vector<CClientConnection*> v_connections;
	BTClientComInterface * p_com;
	BTClientComInterface * p_data_com;
	ConnectionManager m_manager;
	vector<ClientData> v_client_data;
	vector<ModuleData> v_mod_data;

};
