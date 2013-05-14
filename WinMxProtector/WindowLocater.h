/*-----------------------------------------------------------------------------
Class Name: WindowLocater 
Purpose:  To faciliate the Windows Control api
Created:  2/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef WINDOWLOCATER_H
#define WINDOWLOCATER_H
/*-------------------------------INCLUDE---------------------------------------*/
#include "stdafx.h"
#include <vector>
#include "MemProjDefs.h"
#include <process.h> 
//#include <windows.h>

#define kMAXPROCWINDOWNAME 100
using namespace std;

struct WindowHandleInfoStruct
{
  HWND m_processHandle;
  char m_windowName[kMAXPROCWINDOWNAME];
  DWORD m_threadID;
  DWORD m_processID;
  HANDLE m_parent;
};

/*--------------------------CLASS DECLERATION----------------------------------*/
class WindowLocater
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WindowLocater(void);
  virtual ~WindowLocater(void);
  int FindAllWindows();
  int FindAllChildren(HWND parent);
  int GetListOfResults(WindowHandleInfoStruct procData[MAXNUMOFPROCS]);
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/

};
#endif