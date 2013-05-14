/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include <vector>
#include "WindowLocater.h"
#include "stdafx.h"
#include "MemProjDefs.h"
#include <process.h> 
//#include <windows.h>
/*---------------------------CALLBACK FUNCTIONS-------------------------------*/

bool CALLBACK EnumWindowsProcVIAName( HWND hwnd, LPARAM lParam);
bool CALLBACK EnumWindowsProc ( HWND hwnd, LPARAM lParam); 
bool CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) ;
 
/*----------------------------GLOBAL MEMBERS----------------------------------*/
WindowHandleInfoStruct windowProcData[MAXNUMOFPROCS];
int windowProcDataIndex = 0;
int childProcDataIndex = 0;
int DataCount = 0;
/*----------------------------GLOBAL Functions--------------------------------*/
bool GetHWNDInfo(WindowHandleInfoStruct procData[MAXNUMOFPROCS]);
bool DebugGetHWNDInfo();
bool CleanBlocks(WindowHandleInfoStruct procData[MAXNUMOFPROCS]);
/*-----------------------------------------------------------------------------
Function Name: WindowLocater(void)
Arugments:void 
Returns:  void
Access:   Public
Purpose:  Contructor
Created:  3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WindowLocater::WindowLocater(void)
{
  CleanBlocks(windowProcData);
}
/*-----------------------------------------------------------------------------
Function Name:~WindowLocater(void)
Arugments:void 
Returns:  void
Access:   Public
Purpose:  Destructor
Created:  3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WindowLocater::~WindowLocater(void)
{
}
 
/*-----------------------------------------------------------------------------
Function Name:FindAllChildren
Arugments: void
Returns: bool - true if things go well
Access: Public
Purpose:  To wrap up the callback function EnumChildWindows....Takes a handle and 
enumerates it's children window
Created: 3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
int WindowLocater::FindAllChildren(HWND parent)
{
  //empty previous search
  CleanBlocks(windowProcData);

  //enumerate our currently running children under this parent
  EnumChildWindows((HWND) parent,(WNDENUMPROC)EnumWindowsProc,  0) ;

  //evalute our enumeration 
  GetHWNDInfo(windowProcData);

  //return the number of children we found
  return DataCount;
}
/*-----------------------------------------------------------------------------
Function Name:FindAllWindows
Arugments: void
Returns: bool - true if things go well
Access: Public
Purpose: to gather data
Created: 3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
int WindowLocater::FindAllWindows()
{
  //empty previous search
  CleanBlocks(windowProcData);

  //enumerate our currently running processes
  EnumWindows((WNDENUMPROC)EnumWindowsProc, 0);
  
  //evalute our enumeration
  GetHWNDInfo(windowProcData);

  //return the number of parent windows we found
  return DataCount;
}
 
int WindowLocater::GetListOfResults(WindowHandleInfoStruct procData[MAXNUMOFPROCS])
{
  int result = 0;
  int index=0;
  for(index=0;index<DataCount;index++)
  {
    procData[index].m_processHandle = windowProcData[index].m_processHandle;
    procData[index].m_processID     = windowProcData[index].m_processID ;
    procData[index].m_threadID      = windowProcData[index].m_threadID ;
    memcpy(procData[index].m_windowName,windowProcData[index].m_windowName,kMAXPROCWINDOWNAME);
  }
  
  return DataCount;
}
 
/*-----------------------------------------------------------------------------
Function Name: bool CALLBACK EnumWindowsProcVIAPROCID( HWND hwnd, LPARAM lParam) 
Arugments:HWND hwnd - child window handle, LPARAM lParam - user data
Returns:  bool - true to continue else false to terminate
Access:   Public :-(
Purpose:  to enumerate all children of the wain desktop application
Created: 3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam) 
{ 
  bool result = false;

  windowProcData[windowProcDataIndex++].m_processHandle = hwnd;
  DataCount++;
  return true; 
} 
/*-----------------------------------------------------------------------------
Function Name: EnumChildProc( HWND hwnd, LPARAM lParam)
Arugments: HWND hwnd, LPARAM lParam
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
bool GetHWNDInfo(WindowHandleInfoStruct procData[MAXNUMOFPROCS])
{
  bool result = false;

  if(DataCount != 0)
  {
    char nameBuf[kMAXPROCWINDOWNAME];
    DWORD threadID=0;
    DWORD processID=0;

    for(int i=0;i<DataCount;i++)
    {
      //GetWindowThreadProcessId is an API call
      threadID = GetWindowThreadProcessId(procData[i].m_processHandle,&processID);
      
      //GetWindowText is an API call which takes a HWND,returns the name of the HWND
      int length = GetWindowText(procData[i].m_processHandle,nameBuf,kMAXPROCWINDOWNAME );
      procData[i].m_processID      = processID;
      procData[i].m_threadID       = threadID;
      memset(procData[i].m_windowName,0,kMAXPROCWINDOWNAME);
      memcpy(procData[i].m_windowName,nameBuf,kMAXPROCWINDOWNAME);

    }//end for
  }//endif

  return true;
}
/*-----------------------------------------------------------------------------
Function Name: EnumChildProc( HWND hwnd, LPARAM lParam)
Arugments: HWND hwnd, LPARAM lParam
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
bool DebugGetHWNDInfo()
{
  bool result = false;

  /*int numOfHWND = windowProcData.size();
 // WindowHandleInfoStruct tempData;
  char nameBuf[kMAXPROCWINDOWNAME];
  DWORD threadID=0;
  DWORD processID=0;
  for(int i=0;i<numOfHWND;i++)
  {
    WindowHandleInfoStruct &tempData = windowProcData.at(i);
    MDConsoleOutput::Println(("HWND:%x      PROCID:%d     THREADID%d     WNDNAME:%s\n",
      tempData.m_processHandle, tempData.m_processID, tempData.m_threadID, tempData.m_windowName); 
  }*/
  return true;
}

/*-----------------------------------------------------------------------------
Function Name:CleanBlocks(WindowHandleInfoStruct procData[MAXNUMOFPROCS])
Arugments:WindowHandleInfoStruct[MAXNUMOFPROCS] 
Returns:  bool - true if successfull
Access:   Public
Purpose:  to clean up arrays of data
Created:  3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool CleanBlocks(WindowHandleInfoStruct procData[MAXNUMOFPROCS])
{
  bool result = false;
  for(int i=0;i<MAXNUMOFPROCS;i++)
  {
    procData[i].m_processHandle = 0;
    procData[i].m_processID     = 0;
    procData[i].m_threadID      = 0;
    memset(procData[i].m_windowName,0,kMAXPROCWINDOWNAME);
  }
  windowProcDataIndex = 0;
  DataCount=0;
  result = true;
  return result;
}

