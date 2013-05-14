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
#include "WINMXIdentificationManager.h"
//#include "MDConsoleOutput.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
//class WINMXIdentificationManager

/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
WINMXIdentificationManager::WINMXIdentificationManager()
{
  //create this name generator
  m_nameGenerator = new MDRandomNameGenerator();
}
  
WINMXIdentificationManager:: ~WINMXIdentificationManager()
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
  
 
bool WINMXIdentificationManager::FixID(DWORD procID)
{
  bool result = false;
//  MDConsoleOutput::MDPrintln("Attempting to fix procID:",procID);
  m_taskManager = new WINMXTaskManager(procID,m_nameGenerator);
  m_taskManager->SetID();
  delete(m_taskManager);
  m_taskManager=NULL;
  result = true;
  return result;
}
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
int WINMXIdentificationManager::GenerateUniqueID(char* GeneratedId)
{
  int result = -1;
  return result;
}
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
 


 