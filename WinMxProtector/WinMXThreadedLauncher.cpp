/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "WinMXThreadedLauncher.h"
#include "MDFileReaderWriter.h"
//#include "MDConsoleOutput.h"
//#define DEGUG
/*-----------------------------GLOBAL------------------------------------------*/
DWORD WINAPI LauncherExecuterThread(void *);
void debug(char * statement,DWORD value = 0);

/*-----------------------------------------------------------------------------
Function Name: WinMXThreadedLauncher(int numOfProcsToLaunch)
Arugments: int-number of instances to launch
Returns:   void    
Access:    Public
Purpose:   Constructor 
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WinMXThreadedLauncher::WinMXThreadedLauncher()
{
  m_processUtils      = new ProcessUtils();
  m_winmxMemAdjuster  = new WinMXMemoryAdjust();
  m_threadID          = 0;
  m_numberOfProcsLaunched = 0;
  m_running           = false;
  m_launcherExecuterThreadHandle = NULL;
  m_appName           = new char[MAXNAMELENGTH];
  m_appPath           = new char[MAXNAMELENGTH];
  m_currentProcToFix  = NULL;
}

/*-----------------------------------------------------------------------------
Function Name:~WinMXThreadedLauncher()
Arugments: void
Returns:   void
Access:    Public
Purpose:   Destructor
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WinMXThreadedLauncher::~WinMXThreadedLauncher()
{
  //make sure our thread stops
  Stop();

  delete m_processUtils;
  delete m_winmxMemAdjuster;
  delete[] m_appName;
  delete[] m_appPath;
  //clean up
  m_threadID        = NULL;
  m_numberOfProcsLaunched = NULL;
  m_running         = false;
  m_launcherExecuterThreadHandle = NULL;
  m_processUtils    = NULL;
  m_winmxMemAdjuster= NULL;
  m_appName         = NULL;
  m_appPath         = NULL;
};

/*-----------------------------------------------------------------------------
Function Name: IsOK
Arugments: tells us if things were inited alright
Returns:   bool - true if we're ready to go
Access:    Public
Purpose:   to give us status 
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::IsOK()
{
  bool result = false;

  if(m_processUtils != NULL)
  {
    if(m_winmxMemAdjuster != NULL)
    {
      if((m_numberOfProcsToLaunch > 0) && ( m_numberOfProcsToLaunch < MAXNUMBEROFINSTANCES))
      {
        result = true;
      }//end if
    }//end if
  }//end if

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: IsRunning()
Arugments: void
Returns:   bool - true if thread is busy
Access:    Public
Purpose:   to give us thread status 
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::IsRunning()
{
  return m_running;
}
/*-----------------------------------------------------------------------------
Function Name: SetNumberToLaunch(int numOfProcsToLaunch)
Arugments: int-number of instances to launch
Returns:   void    
Access:    Public
Purpose:   Constructor 
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::SetNumberToLaunch(int numOfProcsToLaunch)
{
  bool result = false;
  if(numOfProcsToLaunch > 0)
  {
    m_numberOfProcsToLaunch = numOfProcsToLaunch;
    debug("Number of Procs Requested to Launch",m_numberOfProcsToLaunch);
    result = true;
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: SetPathOfApp(char* appName,char* appPath)
Arugments: char* application name, char* application directory
Returns:   void    
Access:    Public
Purpose:     
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::SetPathOfApp(char* appName,char* appPath)
{
  bool result = false;
  
  if(appName && appPath)
  {
    memcpy(m_appName,appName,MAXNAMELENGTH);
    memcpy(m_appPath,appPath,MAXNAMELENGTH);
    result = true;
  }

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: GetListOfLaunchedInstances()
Arugments: void
Returns:   vector<DWORD> , list of procs running as WINMX
Access:    Public
Purpose:   to tell us what we've launched
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
vector<DWORD> WinMXThreadedLauncher::GetListOfLaunchedInstances()
{
  vector<DWORD> temp;
  return temp;
}
/*-----------------------------------------------------------------------------
Function Name: Start()
Arugments: void
Returns:   bool, true if we start our thread
Access:    Public
Purpose:   public interface to start the work
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::StartLaunch()
{ 
  bool result = false;
  m_currentProcToFix=0;
  m_numberOfProcsLaunched= 0;

  if(IsOK())
  {
    result = StartThread();
  }

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Stop()
Arugments: void
Returns:   bool, true if we start our thread
Access:    Public
Purpose:   public interface to start the work
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::Stop()
{ 
  bool result = true;
  //set the stop flag for the thread
  m_running = false;
  CloseHandle(m_launcherExecuterThreadHandle);
  return result;
}/*-----------------------------------------------------------------------------
Function Name: LaunchWinMXInstance()
Arugments: void
Returns:   DWORD - the process id of the instance launched
Access:    Private
Purpose:   to kick off an instance of the app
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD WinMXThreadedLauncher::LaunchWinMXInstance()
{
  //path of where we are launching from
  //char* apptolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\Winmx.exe";
  //char* dirtolaunch = "C:\\FileSharingPrograms\\WinMX\\WinMX\\";
  //char* settingsFile = "C:\\FileSharingPrograms\\WinMX\\WinMX\\settings.dat";
  char* settingsFile = "C:\\Program Files\\WinMX\\settings.dat";

  DWORD procID = NULL;
  MDFileReaderWriter testToSeeifAppIsUsingFile;
  int numOftrys = 0;

  //wait for use to be able to access settings.dat
  while(testToSeeifAppIsUsingFile.IsFileBusy(settingsFile))
  {
    Sleep(20); 
    //MDConsoleOutput::Println(("**************************File is being used");
    if(numOftrys++>40)
    {
     // MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"We failed a luanch");
      m_running =  false;
      return NULL;
    }
  } 
  procID = m_processUtils->LaunchApplication(m_appName,m_appPath,TIMETOSLEEPAFTERLAUNCH);

  if(procID != NULL)
  {
    m_numberOfProcsLaunched++;
  }

  return procID;
 }
/*-----------------------------------------------------------------------------
Function Name: StartThread()
Arugments: void
Returns:   bool - true if we launched successfully
Access:    Public
Purpose:   to kick off our worker thread
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::KillInstance(DWORD procID)
{
  return true;
}
/*-----------------------------------------------------------------------------
Function Name: StartThread()
Arugments: void
Returns:   bool - true if we launched successfully
Access:    Public
Purpose:   to kick off our worker thread
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WinMXThreadedLauncher::StartThread()
{
  bool result = false;
  //flag thread as busy

  m_running = true;
  //launch our workter thread
  m_launcherExecuterThreadHandle 
                      = CreateThread( 
                        NULL,                       // no security attributes 
                        0,                          // use default stack size  
                        LauncherExecuterThread,     // thread function 
                        this,                       // argument to thread function 
                        NORMAL_PRIORITY_CLASS,      // use default creation flags 
                        &m_threadID);               // returns the thread identifier 

  if(m_launcherExecuterThreadHandle!=NULL)
  {
    result = true;
  }

  //if we're just getting started, launch an instance to fix
  m_currentProcToFix=0;
  if(m_numberOfProcsLaunched == 0)
  {
    m_currentProcToFix = LaunchWinMXInstance();
  }

  return result;
}
/*-----------------------------------------------------------------------------
Function Name: GetNumberOfRunningWinMXs()
Arugments: void
Returns:   int - number of running instances
Purpose:   to check to see if we have enough instances running
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
int WinMXThreadedLauncher::GetNumberOfRunningWinMXs()
{
  int result = 0;
  return result;
}

/*-----------------------------------------------------------------------------
Function Name: LauncherExecuterThread()
Arugments: void
Returns:   DWORD - exiting value of thread
Purpose:   This is our Worker Thread
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
DWORD WINAPI LauncherExecuterThread(void *pData)
{
  WinMXThreadedLauncher *pworkingData = (WinMXThreadedLauncher*)pData;

	debug("WinMXThreadedLauncher::LauncherExecuterThread()");
 
  //our local data
  ProcessNameIDStruct nameID;

  //main working loop
  while(pworkingData->m_running)
	{
    try
		{  
      //do we have an instance we need to fix
      if(pworkingData->m_currentProcToFix != NULL)
      {
       // debug("valid ProcID",currentProcToFix);
        bool flag = pworkingData->m_processUtils->GetProcInfo(pworkingData->m_currentProcToFix,&nameID);
        
        if(flag)
        {
          //if process is named WinMx try to rename its mutex
          debug("Fixing a new WinMx%d\n",(DWORD)pworkingData->m_currentProcToFix);		  
          pworkingData->m_winmxMemAdjuster->FixWinMXFlag((DWORD)pworkingData->m_currentProcToFix,nameID.m_baseAddr);
          debug("Fixed a new WinMx%d\n",(DWORD)pworkingData->m_currentProcToFix);
          pworkingData->Stop();
        }//end of if(flag)
        else 
        {
          Sleep(20);
          debug("Failed to get proc Info");
        }
      }//end of procToFix
    }//end of try
		catch( char *str )
		{
			debug(str);
		}//end of catch

	}//end of while
  debug("Thread Is done working");
	return 0;
}

/*-----------------------------------------------------------------------------
Function Name: debug()
Arugments: void
Returns:   void
Purpose:   to debug
Created:   3/4/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
void debug(char* statement, DWORD value)
{
#ifdef DEGUG
  if(statement != NULL)
  {
    if(value != 0)
    {
    //  MDConsoleOutput::Println(MDConsoleOutput::SERVERITY_ERROR,statement,value);
    }
    else
    {
     // MDConsoleOutput::Println(MDConsoleOutput::SERVERITY_ERROR,statement);
    }
  }//endif
#endif
}
 