/*-----------------------------------------------------------------------------
Class Name: WINMXIdentificationManager  
Purpose:  The purpose of this class is to act as a high level of control stepper class
to get us through all of the tasks that need to be done to change a user id in WinMX 
from a mapped application standpoint.
Created:  
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "WindowLocater.h"
#include "WINMXConfigurationManager.h"
//#include "MDConsoleOutput.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
//class WINMXIdentificationManager

/*-----------------------------------------------------------------------------
Function Name:WINMXConfigurationManager
Arugments: void
Returns: void
Access:  public
Purpose: Constructor
Created: 3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WINMXConfigurationManager::WINMXConfigurationManager()
{
  //create this name generator
  m_nameGenerator = new MDRandomNameGenerator();
}
/*-----------------------------------------------------------------------------
Function Name:~WINMXConfigurationManager
Arugments: void
Returns: void
Access:  public
Purpose: Destructor 
Created: 3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/  
WINMXConfigurationManager:: ~WINMXConfigurationManager()
{
  if(m_taskManager!=NULL)
  {
    delete m_taskManager;
    m_taskManager=NULL;
  }

  if(m_nameGenerator!=NULL)
  {
    delete m_nameGenerator;
    m_nameGenerator=NULL;
  }
}
/*-----------------------------------------------------------------------------
Function Name:ConfigureWinMX
Arugments:  DWORD procID
Returns:  bool...true if all was good
Access:   private
Purpose:  to set up on instance of winmx
Created:  3/30/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXConfigurationManager::ConfigureWinMX(DWORD procID)
{
  bool result = false;
  WINMXTaskManager m_taskManager(procID,m_nameGenerator);
  //set the unique user ID
  result = m_taskManager.SetID();
  //set the FastTrack Shared Folder as the default music folder
  result = m_taskManager.SetupSharedFolder();
  return result;
}
 
 


 