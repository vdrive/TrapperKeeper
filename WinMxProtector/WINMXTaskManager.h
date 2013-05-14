/*-----------------------------------------------------------------------------
Class Name:  WINMXTaskManager
Purpose:  This class will excute tasks that we would like to perform on 
our WinMXProcessIDs;
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINMXTASKMANAGER_H
#define WINMXTASKMANAGER_H
/*-------------------------------INCLUDE---------------------------------------*/
#include "WinMXAppDefs.h"
#include "WINMXWindowLocater.h"
#include "MDRandomNameGenerator.h"
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class WINMXTaskManager
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WINMXTaskManager(DWORD procID, MDRandomNameGenerator *rng);
  ~WINMXTaskManager();
  bool SetID();
  bool GetReadyForSearch(HANDLE parent);
  bool Search(char* Artist,char* Title);
  bool GetReadyForSharedFileSetup(HANDLE parent);
  bool SetupSharedFolder();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
private:
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
  WINMXWindowLocater*    m_winmxWindowLocater;
  WindowHandleInfoStruct m_parentPointer;
  HANDLE                 m_parentHandle;
  DWORD                  m_processID;
  WINMX_APP_STATES       m_winmxState;
  MDRandomNameGenerator* m_nameGenerator;
};//end of class
#endif