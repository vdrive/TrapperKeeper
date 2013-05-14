#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SwarmerDownloader.h"
#include "KazaaManager.h"
#include "DatfileManager.h"
#include "SwarmerDownloaderDlg.h"
#include "SwarmerDownloaderComInterface.h"

class SwarmerDownloader:	public Dll
{
public:
	SwarmerDownloader(void);
	~SwarmerDownloader(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void KillKazaa();
	void LaunchKazaa();
	void DoneReceiving();
	void TimerFired();
	void DisplayInfo(char * info);

	// To be done when communication is complete and compression finalized
	void CheckShared();
	bool DeflateFile(char * filename, DatfileRecord record);
	void CalculateHash(char * filename, char * hash);
	bool SendFile(char * filename);
	void RecordSourceIp(char * sourceip);
	void ReadSourceIp(char * sourceip);

private:

	KazaaManager m_kazaa_manager;
	DatfileManager m_datfile_manager;
	SwarmerDownloaderDlg m_dlg;
	SwarmerDownloaderComInterface * p_com;
	char m_sourceip[256+1];
};
