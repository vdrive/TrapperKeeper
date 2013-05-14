/*-----------------------------------------------------------------------------
Class Name:  MDWinMXLauncher
Purpose:     To be an interface to the WinMXThreadedLauncher
Created:    3/4/04 p.faby
Modified:

  MDWinMXLauncher
      |---WinMXThreadedLauncher        
USAGE:
  MDWinMXLauncher* md = new MDWinMXLauncher();
  md->SetAppName(apptolaunch,dirtolaunch);
  md->SetNumOfAppsToLaunch(NumOfIntancesReq);
  md->Start();

  call md->IsBusy() for status
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _MDWINMXLAUNCHER_H
#define _MDWINMXLAUNCHER_H
/*-------------------------------INCLUDE---------------------------------------*/
#include "WinMXThreadedLauncher.h"

/*--------------------------CLASS DECLERATION----------------------------------*/
class MDWinMXLauncher
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDWinMXLauncher();
  ~MDWinMXLauncher();
  bool SetAppName(char* appname , char* appPath);
  bool SetNumOfAppsToLaunch(int appsToLaunch);
  DWORD StartLaunch();
  bool Busy();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  WinMXThreadedLauncher* m_launcher;
};
#endif