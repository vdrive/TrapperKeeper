/*-----------------------------------------------------------------------------
Class Name:  MDMemoryEditor
Purpose:  
Created:  
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _MDMEMORY_EDITOR_H_
#define _MDMEMORY_EDITOR_H_
/*-------------------------------INCLUDE---------------------------------------*/
#include "MemProjDefs.h"
#include "Stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDMemoryEditor
{
public:
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
  MDMemoryEditor(DWORD procID);
  MDMemoryEditor(HANDLE procHandle,HINSTANCE instance);
  virtual ~MDMemoryEditor();
  DWORD ReadProcMemory(void* offset,unsigned char *dataBuf,  DWORD sizeToRead);  
  bool WriteProcMemory(void* offset,unsigned char *writeBuf,  DWORD sizeToWrite);
  bool MemoryInfoQuery(void* offset, MEMORY_BASIC_INFORMATION* pMemInfo);
  void GetMemoryStateString(DWORD state, char* memState);
  void GetAllocString(DWORD type, char* memAccessType);
  void GetMemoryTypeString(DWORD memType, char* memString );
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
private:
/*----------------------------PRIVATE MEMBERS----------------------------------*/
  DWORD m_processID;
  HANDLE m_handle;
  HINSTANCE m_procInstance;
}; 
#endif 