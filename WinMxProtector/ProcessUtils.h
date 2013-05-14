/*---------------------------------------------------------------------------
Class Name: ProcessUtils
Purpose: ProcessUtils will be a utility to poke around in memory given an .exe name
Created: 2/25/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
/*-----------------------------PREPROCESSOR----------------------------------*/
#ifndef _PROCESS_PROBE_H_
#define _PROCESS_PROBE_H_

/*-------------------------------INCLUDE-------------------------------------*/
#include "ProcessNameID.h"
#include "MemProjDefs.h"
#include "stdafx.h"

/*--------------------------------STRUCT-------------------------------------*/
/*--------------------------CLASS DECLERATION--------------------------------*/
class ProcessUtils{

/*---------------------------PUBLIC FUNCTIONS--------------------------------*/
public:
  ProcessUtils(void);
  virtual ~ProcessUtils();
  static void UnitTest();
  static bool GetProcInfo(DWORD procID, ProcessNameIDStruct *pnInfo);
  static int GetNumberOfIntancesRunning(char* nameOfProc);
  static HANDLE GetProcessHandle(DWORD prodID);
  static void CloseProcessHandle(HANDLE* procHandle);
  static DWORD LaunchApplication(const char* appName,const char* workingDir,int mills);
  static bool KillProcess(DWORD procID);
  static DWORD ReadProcMemory(HINSTANCE hInst,void* offset,unsigned char* dataBuf, DWORD sizeToRead);  
  static bool WriteProcMemory(HINSTANCE hInst,void* offset, unsigned char* writeBuf, DWORD sizeToWrite);
  static bool FlushMemory(HINSTANCE hInst,void* offset, DWORD sizeToFlush);
  static int GetProcessMemUsage(DWORD id);
/*----------------------------PUBLIC MEMBERS---------------------------------*/
/*-------------------------- PRIVATE FUNCTIONS-------------------------------*/
private:
/*----------------------------PRIVATE MEMBERS--------------------------------*/
private:
};

#endif
