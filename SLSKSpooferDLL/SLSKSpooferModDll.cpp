#include "StdAfx.h"
#include "SLSKSpooferModDLL.h"
#include "SLSKSpooferDlg.h"
//#include "SLSKController.h"
#include "SLSKSpoofer.h"
#include "DCHeader.h"
#include "ProjectChecksums.h"
#include "ProjectKeywordsVector.h"
//
//
//

//HINSTANCE SLSKSpooferMod::m_hinst;
//AppID SLSKSpooferMod::m_app_id;

//Change it to your main app class instead of PlugInApp class here
//SLSKController SLSKSpooferMod::m_app;


SLSKSpooferMod::SLSKSpooferMod(void)
{
}

//
//
//
SLSKSpooferMod::~SLSKSpooferMod(void)
{
}

//
//
//
void SLSKSpooferMod::DllInitialize()
{
	//m_app.DllInitialize();
	m_dlg.Create(IDD_SLSKSPOOFER_DIALOG,CWnd::GetDesktopWindow());
	//m_dlg.OnTimer(1);
	CSLSKSpooferApp spoof_app;
}

//
//
//
void SLSKSpooferMod::DllUnInitialize()
{
//	m_app.DllUnInitialize();
}

//
//
//
void SLSKSpooferMod::DllStart()
{
//	m_project_manager.DllStart(this,"SLSKSpoofer");
	//m_app.DllStart();
	m_dlg.m_project_manager.DllStart(this,"SLSKSpoofer");
	int connectioncount = 0;
//	m_controller.GetProjects();
	CString s;
	s.AppendFormat("%d", 0);
	char* tempchar = new char[2];
	strcpy(tempchar, "0");
	m_dlg.setParentStatus(tempchar);
	char* tempchar1 = new char[2];
	strcpy(tempchar1, "0");
	m_dlg.setPeerStatus(tempchar1);
	char* tempchar2 = new char[2];
	strcpy(tempchar2, "0");
	m_dlg.setServerStatus(tempchar2);
	char* tempchar3 = new char[2];
	strcpy(tempchar3, "0");
	m_dlg.setPeerConnectingStatus(tempchar3);
	char* tempchar4 = new char[2];
	strcpy(tempchar4, "0");
	m_dlg.setParentConnectingStatus(tempchar4);
	char* tempchar5 = new char[2];
	strcpy(tempchar5, "0");
	m_dlg.setServerModStatus(tempchar5);
	char* tempchar6 = new char[2];
	strcpy(tempchar6, "0");
	m_dlg.setParentModStatus(tempchar6);
	char* tempchar7 = new char[2];
	strcpy(tempchar7, "0");
	m_dlg.setPeerModStatus(tempchar7);
	m_dlg.m_controller.start();
	m_dlg.m_started_spoofing = false;
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
//	m_dlg.SetTimer(1, 1*1000, NULL);
//	m_dlg.SetTimer(3, 5*60*1000,NULL);
}

//
//
//
void SLSKSpooferMod::DllShowGUI()
{
//	m_app.DllShowGUI();
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SLSKSpooferMod::DataReceived(char *source_name, void *data, int data_length)
{

}