#pragma once

#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "MediaMakerDlg.h"
#include "MovieMakerModule.h"
#include "MusicMakerModule.h"
#include "MediaSyncher.h"
#include "DeleteModule.h"
#include "FileAssociation.h"
#include "../KazaaLauncher/KazaaLauncherInterface.h"
#include "../WinMXLauncher/WinMXLauncherInterface.h"
#include "TrailerMakerModule.h"
#include "WinMXMusicModule.h"
#include "FasttrackGiftDecoyerInterface.h"

class MediaManager:	public Dll
{
public:
	MediaManager(void);
	~MediaManager(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();


	void CheckMapsForDeletion(vector<TKSyncherMap::TKFile> &file_list);
	void CompareMapsTemplates(vector<TKSyncherMap::TKFile> &template_list);
	void ReadMapsFile();
	void DisplayInfo(char * info);
	void PreprocessTemplates(vector<TKSyncherMap::TKFile> &template_list);
	bool AddFileAssociation(char * filename, char * assocname);
	void RewriteMapsFile();
	void RemoveFileAssociationFromList(FileAssociation fa);
	int GetNumKazaas();


	bool KillAllKazaa();
	bool RelaunchAllKazaa();
	bool KillGift();
	bool RestartGift();
	bool KillWinMx();
	bool RestartWinMx();

	void CheckLogfile();
	void TimerFired();// Timer for deletion check fired

	vector<FileAssociation> v_maps_list;

private:
	CMediaMakerDlg m_dlg;
	MovieMakerModule m_movie_mod;
	MusicMakerModule m_music_mod;
	TrailerMakerModule m_trailer_mod;
	MediaSyncher m_syncher;
	DeleteModule m_deleter;
	KazaaLauncherInterface m_kazaa_interface;
	WinMxLauncherInterface m_winmx_interface;
	WinMXMusicModule m_winmx_music_mod;
	FasttrackGiftDecoyerInterface m_gift_interface;

};
