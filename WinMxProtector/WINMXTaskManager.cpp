/*-----------------------------------------------------------------------------
Class Name:  WINMXTaskManager
Purpose:  This class will excute tasks that we would like to perform on 
our WinMXProcessIDs;
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "WINMXTaskManager.h"
#include "ProcessUtils.h"
#include "MDSysFileControlWrapper.h"
//#include "MDConsoleOutput.h"
//#include "commctrl.h"
//#include <vector>
#include "MDMemoryEditor.h"
//using namespace std;
/*----------------------------PREPROCESSOR-------------------------------------*/
#define WINMXSEARCHXPOINT 334    //Cords where the search button is located
#define WINMXSEARCHYPOINT 37     //Cords where the search button is located

#define WINMXSHAREDXPOINT 150    //Cords where the shared button is located
#define WINMXSHAREDYPOINT 37     //Cords where the shared button is located

#define WINMXSETUPSHAREDXPOINT 70//Cords where the setupshared button is located
#define WINMXSETUPSHAREDYPOINT 10 //Cords where the setupshared button is located

#define WINMXREFRESHSHAREDXPOINT 334//Cords where the refresh shared button is located
#define WINMXREFRESHSHAREDYPOINT 60 //Cords where the refresh shared button is located
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
/*-----------------------------------------------------------------------------
Function Name: WINMXTaskManager()
Arugments:void
Returns:  void
Access:   Public
Purpose:  Contructor
Created:  3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WINMXTaskManager::WINMXTaskManager(DWORD procID, MDRandomNameGenerator *rng)
{
  m_processID           = procID;
  m_winmxWindowLocater  = new WINMXWindowLocater(m_processID);
  m_nameGenerator       = rng;
}  
/*-----------------------------------------------------------------------------
Function Name:~WINMXTaskManager()
Arugments:void
Returns: void
Access:  Public
Purpose: Destructor
Created: 3/12/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WINMXTaskManager::~WINMXTaskManager()
{
  delete m_winmxWindowLocater;
  m_winmxWindowLocater  = NULL;
}
/*-----------------------------------------------------------------------------
Function Name: GetReadyForSearch()
Arugments:void
Returns:  bool - true if get it going
Access:   Public
Purpose:  To send the mouse click to winmx to get it into search mode
Created:  3/16/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXTaskManager::GetReadyForSearch(HANDLE parent)
{
  bool result = false;


  Sleep(2000);
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: SetupSharedFolder()
Arugments:void
Returns:  bool - true if gets set
Access:   Public
Purpose:   
Created:  3/16/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
 
/*-----------------------------------------------------------------------------
Function Name: GetReadyForSearch()
Arugments:void
Returns:  bool - true if get it going
Access:   Public
Purpose:  To send the mouse click to winmx to get it into search mode
Created:  3/16/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXTaskManager::GetReadyForSharedFileSetup(HANDLE parent)
{
  bool result = false;
//********************************************************************************
//1. Step one press the "Shared Files" button, which really isn't a button control,
//just a mapped image
  DWORD point=0;
  //y goes in the high part of the word
  point = WINMXSHAREDYPOINT;
  //shift this action up like george and wezzy
  point = point << 16;
  //now OR it for the lower part of the word where the x cord goes
  point = point | WINMXSHAREDXPOINT;
  //send the message
  result = SendMessage((HWND)parent,WM_LBUTTONDOWN,MK_LBUTTON, (LPARAM)point);
  Sleep(2000);
  
  SetupSharedFolder();
//*********************************************************************************
//2. Step two, press the "Share Folder" button, which again, really isn't one
  /*point=0;
  //y goes in the high part of the word
  point = WINMXSETUPSHAREDYPOINT;
  //shift this action up like george and wezzy
  point = point << 16;
  //now OR it for the lower part of the word where the x cord goes
  point = point | WINMXSETUPSHAREDXPOINT;
  //send the message
  //result = SendMessage((HWND)parent,WM_LBUTTONDOWN,MK_LBUTTON, (LPARAM)point);
 
  Sleep(2000);*/
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: SetID()
Arugments:void
Returns:  bool - true if we set the user ID
Access:   Public
Purpose:  To wrap up the functionality of changing a user ID
Created:  3/11/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXTaskManager::SetID()
{
  bool result = false;
  
  int WINMXMODE = 0x01;
  if(m_winmxWindowLocater!=NULL)
  { 
    int requiredStepsForTask = 4;
    WindowHandleInfoStruct tempStruct;
    int Namecount=0;

    //temporary test names we'll use
    char IdNameToUse[50];

    //the controls that we need for this procedure
    char* commandNeeds[] =  {
                              kWINMX_CHANGE_ID_STRING,              
                              kWINMX_PEER_NETWORK_ID_EDIT_BOX,   
                              kWINMX_PEER_NETWORK_ID_EDIT_OK,
                              kWINMX_NETWORK_REFRESH
                            };

    //step through the three steps to creating a new user id
    for(int step=0;step<requiredStepsForTask;step++)
    {
      //1.Get the control handle of the object we need
      result = m_winmxWindowLocater->GetControlHandle(commandNeeds[step],&tempStruct,WINMXMODE);
      //2.if we get it proceed
      if(result)
      {
        switch(step)
        {
          //step 0: Click the change button, creating the dialog for us to change it's id       
          case 0:
            {
              //MDConsoleOutput::MDPrintln("step 0: Click the change button");
              //press this click button
              PostMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
              Sleep(1000);
            }
            break;

          //step 1: Set the edit box to our new User Id we wish to use
          case 1:
            {
              //MDConsoleOutput::MDPrintln("step 1: Set the edit box" );
              bool result = m_nameGenerator->GetRandomName(IdNameToUse);
              if(result)
              {
                SendMessage(tempStruct.m_processHandle,WM_SETTEXT,strlen(IdNameToUse),(LPARAM)IdNameToUse);
              }
            }
            break;
          //step 2: PressOK so that we can get the show back on the road
          case 2:
            {
              //MDConsoleOutput::MDPrintln("step 2: PressOK");
              SendMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
              Sleep(500);
            }
            break;
          //step 3: Press the Network refresh button to update session
          case 3:
            {
              //MDConsoleOutput::MDPrintln("step 3: Press the Network refresh button");
              //click the button
              SendMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
              //minimize the window
            }
            break;
          //default
          default:
            break;
        }//end switch
      }//end if
      else 
      {
        //since we're not getting what we need, maybe we should kill the process
        ProcessUtils::KillProcess(tempStruct.m_processID);
        //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"Failed to Change ID");
        break;
      }
    }//end for
    ///GetReadyForSearch(tempStruct.m_parent);
    //Search("asdf","asdf");
    m_parentHandle = tempStruct.m_parent;
  }//end if
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: Search(char* Artist,char* Title)
Arugments:char* Artist,char* Title
Returns:  bool - true if we set the user ID
Access:   Public
Purpose:  To wrap up the functionality of searching for a title
Created:  3/16/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXTaskManager::Search(char* Artist,char* Title)
{
  bool result = false;
  char* name = "50 cent";
  char* title = "Get Rich Or die trying";
  int WINMXMODE = 0x02;
  HANDLE header;

  if(m_winmxWindowLocater!=NULL)
  { 
    int requiredStepsForTask = 6;
    WindowHandleInfoStruct tempStruct;
    int Namecount=0;

    //the controls that we need for this procedure
    char* commandNeeds[] =  {              
                              kWINMX_ARTIST_EDIT_BOX,   
                              kWINMX_TITLE_EDIT_BOX,
                              kWINMX_SEARCH_COMMAND_BUTTON,
                              kWINMX_SEARCH_STOP_BUTTON,
                              kWINMX_SEARCH_HEADER_LIST_BOX,
                              kWINMX_SEARCH_LIST_BOX
                            };

    //step through the three steps to creating a new user id
    for(int step=0;step<requiredStepsForTask;step++)
    {
      //1.Get the control handle of the object we need
      result = m_winmxWindowLocater->GetControlHandle(commandNeeds[step],&tempStruct,WINMXMODE);
      //2.if we get it proceed
      if(result)
      {
        switch(step)
        {
        case 0:
          {
            //SendMessage(tempStruct.m_processHandle,WM_SETTEXT,strlen(name),(LPARAM)name);
          }
          break;
        case 1:
          {
            //SendMessage(tempStruct.m_processHandle,WM_SETTEXT,strlen(title),(LPARAM)title);
          }
          break;
        case 2:
          {
            //click the button
            //PostMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
          }
          break;
        case 3:
          {
            //Sleep(10000);
            //click the button
            //PostMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
          }
          break;
        case 4:
          {
            header = tempStruct.m_processHandle;
          }
          break;
        case 5:
          {
            
 	          // Get the number of columns in the search return
	          /*MDSysListControlWrapper listControl((HWND)tempStruct.m_processHandle,header,m_processID);
            int num_columns = listControl.GetListCount(); 
            MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"NumberOfEntries:",(int)num_columns);
            vector<string> data;
            listControl.GetListData(data);*/
          }
          break;
        default:
          break;
        }//end of switch
      }//end of result
      else
      {
        //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"We !@#$#CkEd!up getting list info");
      }//end of else
    }//end of for
  }//end of (m_winmxWindowLocater!=NULL)
  return result;
}

/*-----------------------------------------------------------------------------
Function Name: SetupSharedFolder()
Arugments:void
Returns:  bool - true if gets set
Access:   Public
Purpose:   
Created:  3/16/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool WINMXTaskManager::SetupSharedFolder()
{
  bool result = false;
  int WINMXMODE = 0x03;
//  HANDLE header;

  if(m_winmxWindowLocater!=NULL)
  { 
    int requiredStepsForTask = 3;
    WindowHandleInfoStruct tempStruct;
    int Namecount=0;

    DWORD point=0;
    //y goes in the high part of the word
    point = WINMXSHAREDYPOINT;
    //shift this action up like george and wezzy
    point = point << 16;
    //now OR it for the lower part of the word where the x cord goes
    point = point | WINMXSHAREDXPOINT;
    //send the message
    result = SendMessage((HWND)m_parentHandle,WM_LBUTTONDOWN,MK_LBUTTON, (LPARAM)point);
    Sleep(2000);
  

    //the controls that we need for this procedure
    char* commandNeeds[] =  {  kWINMX_SETUP_SHARE_COMPONENT,
                               kWINMX_SETUP_SHARE_DIALOG,
                               kWINMX_PEER_NETWORK_ID_EDIT_OK };



    //step through the three steps to creating a new user id
    for(int step=0;step<requiredStepsForTask;step++)
    {
      //1.Get the control handle of the object we need
      result = m_winmxWindowLocater->GetControlHandle(commandNeeds[step],&tempStruct,WINMXMODE);
      //2.if we get it proceed
      if(result)
      {
        switch(step)
        {
        case 0:
          {
          point=0;
          //y goes in the high part of the word
          point = WINMXSETUPSHAREDYPOINT;
          //shift this action up like george and wezzy
          point = point << 16;
          //now OR it for the lower part of the word where the x cord goes
          point = point | WINMXSETUPSHAREDXPOINT;
          //send the message
          result = PostMessage((HWND)tempStruct.m_processHandle,WM_LBUTTONDOWN,MK_LBUTTON, (LPARAM)point);
          Sleep(3000);
          }
          break;
        case 1:
        {
          //create teh FileControlWrapper to control the app
          MDSysFileControlWrapper fw(m_processID,(HWND)tempStruct.m_processHandle);
          //out TVITEM which we pass in and ultamately feed in to the Selector to select
          TVITEM item;
          item.pszText = new char[1024];
          char* blah = "";
          
          if(fw.SearchForItemNamed(blah,&item))
            fw.SelectItem(&item); 

          delete[] item.pszText;
          //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"We found brose");
        }
        break;
        case 2:
        {
          //MDConsoleOutput::MDPrintln("step 2: PressOK");
          SendMessage(tempStruct.m_processHandle,BM_CLICK,0,0);
          Sleep(5000);
        }        
        break;
        default:
          break;
        }//end of switch
      }//end of result
      else
      {
        //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"We !@#$#CkEd!up getting list info");
      }//end of else
    }//end of for

    ShowWindow((HWND)m_parentHandle,SW_SHOWMINNOACTIVE);//SW_MINIMIZE);
  }//end of (m_winmxWindowLocater!=NULL) 

return result;
}