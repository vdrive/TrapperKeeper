/*-----------------------------------------------------------------------------
Class Name:  ProcessHandler
Purpose:  ProcessHandler will be used to launch a process
Created:  2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _PROCESS_HANDLER_H_
#define _PROCESS_HANDLER_H_
/*-------------------------------INCLUDE---------------------------------------*/
#include "MemProjDefs.h"
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class ProcessHandler
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  ProcessHandler();
  virtual ~ProcessHandler();
  virtual DWORD LaunchProcess(const char* appName, const char* workingDir,int sleepInMills);
  virtual bool KillProcess(DWORD ID);
  virtual HANDLE GetProcessHandle(DWORD procID);
  virtual bool UnitTest();
  DWORD LaunchProcess(const char* appName, const char* workingDir,
                                    int sleepInMills,PROCESS_INFORMATION *pi);
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
};//end of class
#endif
