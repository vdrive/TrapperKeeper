/*----------------------------PREPROCESSOR-----------------------------------*/
/*-------------------------------INCLUDE-------------------------------------*/
#include "ProcessUtils.h"
#include "ProcessHandler.h"
//#include "MDConsoleOutput.h"
#include <psapi.h>
/*---------------------------PUBLIC FUNCTIONS--------------------------------*/
void GetError();
/*-----------------------------------------------------------------------------
Function Name: ProcessUtils()
Arugments: void
Returns:   void
Purpose:   Constructor
Created:2/25/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
ProcessUtils::ProcessUtils()
{
}

/*-----------------------------------------------------------------------------
Function Name: ~ProcessUtils()
Arugments: void
Returns:   void
Purpose:   Destructor
Created:   2/25/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
ProcessUtils::~ProcessUtils()
{
}
/*-----------------------------------------------------------------------------
Function Name: GetNumberOfIntancesRunning()
Arugments: char* - name of process that we want to find
Returns:   int - number of instances running
Purpose:   to find the number of instances of a certain application name
Created:   3/4/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
int ProcessUtils::GetNumberOfIntancesRunning(char* nameOfProc)
{
  ProcessHandler ph;
  int numOfInstances=0;
	DWORD ids[1000];
	HMODULE modules[1000];
  DWORD count,nmod;
  EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until winmx shows up
	count/=(sizeof(DWORD));

  for(int i=0;i<(int)count;i++)
  {
    HANDLE handle=ph.GetProcessHandle((DWORD)ids[i]);//OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		
    if(handle != NULL)
    {
      EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		  nmod/=(sizeof(HMODULE));

		  if(nmod>0)
		  {
		    char name[100];
			  GetModuleBaseName(handle,modules[0],name,99);
  		
			  if(stricmp(nameOfProc,name)==0)
        {
          numOfInstances++;
        }
      }//end if null
      CloseProcessHandle(&handle);
    }//end if
  }//end of for

  return numOfInstances;
}
/*-----------------------------------------------------------------------------
Function Name: GetNamesOfProcs(DWORD procID, ProcessNameIDStruct *name)
Arugments: DWORD - proc id, ProcessNameIDStruct * - name of proc
Returns:   bool if proc is found
Purpose:   To return the name of the specified proc id
Created:   2/26/04   p. faby
Modified:
-----------------------------------------------------------------------------*/
bool ProcessUtils::GetProcInfo(DWORD procID, ProcessNameIDStruct *pnInfo)
{
  bool result = false;
  char szProcessName[MAX_PATH] = "unknown";
  ProcessHandler ph;
  HANDLE handle = ph.GetProcessHandle((DWORD)procID);

  // Get the process name.
  if ( handle != NULL)
  {
    HMODULE hMod[1000];
    DWORD cbNeeded;

    if(EnumProcessModules( handle, hMod, sizeof(HMODULE)*1000, &cbNeeded))
    {
      if(GetModuleBaseName( handle, hMod[0], szProcessName,sizeof(szProcessName) ))
      {
        pnInfo->m_baseAddr = (DWORD)hMod[0];
        pnInfo->m_procID   = procID;
        
        //MDConsoleOutput::Println(("Number of modules found :%d\n",(cbNeeded /sizeof(HMODULE)));
        //MDConsoleOutput::Println(("Name of BaseModule:%s\n",szProcessName);
        memcpy(pnInfo->m_procName,szProcessName,MAXNAMELENGTH);
        CloseProcessHandle(&handle);
        handle=0;
        result = true;
       }
     }
  }
  else
  {
    DWORD error = GetLastError();
   // MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"ProcessUtils::ProcID:",procID);
  }
  return result;
}

void GetError()
{
    DWORD error = GetLastError();
    //MDConsoleOutput::MDPrintln("ProcessUtils Error:",error);
}
/*-----------------------------------------------------------------------------
Function Name: GetProcessHandle(DWORD prodID)
Arugments:DWORD- procID
Returns:  HANDLE - handle to the process
Access:   Public
Purpose:  To get a unique process id's handle
Created:  3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
HANDLE ProcessUtils::GetProcessHandle(DWORD prodID)
{
  HANDLE result = NULL;
  ProcessHandler ph;
  result = ph.GetProcessHandle(prodID);
  return result;
}
void ProcessUtils::CloseProcessHandle(HANDLE* procHandle)
{
  if(procHandle!=NULL)
    CloseHandle(*procHandle);
}
/*-----------------------------------------------------------------------------
Function Name: LaunchApplication(const char* appName,const char* workingDir,int mills)
Arugments:char* appName, char* workingDir, int mills to wait after launching
Returns:  DWORD - the process id of created app, 0 - if failed
Access:   Public
Purpose:  to be a friendly handler to launch apps
Created:  3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD ProcessUtils::LaunchApplication(const char* appName,const char* workingDir,int mills)
{
  DWORD result = 0;
  ProcessHandler ph;
  result = ph.LaunchProcess(appName,workingDir,mills);
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: KillProcess(DWORD procID)
Arugments:DWORD procID of app to kill
Returns: bool - true if we were successful
Access:  Public
Purpose: To be a friendly process killer
Created: 3/5/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool ProcessUtils::KillProcess(DWORD procID)
{
  bool result = false;
  ProcessHandler ph;
  result = ph.KillProcess(procID);
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
int ProcessUtils::GetProcessMemUsage(DWORD procID)
{
  bool result = false;
  PROCESS_MEMORY_COUNTERS pmc[100];
  DWORD size=sizeof(PPROCESS_MEMORY_COUNTERS)*100;

  result = GetProcessMemoryInfo(GetProcessHandle(procID),pmc,size);

  return (int)pmc[0].WorkingSetSize;
}
/*-----------------------------------------------------------------------------
Function Name:
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
DWORD ProcessUtils::ReadProcMemory(HINSTANCE hInst,
                                    void* offset, unsigned char* dataBuf,  DWORD sizeToRead)
{
  DWORD result = 0;

  if(hInst!=NULL)
  {
    if(offset>=0)
    {
      if(sizeToRead>0)
      {
        if(dataBuf!=NULL)
        {
          ReadProcessMemory(hInst,(void*)(offset), dataBuf, sizeToRead, &result);
          //MDConsoleOutput::Println(("Ins:%d        Offset:%d     SizeToRead:%d   SizeRead:%d\n",hInst,offset,sizeToRead,result);
        }
      }
    }
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
bool ProcessUtils::FlushMemory(HINSTANCE hInst,void* offset, DWORD sizeToFlush)
{
  bool result = false;
  if(hInst!=NULL)
  {
    if(offset>0)
    {
      if(sizeToFlush>0)
      {
        result = FlushInstructionCache(hInst,(void*)(offset),sizeToFlush);
      }
    }
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
bool ProcessUtils::WriteProcMemory(HINSTANCE hInst,
                                    void* offset, unsigned char* writeBuf,DWORD sizeToWrite)
{
  bool result = false;
  DWORD bytesWritten=0;
  if(hInst!=NULL)
  {
    if(offset>0)
    {
      if(sizeToWrite>0)
      {
        if(writeBuf!=NULL)
        {
          WriteProcessMemory(hInst,(void*)(offset),writeBuf,sizeToWrite,&bytesWritten);
        }
      }
    }
  }

  if(bytesWritten == sizeToWrite)
  {
    result = true;
  }
 
  return result;
}

/*-----------------------------------------------------------------------------
Function Name:UnitTest()
Arugments: void
Returns:   void
Purpose:   To test our class and sub classes
Created:   2/26/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
void ProcessUtils::UnitTest()
{
/*  ProcessNameIDStruct nameID;
  bool result=false;
  char* apptolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\Winmx.exe";
  char* dirtolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\";
  PROCESS_INFORMATION pi;
  ProcessHandler pl;
  int procID = pl.LaunchProcess(apptolaunch,dirtolaunch,&pi);
  int t =  GetProcInfo(procID,&nameID);
  Sleep(1000);
  CloseHandle(nameID.m_procInfo.hProcess);*/
} 

 