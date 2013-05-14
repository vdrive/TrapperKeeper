/*-----------------------------------------------------------------------------
Class Name:  WinMXThreadedLauncher
Purpose:     facilitates the launching of multiple WinMX-s
Created:     3/4/04 p.faby
Modified:

-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _WINMXTHREADEDLAUNCHER_H
#define _WINMXTHREADEDLAUNCHER_H
/*-------------------------------DEFINES---------------------------------------*/
#define MAXNUMBEROFINSTANCES 610
#define TIMETOSLEEPAFTERLAUNCH 0
/*-------------------------------INCLUDE---------------------------------------*/
#include "ProcessUtils.h"
#include "ProcessHandler.h"
#include "WinMXMemoryAdjust.h"
#include "MemProjDefs.h"
#include <vector>
using namespace std ;
/*--------------------------CLASS DECLERATION----------------------------------*/
class WinMXThreadedLauncher
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WinMXThreadedLauncher();
  virtual ~WinMXThreadedLauncher();
  virtual bool SetPathOfApp(char* appName, char* appPath);
  virtual bool SetNumberToLaunch(int numOfProcsToLaunch);
  virtual bool IsRunning();
  virtual bool IsOK();
  virtual bool StartLaunch();
  virtual bool Stop();
  virtual vector<DWORD> GetListOfLaunchedInstances();
  virtual int GetNumberOfRunningWinMXs();
  virtual DWORD LaunchWinMXInstance();
  virtual bool KillInstance(DWORD procID);
  virtual bool StartThread();

/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
  //ProcessHandler*   m_ProcessHandler;
  ProcessUtils*     m_processUtils;
  WinMXMemoryAdjust* m_winmxMemAdjuster;
  HANDLE            m_launcherExecuterThreadHandle;
  int               m_numberOfProcsToLaunch;
  bool              m_running;
  DWORD             m_threadID;
  char*             m_appName;
  char*             m_appPath;
  int               m_numberOfProcsLaunched;
  DWORD             m_currentProcToFix;
/*----------------------------PRIVATE MEMBERS----------------------------------*/
 
};
#endif