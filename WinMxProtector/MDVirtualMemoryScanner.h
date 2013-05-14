/*-----------------------------------------------------------------------------
Class Name: MDVirtualMemoryScanner
Purpose:  The purpose of Virtual Memory Scanner is to be a friendly wrapper 
to getting at the different areas of memory which a progam allocates
Created:   3/23/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDVIRTUALMEMORYSCANNER_H
#define MDVIRTUALMEMORYSCANNER_H
/*-------------------------------INCLUDE---------------------------------------*/
//#include "windows.h"
#include "MDMemoryEditor.h"
#include "MDFileReaderWriter.h"
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDVirtualMemoryScanner
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDVirtualMemoryScanner(DWORD processID);
  ~MDVirtualMemoryScanner();  
  bool ScanVirtualMemorySpace();
  bool GetProcessMemoryInformation();
  bool findData(char* data,MEMORY_BASIC_INFORMATION memInfo,MDFileReaderWriter* rw);
  bool ClearData(char* data,MEMORY_BASIC_INFORMATION memInfo,MDFileReaderWriter* rw);
  bool ClearVirtualMemorySpace();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  DWORD                     m_processID;
  MDMemoryEditor*           m_memoryHacker;
  MEMORY_BASIC_INFORMATION  m_processMemoryInfo;
};
#endif
