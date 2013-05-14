/*-----------------------------------------------------------------------------
Class Name: FileReaderWriter
Purpose:  To use api calls to write files
Created:  3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _MDFILEREADERWRITER_H_
#define _MDFILEREADERWRITER_H_
/*-------------------------------INCLUDE---------------------------------------*/
//#include <windows.h>
#include "stdafx.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDFileReaderWriter
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDFileReaderWriter();
  ~MDFileReaderWriter();
  bool OpenFileAccessor(char* fileName, DWORD access);
  bool CloseFileAccessor();
  bool IsFileBusy(char* fileName);
  bool OpenFile(char* fileName,DWORD access);
  bool Write(unsigned char* byteArray,DWORD bytesToWrite);
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
  static const DWORD READACCESS  =  GENERIC_READ;
  static const DWORD WRITEACCESS = GENERIC_WRITE;
  static bool IsFileOpen(char* fileName);
/*----------------------------PRIVATE MEMBERS----------------------------------*/
  HANDLE  m_fileHandle;
  DWORD   m_index;
};
#endif