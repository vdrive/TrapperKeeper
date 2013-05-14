/*-------------------------------INCLUDE---------------------------------------*/
#include "MDWINMXAppManager.h"
#include "MDRandomNameGenerator.h"
#include "MDWinMXLauncher.h"
#include "WINMXConfigurationManager.h"
#include "WINMXTaskManager.h"

/*----------------------------PREPROCESSOR-------------------------------------*/
 
#define WINMXSTRING "WinMX.exe"
#define APPTOLAUNCH "C:\\Program Files\\WinMX\\WinMX.exe"
#define DIRTOLAUNCH "C:\\Program Files\\WinMX\\"
/*-------------------------------GLOBAL----------------------------------------*/
DWORD WINAPI AppManagementThread(void *);
int g_numOfInstancesToRun = 0;
bool g_running = false;
DWORD g_ourThreadID;
HANDLE g_ourThreadHandle;

/*-----------------------------------------------------------------------------
Function Name:MDWINMXAppManager()
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
MDWINMXAppManager::MDWINMXAppManager()
{
}
/*-----------------------------------------------------------------------------
Function Name:~MDWINMXAppManager()
Arugments: 
Returns: 
Access:
Purpose: 
Created:  3/04/2004 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDWINMXAppManager::~MDWINMXAppManager()
{
}
/*-----------------------------------------------------------------------------
Function Name:SetNumberOfInstancesToManage(int numOfApps)
Arugments: int - num of instances to keep going
Returns: 
Access:
Purpose: 
Created:  3/04/2004 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDWINMXAppManager::SetNumberOfInstancesToManage(int numOfApps)
{
  bool result = false;
  if(numOfApps >= 0)
  {
    g_numOfInstancesToRun = numOfApps;
    result = true;
  }
  return result;
}
bool MDWINMXAppManager::IsBusy()
{ 
  return g_running;
}
/*-----------------------------------------------------------------------------
Function Name:GO()
Arugments:void
Returns:  bool, true if we launch
Access:  Public
Purpose: 
Created:  3/04/2004 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDWINMXAppManager::GO()
{
  bool result = false;

  g_ourThreadID = 0;
  g_ourThreadHandle = NULL;

  //flag to notify work to be done
  g_running = true;

  //launch our workter thread
  g_ourThreadHandle = CreateThread( 
                        NULL,                       // no security attributes 
                        0,                          // use default stack size  
                        AppManagementThread,     // thread function 
                        NULL,                       // argument to thread function 
                        NORMAL_PRIORITY_CLASS,      // use default creation flags 
                        &g_ourThreadID);               // returns the thread identifier 
  if(g_ourThreadHandle != NULL)
  {
    result = true;
  }
  else
  {
    g_running = false;
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: AppManagementThread(void *)
Arugments: void....whatever we want it to be
Returns: 
Access:
Purpose: 
Created:  3/04/2004 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD WINAPI AppManagementThread(void *)
{
  WINMXConfigurationManager configManager;
  MDWinMXLauncher launcher;
  launcher.SetAppName(APPTOLAUNCH,DIRTOLAUNCH);
 
  int numOfInst=0;
  
  while(g_running)
  {
    //check the number of instacnes running
    numOfInst = ProcessUtils::GetNumberOfIntancesRunning(WINMXSTRING);
    //if we need to launch another instance, let's do it
    if(numOfInst < g_numOfInstancesToRun)
    {
      launcher.SetNumOfAppsToLaunch(1);
      DWORD procID = launcher.StartLaunch();
      Sleep(5000);

      if(procID == NULL)
      {
        printf("We just really failed a launch");
      }
      else
      {
        //Set the name of this new user and setup the shared folder
        configManager.ConfigureWinMX(procID); 
        //To Do: Maybe save these processIDs for future reference
      }
    }
    else
    {
      Sleep(500);
    }
  }
  return 1;
}

