/*-----------------------------------------------------------------------------
Name:    WinMxTest.cpp
Purpose: Defines the entry point for the console application.  
Created: 3/04/04 p.faby
Modified: 5/4/2004 A.D. Updated Num of apps to run.
-------------------------------------------------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "windows.h"
#include "MDWINMXAppManager.h"
#include "MDConsoleOutput.h"
/*----------------------------PREPROCESSOR-------------------------------------*/
#define NUMOFAPPSTORUN 5  //change this number to obviously control the # of 
                          //instances to run!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define APPVERSION            "Version 1.0"
#define APPNAME               "WinMX Application Launcher"
#define NUMBEROFAPPSTOLAUNCH  "Number of instances to manage:5"
#define COPYWRITE             "Copywrite MediaDefender 2004"
#define WARNING "Using this code without the written consent of MediaDefender/OnSystems is forbidden"
/*-----------------------------------------------------------------------------
Function Name: main
Arugments: This is our main thread of exectution for the WINMX launcher stuff
Returns: int - status of the main
Access:  Public
Purpose: To launch MDWINMXAppManager
Created: 3/15/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  MDConsoleOutput::MDPrintln(2,APPNAME);
  MDConsoleOutput::MDPrintln(2,APPVERSION);         
  MDConsoleOutput::MDPrintln(2,NUMBEROFAPPSTOLAUNCH);
  MDConsoleOutput::MDPrintln(2,COPYWRITE);
  MDConsoleOutput::MDPrintln(2,WARNING);

  DWORD numOfIntsToLaunch = NUMOFAPPSTORUN;
  MDWINMXAppManager appManager;
  appManager.SetNumberOfInstancesToManage(numOfIntsToLaunch);
  appManager.GO();
  
  //This is our chill-out loop.....by now we've already launched
  //all of our instances which our little heart desires.......SO.
  //we just basically spin and let our interal classes do all the
  //work, we keep spinning just to stay alive
  while(appManager.IsBusy())
  {
    Sleep(60000);
  } 

return 0;
}

 