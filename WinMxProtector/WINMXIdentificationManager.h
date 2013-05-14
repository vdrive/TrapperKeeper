/*-----------------------------------------------------------------------------
Class Name: WINMXIdentificationManager  
Purpose:  The purpose of this class is to act as a high level of control stepper class
to get us through all of the tasks that need to be done to change a user id in WinMX 
from a mapped application standpoint.
Created:   3/14/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINMXIDENTIFICATIONMANAGER_H
#define WINMXIDENTIFICATIONMANAGER_H
/*-------------------------------INCLUDE---------------------------------------*/
 
#include "MDRandomNameGenerator.h"
#include "WINMXTaskManager.h"
//#include <windows.h>
/*--------------------------CLASS DECLERATION----------------------------------*/
class WINMXIdentificationManager
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WINMXIdentificationManager();
  virtual ~WINMXIdentificationManager();
  bool FixID(DWORD procID);
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
  int GenerateUniqueID(char* GeneratedId);
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
  HWND              m_ourMainWindow;
  WINMXTaskManager*  m_taskManager; 
  MDRandomNameGenerator *m_nameGenerator;
};//end WINMXIdentificationManager

#endif


