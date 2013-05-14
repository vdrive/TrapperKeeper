#include "StdAfx.h"
#include "BTTorrentGeneratorDll.h"

//
//
//
BTTorrentGeneratorDll::BTTorrentGeneratorDll(void)
{
}

//
//
//
BTTorrentGeneratorDll::~BTTorrentGeneratorDll(void)
{
}

//
//
//
void BTTorrentGeneratorDll::DllInitialize()
{
	// Create the main dialog window
	m_dlg.Create( IDD_BTTORRENTGENERATOR_DLG, CWnd::GetDesktopWindow() );

	// Create the writable directory
	CreateDirectory("C:\\BTTorrentGenerator", NULL);

	m_torrentCreationManager.Init( this );
}

//
//
//
void BTTorrentGeneratorDll::DllUnInitialize()
{
	m_torrentCreationManager.Stop();
}

//
//
//
void BTTorrentGeneratorDll::DllStart()
{
	m_torrentCreationManager.Run();
}

//
//
//
void BTTorrentGeneratorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTTorrentGeneratorDll::DisplayTorrents(vector<string> *pvFilenames, vector<string> *pvHashes)
{
	m_dlg.DisplayTorrents( pvFilenames, pvHashes );
}

//
//
//
void BTTorrentGeneratorDll::ResetProgressBar(size_t nTotal)
{
	m_dlg.ResetProgressBar( nTotal );
}

//
//
//
void BTTorrentGeneratorDll::UpdateProgressBar(void)
{
	m_dlg.UpdateProgressBar();
}

//
//
//
void BTTorrentGeneratorDll::UpdateProgressText(const char *pMsg)
{
	m_dlg.UpdateProgressText( pMsg );
}
