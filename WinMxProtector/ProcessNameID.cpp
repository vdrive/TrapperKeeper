/*----------------------------PREPROCESSOR-----------------------------------*/
/*-------------------------------INCLUDE-------------------------------------*/
#include "ProcessNameID.h"
/*-----------------------------------------------------------------------------
Function Name: ProcName()
Arugments: void
Returns: void
Purpose: Constructor
Created: 2/25/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
ProcessNameID::ProcessNameID(void)
{
  m_indexOfPairs = 0;
}
/*-----------------------------------------------------------------------------
Function Name: ~ProcNameID
Arugments: void
Returns: void
Purpose: to do clean up of the class
Created: 2/25/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
ProcessNameID::~ProcessNameID(void)
{
  //make sure we release the handles that we had copied
  //ReleaseHandles();
}
/*-----------------------------------------------------------------------------
Function Name: ReleaseHandles()
Arugments: void 
Returns:  void
Purpose: to release the handles that we had aquired
Created: 2/25/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
void ProcessNameID::ReleaseHandles()
{
  char blankname[MAXNAMELENGTH] = "no proc name";
  for(int i=0;i<m_indexOfPairs;i++)
  {
    //m_NameIDPairs[i].m_procHandle = 0;
    m_NameIDPairs[i].m_baseAddr = 0;
    m_NameIDPairs[i].m_procID = 0;
    memcpy((char*)m_NameIDPairs[i].m_procName,blankname,MAXNAMELENGTH);
  }
}
/*-----------------------------------------------------------------------------
Function Name: SetProcIDName(const DWORD procID,const char *procName, 
                             const DWORD baseAddr, const HANDLE handle)
Arugments: process ID number, name of process, base address, handle  
Returns:  bool - true if succesfull
Purpose: to place the data into our structure for future use
Created: 2/25/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool ProcessNameID::SetProcIDName(const DWORD procID,const char *procName, 
                               const DWORD baseAddr, const HANDLE handle)
{
  bool result = false;
 
  memcpy((char*)m_NameIDPairs[m_indexOfPairs].m_procName,procName,MAXNAMELENGTH);
  m_NameIDPairs[m_indexOfPairs].m_procID = procID;
  m_NameIDPairs[m_indexOfPairs].m_baseAddr = baseAddr;
  m_indexOfPairs++;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: GetProcNameID(const char* procName,NameID *nameID)
Arugments: name of process we want, NameID structure for use to copy data into
Returns: bool - true if successful
Purpose: to find the process by name and return info about the process
Created:
Modified:
-------------------------------------------------------------------------------*/
bool ProcessNameID::GetProcNameID(const char* procName,ProcessNameIDStruct *nameID)
{
  bool result = false;

  for(int index=0;index<m_indexOfPairs;index++)
  {
    //check to see if this is the process name that we are looking for
    if(!memcmp(procName,m_NameIDPairs[index].m_procName,sizeof(procName)))
    {//if it is, then we copy data into the struct
      memcpy(nameID,(ProcessNameIDStruct*)&m_NameIDPairs[index],sizeof(ProcessNameIDStruct));
      result = true;
      break;
    }
  }
  return result;
}