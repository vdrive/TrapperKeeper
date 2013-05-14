#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "fileentry.h"
#include "SQLInterface.h"
#include "FileManager.h"
#include "SwarmerSourceDlg.h"
#include "SwarmerSourceComInterface.h"

class SwarmerSourceDll:	public Dll
{
public:
	SwarmerSourceDll(void);
	~SwarmerSourceDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void SetRegistryTimer();
	void CheckRegistryTimer();
	void ProcessData();
	void SendFilesToBeMade();
	void SendRecordsComplete(string ip);

	void GenerateMapFromFiles();
	void EnumDestinations();

private:

	FileManager m_manager;
	SQLInterface m_db;
	vector<FileEntry> v_file_list;
	CSwarmerSourceDlg m_dlg;
	vector<FileEntry> v_makefile_list;
	SwarmerSourceComInterface * p_com;
	vector<string> v_destips;
};
