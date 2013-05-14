#include "StdAfx.h"
#include "WinMxProtectorDll.h"

//#include "windows.h"
#include "MDWINMXAppManager.h"
//#include "MDConsoleOutput.h"
/*----------------------------PREPROCESSOR-------------------------------------*/
#define NUMOFAPPSTORUN 5  //change this number to obviously control the # of 
                          //instances to run!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define APPVERSION            "Version 1.0"
#define APPNAME               "WinMX Application Launcher"
#define NUMBEROFAPPSTOLAUNCH  "Number of instances to manage:5"
#define COPYWRITE             "Copywrite MediaDefender 2004"
#define WARNING "Using this code without the written consent of MediaDefender/OnSystems is forbidden"

//
//
//
WinMxProtectorDll::WinMxProtectorDll(void)
{
}

//
//
//
WinMxProtectorDll::~WinMxProtectorDll(void)
{
}

//
//
//
void WinMxProtectorDll::DllInitialize()
{
	//m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void WinMxProtectorDll::DllUnInitialize()
{

}

//
//
//
void WinMxProtectorDll::DllStart()
{

  //MDConsoleOutput::MDPrintln(2,APPNAME);
  //MDConsoleOutput::MDPrintln(2,APPVERSION);         
  //MDConsoleOutput::MDPrintln(2,NUMBEROFAPPSTOLAUNCH);
  //MDConsoleOutput::MDPrintln(2,COPYWRITE);
  //MDConsoleOutput::MDPrintln(2,WARNING);

  DWORD numOfIntsToLaunch = NUMOFAPPSTORUN;
  MDWINMXAppManager appManager;
  appManager.SetNumberOfInstancesToManage(numOfIntsToLaunch);
  appManager.GO();
  

}

//
//
//
void WinMxProtectorDll::DllShowGUI()
{
	//m_dlg.ShowWindow(SW_NORMAL);
	//m_dlg.BringWindowToTop();
}

//
//
//
void WinMxProtectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}