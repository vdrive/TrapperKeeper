/*-----------------------------------------------------------------------------
Class Name: WINMXConfigurationManager  
Purpose:  The purpose of this class is to act as a high level of control stepper class
to get us through all of the tasks that need to be done to change a user id in WinMX 
from a mapped application standpoint.
Created:   3/14/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINMXCONFIGURATIONMANAGER_H
#define WINMXCONFIGURATIONMANAGER_H
/*-------------------------------INCLUDE---------------------------------------*/
 
#include "MDRandomNameGenerator.h"
#include "WINMXTaskManager.h"
//#include <windows.h>
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class WINMXConfigurationManager
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WINMXConfigurationManager();
  virtual ~WINMXConfigurationManager();
  bool ConfigureWinMX(DWORD procID);
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  HWND              m_ourMainWindow;
  WINMXTaskManager*  m_taskManager; 
  MDRandomNameGenerator *m_nameGenerator;
};//end WINMXConfigurationManager

#endif


