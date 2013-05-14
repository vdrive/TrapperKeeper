 /*-----------------------------------------------------------------------------
Class Name: WINMXWindowLocater
Purpose:  
Created:  
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
/*------------------------------INCLUDE----------------------------------------*/
#include "WINMXWindowLocater.h"
//#include "MDConsoleOutput.h"
/*-------------------------------GLOBAL----------------------------------------*/
HWND parent;
int Namecount=0;
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
/*-----------------------------------------------------------------------------
Function Name: WINMXWindowLocater()
Arugments: void
Returns: void
Access:  Public
Purpose: Contructor
Created: 2/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WINMXWindowLocater::WINMXWindowLocater(DWORD procID)
{
  m_currentProcId = procID;
}
/*-----------------------------------------------------------------------------
Function Name: ~WINMXWindowLocater()
Arugments: void
Returns: void
Access:  Public
Purpose: Destructor
Created: 2/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WINMXWindowLocater::~WINMXWindowLocater()
{
}
/*-----------------------------------------------------------------------------
Function Name: ~WINMXWindowLocater()
Arugments: void
Returns: void
Access:  Public
Purpose: Destructor
Created: 2/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXWindowLocater::InitOurControlData()
{
  bool result = false;
 
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: FindWINMXControls(WindowHandleInfoStruct dataArray[MAXNUMOFPROCS],int numberToSearch)
Arugments: WindowHandleInfoStruct dataArray[MAXNUMOFPROCS] int numberToSearch
Returns: void
Access:  Public
Purpose: to locate the controls that we want to hijack
Created: 2/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXWindowLocater::FindWINMXControls(WindowHandleInfoStruct dataArray[MAXNUMOFPROCS],
                                           int numberToSearch,int winMXState)
{
  bool result = false;
  //if we didn't get any suggestion lets not look
  if(numberToSearch > 0)
  {
    int count=0;
    int specialEditboxCase = 1;
    char* array[]={ 
                    kWINMX_APP_STRING,
                    kWINMX_NETWORKS_COMMAND_BUTTON,      
                    kWINMX_CHANGE_ID_STRING,              
                    kWINMX_NETWORK_DISCONNECT,            
                    kWINMX_NETWORK_CONNECT,              
                    kWINMX_PEER_NETWORK_ID_EDIT_DIALOG,   
                    kWINMX_PEER_NETWORK_ID_EDIT_OK,    
                    kWINMX_PEER_NETWORK_ID_EDIT_CANCEL, 
                    kWINMX_PEER_NETWORK_ID_EDIT_BOX,
                    kWINMX_SEARCH_COMMAND_BUTTON,
                    kWINMX_NETWORK_REFRESH,
                    kWINMX_ARTIST_EDIT_BOX,  
                    kWINMX_TITLE_EDIT_BOX,
                    kWINMX_USERID_EDIT_BOX,
                    kWINMX_SEARCH_STOP_BUTTON,
                    kWINMX_SEARCH_LIST_BOX,
                    kWINMX_SEARCH_HEADER_LIST_BOX,
                    kWINMX_SETUP_SHARE_COMPONENT,
                    kWINMX_SETUP_SHARE_DIALOG
                  };
 
    for(int index=0;index<numberToSearch;index++)
    {
      //get the current name for the window we are inspecting
      const char* inspectName = dataArray[index].m_windowName;

      //switch to fix buttons 
      char temp[100];
      GetClassName(dataArray[index].m_processHandle,temp,100);
      //MDConsoleOutput::Println(("Looking For: %s \n ",inspectName);
      BOOL enabled = IsWindowEnabled(dataArray[index].m_processHandle);
      BOOL isWindow = IsWindow(dataArray[index].m_processHandle);
      BOOL isWindowVisible = IsWindowVisible(dataArray[index].m_processHandle);
     // MDConsoleOutput::Println(("Button Control:%s       ",inspectName);
     // MDConsoleOutput::Println(("HWND:0x%x \n",dataArray[index].m_processHandle);
     // MDConsoleOutput::Println(("Enabled: %d    IsWindow: %d     IsWindowVisible: %d\n \n ",enabled,isWindow,isWindowVisible);
      if(enabled && isWindow && isWindowVisible)
      { 
        //if this control is a button
        if(strcmp(temp,"Button")==0)  
        {
          for(int names=0;names<kWINMX_NUM_MAIN_COMPONENTS;names++)
          { 
            if(strcmp(inspectName,array[names])==0)
            {
              m_controlList[names].m_processHandle  = dataArray[index].m_processHandle;
              m_controlList[names].m_processID      = dataArray[index].m_processID;
              m_controlList[names].m_threadID       = dataArray[index].m_threadID;
              m_controlList[names].m_parent         = parent;
              memcpy(m_controlList[names].m_windowName,inspectName,kMAXPROCWINDOWNAME); 
            }//end if
          }//end of for
        }
        //else if this control is an Edit control
        else if(strcmp(temp,"Edit")==0) 
        {
          int base = 9;                
//MDConsoleOutput::Println("Found Edit");

//HACK TO DETER THE DIFFERENCE BETWEEN THE NETWORK ID SEARCH & THE ARTIST 
          switch(winMXState)
          {
            //if one we are searching for the edit name id
          case 1:
            {
              //first time we find this set our internal name for it as USERIDEDIT;
              m_controlList[base].m_processHandle  = dataArray[index].m_processHandle;
              m_controlList[base].m_processID      = dataArray[index].m_processID;
              m_controlList[base].m_threadID       = dataArray[index].m_threadID;
              memcpy(m_controlList[base].m_windowName,
                      kWINMX_PEER_NETWORK_ID_EDIT_BOX,kMAXPROCWINDOWNAME);
            }
            break;
            //if we are here we are searching for artist edit box and title box
          case 2:
            {
              //first time we find this set our internal name for it as USERIDEDIT;
              m_controlList[base+specialEditboxCase].m_processHandle  = dataArray[index].m_processHandle;
              m_controlList[base+specialEditboxCase].m_processID      = dataArray[index].m_processID;
              m_controlList[base+specialEditboxCase].m_threadID       = dataArray[index].m_threadID;
              if(specialEditboxCase==1)
              {
                memcpy(m_controlList[base+specialEditboxCase].m_windowName,
                  kWINMX_ARTIST_EDIT_BOX,kMAXPROCWINDOWNAME);
              }
              else
              {
                memcpy(m_controlList[base+specialEditboxCase].m_windowName,
                  kWINMX_TITLE_EDIT_BOX,kMAXPROCWINDOWNAME);
              }
              specialEditboxCase++;
            }
          break;

          default:
            break;
          }


          //MDConsoleOutput::Println(("Found the Edit Control Named:%s\n",inspectName);
        }
        else if(strcmp(temp,"SysListView32")==0)
        {
          m_controlList[15].m_processHandle  = dataArray[index].m_processHandle;
          m_controlList[15].m_processID      = dataArray[index].m_processID;
          m_controlList[15].m_threadID       = dataArray[index].m_threadID;
          m_controlList[15].m_parent         = parent;
          memcpy(m_controlList[15].m_windowName,kWINMX_SEARCH_LIST_BOX,kMAXPROCWINDOWNAME); 
 
        }//end else if
        else if(strcmp(temp,"SysHeader32")==0)
        {
          m_controlList[16].m_processHandle  = dataArray[index].m_processHandle;
          m_controlList[16].m_processID      = dataArray[index].m_processID;
          m_controlList[16].m_threadID       = dataArray[index].m_threadID;
          m_controlList[16].m_parent         = parent;
          memcpy(m_controlList[16].m_windowName,kWINMX_SEARCH_HEADER_LIST_BOX,kMAXPROCWINDOWNAME); 
        }
        else if(strcmp(temp,"Afx:400000:b:10011:0:0")==0)
        {
          m_controlList[17].m_processHandle  = dataArray[index].m_processHandle;
          m_controlList[17].m_processID      = dataArray[index].m_processID;
          m_controlList[17].m_threadID       = dataArray[index].m_threadID;
          m_controlList[17].m_parent         = parent;
          memcpy(m_controlList[17].m_windowName,kWINMX_SETUP_SHARE_COMPONENT,kMAXPROCWINDOWNAME); 
        }
        else if(strcmp(temp,"SysTreeView32")==0)
        {
          m_controlList[18].m_processHandle  = dataArray[index].m_processHandle;
          m_controlList[18].m_processID      = dataArray[index].m_processID;
          m_controlList[18].m_threadID       = dataArray[index].m_threadID;
          m_controlList[18].m_parent         = parent;
          memcpy(m_controlList[18].m_windowName,kWINMX_SETUP_SHARE_DIALOG,kMAXPROCWINDOWNAME);     
        }
        ////MDConsoleOutput::Println(("\n");
      }//end enabled check
    }//end of for
  }//end of if numberToSearch >= 0
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: GetControlHandle(char* ControlToGrab,WindowHandleInfoStruct* data)
Arugments: char* name of the component that we want, WindowHandleInfoStruct for 
data to copy into
Returns:  bool - true if we find the control
Access:   Public
Purpose:  To wrap up the functionality of finding windows
Created:  3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXWindowLocater::GetControlHandle(char* ControlToGrab,
                                          WindowHandleInfoStruct* data,int winMXState)
{
  bool result = false;

  if(ControlToGrab != NULL)
  {
    //looking for name
    //MDConsoleOutput::Println(("Searching for: %s \n",ControlToGrab);
    InitWINMXControls(winMXState);

    for(int index=0;index<kWINMX_NUM_MAIN_COMPONENTS;index++)
    { 
      if(strcmp(ControlToGrab,m_controlList[index].m_windowName) == 0)
      {
            data->m_processHandle  = m_controlList[index].m_processHandle;
            data->m_processID      = m_controlList[index].m_processID;
            data->m_threadID       = m_controlList[index].m_threadID;
            data->m_parent         = m_controlList[index].m_parent;
            memcpy(data->m_windowName,ControlToGrab,kMAXPROCWINDOWNAME);
            result = true;
            break;
      }//end of check
    //continue to look
    }//end of for
  }
  else
  {
    //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"Request char* == NULL");
  }
  return result;
}

/*------------------------------------------------------------------------------
Function Name: InitWINMXControls()
Arugments:void
Returns:  bool - true if we inited our list
Access:   Public
Purpose:  To wrap up the functionality of our tasks of initing windows
Created:  3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXWindowLocater::InitWINMXControls(int winMXState)
{
  int result = 0;

  //WindowsLocater call
  result = FindAllWindows();

  //do search based on procID
  WindowHandleInfoStruct tempArray[MAXNUMOFPROCS];
  WindowHandleInfoStruct childArray[MAXNUMOFPROCS];
  
  //lets only get data for our process ID
  result = DoSearch(m_currentProcId,tempArray);
  parent = (HWND)tempArray[0].m_processHandle;
  for(int i=0;i<result;i++)
  {
    FindAllChildren((HWND)tempArray[i].m_processHandle); 
    BOOL enabled = IsWindowEnabled((HWND)tempArray[i].m_processHandle);
    BOOL isWindow = IsWindow((HWND)tempArray[i].m_processHandle);
    BOOL isWindowVisible = IsWindowVisible((HWND)tempArray[i].m_processHandle);
    char name[100];
    //get the controls Text
    GetWindowText(tempArray[i].m_processHandle,name,kMAXPROCWINDOWNAME );   
    
    int count = GetListOfResults(childArray);
    //if(count>0)MDConsoleOutput::Println(("Looking for children of handle:%x\n",tempArray[i].m_processHandle);
    FindWINMXControls(childArray,count,winMXState);
  }
  return result;
}
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*-----------------------------------------------------------------------------
Function Name: GetControlHandles()
Arugments: void
Returns:   bool - true if we are succesful
Access:    Private
Purpose:   to wrap up the details of our work
Created:   3/8/04 p.faby
Modified:
-------------------------------------------------------------------------------*/  
bool WINMXWindowLocater::GetControlHandles()
{
  bool result = false;
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: DoSearch(DWORD procID,WindowHandleInfoStruct dataArray[MAXNUMOFPROCS])
Arugments: DWORD procId, process id we're interested in, 
            WindowHandleInfoStruct[] array to copy data to
Returns:   int number of results
Access:    public
Purpose:  
Created:   3/11/04 p.faby
Modified:
-------------------------------------------------------------------------------*/  
int WINMXWindowLocater::DoSearch(DWORD procID,WindowHandleInfoStruct dataArray[MAXNUMOFPROCS])
{
  bool result = false;

  WindowHandleInfoStruct tempArray[MAXNUMOFPROCS];
  int size = GetListOfResults(tempArray);

 // char nameBuf[kMAXPROCWINDOWNAME];
  DWORD threadID=0;
  DWORD processID=0;
  int count = 0;

  for(int index=0;index<size;index++)
  {
    if(tempArray[index].m_processID==procID)
    {
      dataArray[count].m_processHandle = tempArray[index].m_processHandle;
      dataArray[count].m_processID     = tempArray[index].m_processID ;
      dataArray[count].m_threadID      = tempArray[index].m_threadID ;
      memcpy(dataArray[count].m_windowName,tempArray[index].m_windowName,kMAXPROCWINDOWNAME);
      count++;
    }
  }
  return count;
}


 
