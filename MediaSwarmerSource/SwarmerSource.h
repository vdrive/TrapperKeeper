#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "SwarmerSourceDlg.h"
#include "MusicProject.h"
#include "FileEntry.h"
#include "SQLInterface.h"

class SwarmerSource: public Dll
{
public:
	SwarmerSource(void);
	~SwarmerSource(void);


	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void DataReceived(char *source_name, void *data, int data_length);
	
	bool DeflateFile(char * filepath);
	void CopyCompletedDownloads();
	bool MakeSig2Dat(char * filename, int bytes, char hash[40]);
	bool GetFileList();
	void ConvertHash(char hash[40]);
	void CloseandRelaunchKazaa();

private:
	SQLInterface m_db;
	SwarmerSourceDlg m_dlg;
	vector<FileEntry> file_list;

};
