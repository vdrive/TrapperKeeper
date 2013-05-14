/*-----------------------------------------------------------------------------
Class Name: ProcNameID
Purpose: ProcNameID will be used to associate a process name with a process ID
Created: 2/25/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
/*-----------------------------PREPROCESSOR----------------------------------*/
#ifndef _ProccessNameID_H_
#define _ProccessNameID_H_

/*-------------------------------INCLUDE-------------------------------------*/
#include "MemProjDefs.h"
#include "Stdafx.h"

/*--------------------------------STRUCT--------------------------------------*/
struct ProcessNameIDStruct
{
  DWORD    m_procID;
  char     m_procName[MAXNAMELENGTH];
  DWORD    m_baseAddr;
  PROCESS_INFORMATION m_procInfo;
};
/*--------------------------CLASS DECLERATION--------------------------------*/
class ProcessNameID
{
/*---------------------------PUBLIC FUNCTIONS--------------------------------*/
public:
  ProcessNameID(void);
  ~ProcessNameID(void);
  bool SetProcIDName(const DWORD procID,const char *procName, const DWORD baseAddr, const HANDLE handle);
  bool GetProcNameID(const char* procName, ProcessNameIDStruct *nameID);

/*-------------------------- PRIVATE FUNCTIONS-------------------------------*/
  void ReleaseHandles();
/*----------------------------PUBLIC MEMBERS---------------------------------*/
/*----------------------------PRIVATE MEMBERS--------------------------------*/
private:
  int m_indexOfPairs ;
  ProcessNameIDStruct   m_NameIDPairs[MAXNUMOFPROCS] ;
};
#endif
