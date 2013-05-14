/*-----------------------------------------------------------------------------
Class Name: WINMXWindowLocater
Purpose:  The purpose of the WINMXWindowLocater class is to get the handles to
all of the useful controls that we need to control a certain processIDs interface
Created:  3/9/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINMXWINDOWLOCATER_H
#define WINMXWINDOWLOCATER_H
/*-------------------------------INCLUDE---------------------------------------*/
#include "WindowLocater.h"
#include "WinMXAppDefs.h"
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class WINMXWindowLocater : public WindowLocater
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WINMXWindowLocater(DWORD procID);
  virtual ~WINMXWindowLocater();
  bool InitWINMXControls(int winMXState);  // 0x01 = Network 0x02 = Search Page 0x03 = Shared Setup
  bool GetControlHandle(char* ControlToGrab,WindowHandleInfoStruct* pData,int winMXState);
/*---------------------------PRIVATE FUNCTIONS---------------------------------*/
  int DoSearch(DWORD procID,WindowHandleInfoStruct procData[MAXNUMOFPROCS]);
  bool GetControlHandles();
  bool InitOurControlData();
  bool FindWINMXControls(WindowHandleInfoStruct dataArray[MAXNUMOFPROCS],int numToSearch,int winMXState);
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  WindowHandleInfoStruct m_controlList[kWINMX_NUM_MAIN_COMPONENTS];
  DWORD m_currentProcId;

};//end class
#endif
