#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTRemoverDlg.h"
#include "projectmanager.h"
#include "webremover.h"

class BTRemoverDll :	public Dll
{
public:
	BTRemoverDll(void);
	~BTRemoverDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void SupplySynched(const char* source_ip);

	void Collect();
	void Post(string filename, string name, char *torrent, int size);
	string GetPostInfo(string data);
	string GetSubstring(const string data, int * pos, string start_token, string end_token);
private:
	BTRemoverDlg m_dlg;
	vector<string> v_ids;
	vector<WebRemover*> v_search;

	//SyncherDest m_syncher;
};
