#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTTorrentGeneratorDlg.h"
#include "TorrentCreationManager.h"

//
//
//
class BTTorrentGeneratorDll
	: public Dll
{
public:
	BTTorrentGeneratorDll(void);
	~BTTorrentGeneratorDll(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void DisplayTorrents(vector<string> *pvFilenames, vector<string> *pvHashes);

	void ResetProgressBar(size_t nTotal);
	void UpdateProgressBar(void);

	void UpdateProgressText(const char *pMsg);

private:
	BTTorrentGeneratorDlg		m_dlg;
	CTorrentCreationManager		m_torrentCreationManager;
};
