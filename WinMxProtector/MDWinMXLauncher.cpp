/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "MDWinMXLauncher.h"
/*-----------------------------------------------------------------------------
Function Name: MDWinMXLauncher()
Arugments: void
Returns:   void
Access:    Public
Purpose:   Constructor
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDWinMXLauncher::MDWinMXLauncher()
{
  m_launcher = new WinMXThreadedLauncher();
}
/*-----------------------------------------------------------------------------
Function Name: ~MDWinMXLauncher()
Arugments: void
Returns:   void
Access:    Public
Purpose:   Destructor
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDWinMXLauncher::~MDWinMXLauncher()
{
  m_launcher->Stop();
  delete m_launcher;
  m_launcher = NULL;
}
/*-----------------------------------------------------------------------------
Function Name: SetAppName(char* appname , char* appPath)
Arugments: char* Application Name, char* Application Path
Returns:   bool - true if valid
Access:    Public
Purpose:   to set the file we want to launch
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDWinMXLauncher::SetAppName(char* appname , char* appPath)
{
  bool result = false;
  if(m_launcher!=NULL)
  {
    result = m_launcher->SetPathOfApp(appname,appPath);
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: SetNumOfAppsToLaunch(int appsToLaunch)
Arugments: int - number of applications to launch
Returns:   bool - true if a valid range
Access:    Public
Purpose:   to set the number of apps to launch
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDWinMXLauncher::SetNumOfAppsToLaunch(int appsToLaunch)
{
  bool result = false;
  if(m_launcher!=NULL)
  {
    result = m_launcher->SetNumberToLaunch(appsToLaunch);
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Start()
Arugments: void 
Returns:   bool - true if start was succesfull
Access:    Public
Purpose:   to start the launching of the procs
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD MDWinMXLauncher::StartLaunch()
{
  DWORD result = false;
  if(m_launcher!=NULL)
  {
    result = m_launcher->StartLaunch();
  }
  while(Busy()){Sleep(100);}

  result = m_launcher->m_currentProcToFix;

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Busy()
Arugments: void
Returns:   bool - true if still busy launching applications
Access:    Public
Purpose:   to give us status of the applications we're trying to launch
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDWinMXLauncher::Busy()
{
  bool result = false;
  if(m_launcher!=NULL)
  {
    result = m_launcher->IsRunning();
  }
  return result;
}

