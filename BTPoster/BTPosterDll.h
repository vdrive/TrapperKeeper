#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTPosterDlg.h"

class BTPosterDll :	public Dll
{
public:
	BTPosterDll(void);
	~BTPosterDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void SupplySynched(const char* source_ip);

	void Post(string filename, string name, char *torrent, int size);
	string GetPostInfo(string data);
	string GetSubstring(const string data, int * pos, string start_token, string end_token);
private:
	BTPosterDlg m_dlg;
	//SyncherDest m_syncher;
};
