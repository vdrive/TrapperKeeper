/*-----------------------------------------------------------------------------
Name:     ProcessHandler.cpp
Purpose:  ProcessHandler will be used to launch a process
Created:  2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-----------------------------------*/
/*-------------------------------INCLUDE-------------------------------------*/
#include "ProcessHandler.h"
//#include "MDConsoleOutput.h"
/*---------------------------PUBLIC FUNCTIONS--------------------------------*/
/*-----------------------------------------------------------------------------
Function Name: ProcessHandler()
Arugments: void
Returns:   void
Purpose:   Contructor
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
ProcessHandler::ProcessHandler(){};
/*-----------------------------------------------------------------------------
Function Name: ~ProcessHandler()
Arugments: void
Returns:   void
Purpose:   Destructor
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
ProcessHandler::~ProcessHandler(){};

/*-----------------------------------------------------------------------------
Function Name:  KillProcess(DWORD ID)
Arugments: DWORD - process id to kill
Returns:   bool - true if killed
Purpose:   to kill a process
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool ProcessHandler::KillProcess(DWORD ID)
{
  bool result = false;
  
  if(ID>0)
  {
    //Get the handle for the process we want to kill
    HANDLE handle = GetProcessHandle(ID);
    
    //now kill it
    if(handle != NULL)
    {
      result = TerminateProcess(handle,0);
    }
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:  GetProcessHandle(DWORD ID)
Arugments: DWORD - process id to kill
Returns:   HANDLE - handle of the requested process
Purpose:   to get a handle to a process
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
HANDLE ProcessHandler::GetProcessHandle(DWORD procID)
{
  HANDLE handle=NULL;

  if(procID>0)
  {
    //Get the handle for the process we want to kill
    handle=OpenProcess(PROCESS_ALL_ACCESS,true,procID);
  }

  return handle;
}
/*-----------------------------------------------------------------------------
Function Name:  LaunchProcess(const char* appName)
Arugments: const char* - the name of the process we want including dir
Returns:   DWORD - the process id
Purpose:   launch a process and to return the process id to know what we launched
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD ProcessHandler::LaunchProcess(const char* appName, const char* workingDir,int sleepInMills)
{

  //MDConsoleOutput::MDPrintln("Attempting Launching new Process");
  DWORD result = NULL;
  try{
    if(appName != 0)
    {
      STARTUPINFO si = {sizeof si};
      //specify this flag to say "use the wShowWindow" flag
      si.dwFlags      = STARTF_USESHOWWINDOW;
      //use this guy to create the window as maximized
      si.wShowWindow  = SW_SHOWMAXIMIZED;

      PROCESS_INFORMATION pi; 
      //Attempt to launch process
		  bool success = CreateProcess(
					  appName,
					  NULL,
					  NULL,
					  NULL,
					  FALSE,
					  //ABOVE_NORMAL_PRIORITY_CLASS,
            BELOW_NORMAL_PRIORITY_CLASS,
					  NULL,
					  workingDir,
					  &si,
					  &pi);

      //give this guy time to kick off
      if(sleepInMills>0)
      {
        Sleep(sleepInMills);
      }
      //if true, we successfully launched the application
      if(success)
      {
        //s et the return value to the process ID
        result = pi.dwProcessId;

      }
      else
      {
        //char szBuf[80]; 
        DWORD dw = GetLastError(); 
      //  MDConsoleOutput::MDPrintln("GetLastError returned:",dw);  
      }
    }//end if
  }
  catch(char* i)
  {
   // MDConsoleOutput::MDPrintln("Error in ProcessHandler:",i);
  }
  return result;
}

DWORD ProcessHandler::LaunchProcess(const char* appName, const char* workingDir,
                                    int sleepInMills,PROCESS_INFORMATION *pi)
{
 // MDConsoleOutput::MDPrintln("Attempting Launching new Process");
  DWORD result = NULL;
  if(appName != 0)
  {
    //process structs needed to launch the app
		STARTUPINFOA si = {sizeof si};
    //PROCESS_INFORMATION pi; 
    //Attempt to launch process
		bool success = CreateProcess(
					appName,
					NULL,
					NULL,
					NULL,
					FALSE,
					//ABOVE_NORMAL_PRIORITY_CLASS,
          BELOW_NORMAL_PRIORITY_CLASS,
					NULL,
					workingDir,
					&si,
					pi);

    //give this guy time to kick off
    if(sleepInMills>0)
    {
      Sleep(sleepInMills);
    }
    //if true, we successfully launched the application
    if(success)
    {
      //s et the return value to the process ID
      result = pi->dwProcessId;
      //MDConsoleOutput::MDPrintln("ProcessID:",pi->dwProcessId);
      //MDConsoleOutput::MDPrintln("ThreadID:",pi->dwThreadId);


    }
    else
    {
      DWORD dw = GetLastError(); 
      //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"GetLastError returned:",dw);  
    }
  }//end if
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:  UnitTest()
Arugments: void
Returns:   true if we successfully launched the app 
Purpose:   to test the ProcessLauncer class
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool ProcessHandler::UnitTest()
{
/*  char* apptolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\Winmx.exe";
  char* dirtolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\";
  PROCESS_INFORMATION pi;
  int procID = LaunchProcess(apptolaunch,dirtolaunch,&pi);
  if(procID != -1) result = true;
  return result;*/
  return true;
}
/*-------------------------- PRIVATE FUNCTIONS-------------------------------*/
 