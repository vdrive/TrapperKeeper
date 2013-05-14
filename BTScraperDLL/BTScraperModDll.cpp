#include "StdAfx.h"
#include "BTScraperModDll.h"

//
//
//
BTScraperMod::BTScraperMod(void)
{
	b_started = false;
//	_crtBreakAlloc = 182;
}

//
//
//
BTScraperMod::~BTScraperMod(void)
{
	delete p_com_interface;
	p_com_interface = NULL;
}

//
//
//
void BTScraperMod::DllInitialize()
{
	m_dlg.Create(IDD_BTSCRAPERDLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg_hwnd = m_dlg.GetSafeHwnd();
}

//
//
//
void BTScraperMod::DllUnInitialize()
{
}

//
//
//
void BTScraperMod::DllStart()
{
	p_com_interface = new BTSComInterface();
	p_com_interface->Register(this, 891);

	c.SetWindow(&m_dlg);
	c.SetDone(false);

	m_dlg.OnTimer(1);
	m_dlg.SetTimer(1, 5*60*1000,NULL);  // 5 mins between scrapes
	m_dlg.SetTimer(2, 5*60*1000, NULL); // check for new trackers
	m_dlg.SetTimer(3, 60*60*1000, NULL); // update seed count of trackers

}

void BTScraperMod::Collect() {
	if(!b_started) {
		m_dlg.Log("Starting Collection Thread");
		b_started = true;
		c.Collect();
	}
	b_started = false;
}


//
//
//
void BTScraperMod::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	
}

//
//
//
void BTScraperMod::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}