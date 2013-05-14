#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "SwarmerSourceDlg.h"
#include "MusicProject.h"
#include "FileEntry.h"
#include "SQLInterface.h"
#include "swarmersyncher.h"

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

	void ReadMapsFile();
	void ReadMusicProjects(vector<MusicProject> *projects);
	bool CompareHashWithMaps(FileEntry entry);
	void DeleteLeftoverFiles();
	void MakeOnTimer();
	void FinishMaking();
	void SetRegistryTimer();
	void CheckRegistryTimer();

	void Click(HWND hwnd);
	void ClosePopups();
	void CalculateHash(char * outputfilename, char * hashin);

private:
	SQLInterface m_db;
	SwarmerSourceDlg m_dlg;
	MediaSyncher m_syncher;
	vector<FileEntry> v_file_list;
	vector<FileEntry>v_makefile_list;
	int m_listpos;
	vector<FileEntry> old_file_list;

};
