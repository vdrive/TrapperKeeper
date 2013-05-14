//Implemenation file for MDSysFileCotnrolWrapper
/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "MDSysFileControlWrapper.h"
#include "ProcessUtils.h"
//#include "MDConsoleOutput.h"
#include "MDMemoryEditor.h"
#include <vector>
using namespace std;
/*-----------------------------------------------------------------------------
Function Name:MDSysFileControlWrapper
Arugments: void
Returns: 
Access:  public
Purpose: Constructor
Created: 3/28/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDSysFileControlWrapper::MDSysFileControlWrapper(int processID,HWND browseHWND)
{
  if(processID > 0)
  {
    m_processID = processID;
    m_pMemoryEditor = new MDMemoryEditor(processID);
    m_processHandle = ProcessUtils::GetProcessHandle(m_processID);
    m_folderBorwseHWND = browseHWND;
  }
}
/*-----------------------------------------------------------------------------
Function Name:~MDSysFileControlWrapper
Arugments: void
Returns: 
Access:  public
Purpose: Destroy
Created: 3/28/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDSysFileControlWrapper::~MDSysFileControlWrapper()
{
  CloseHandle(m_processHandle);
  if(m_pMemoryEditor != NULL)
  {
    delete m_pMemoryEditor;
    m_pMemoryEditor    = NULL;
    m_folderBorwseHWND = NULL;
    m_processID        = NULL;
  }
}
/*-----------------------------------------------------------------------------
Function Name:SearchForItemNamed
Arugments: const char* dirTofind,TVITEM* searchItem
Returns: bool
Access:  public
Purpose: Complex Code....We're not acutally using the dirTofind right now...even though
in the future it might be nice to add this use.  This function basically finds the
FastTrack Shared folder and selects it
Created: 3/28/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDSysFileControlWrapper::SearchForItemNamed(const char* dirTofind,TVITEM* searchItem)
{
  bool result = false;
  vector<HTREEITEM> data;
  vector<TVITEM> pathFound;
  DWORD count = TreeView_GetCount((HWND)m_folderBorwseHWND);
  HTREEITEM tiStart = TreeView_GetRoot((HWND)m_folderBorwseHWND);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  HTREEITEM   tiCurrItem = TreeView_GetParent((HWND)m_folderBorwseHWND,tiStart);
  HTREEITEM   tiNextItem;
  TVITEM      foundItem;
  foundItem.pszText = new char[1024];
  
  int         iRoot = 1;
  int         iRet = 1;
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  int count1=0;
  do
  {
    //here we use the current itemtiCurrItem
    if(tiCurrItem != NULL)
    {
      data.push_back(tiCurrItem);
      GetTVITEM(&tiCurrItem,&foundItem);

      //MDConsoleOutput::MDPrintln(2,foundItem.pszText);

      char* strMyComputer   = "My Computer";
      char* strLocalDisk    = "Local Disk (C:)";
      char* strFTS          = "FastTrack Shared";
      char* strMyNetworkPlaces = "My Network Places";
     
      if((strcmp((char*)foundItem.pszText,(char*)strMyComputer) == 0) ||
        (strcmp((char*)foundItem.pszText,(char*)strLocalDisk) == 0) ||
        (strcmp((char*)foundItem.pszText,(char*)strFTS) == 0) )
      {

        
        pathFound.push_back(foundItem);
        //we were successful
        if(pathFound.size() == 3)
        {
          result = true;
         // memcpy(searchItem,&foundItem,sizeof(TVITEM)+1024);
          memcpy(&searchItem->mask,&foundItem.mask,sizeof( UINT ));
          memcpy(&searchItem->hItem,&foundItem.hItem,sizeof(HTREEITEM)); 
          memcpy(searchItem->pszText,(char*)(foundItem.pszText),1024);
          return result;
        }
        if(memcmp((char*)foundItem.pszText,(char*)strLocalDisk,strlen(strLocalDisk)) == 0)
        {
          TreeView_Expand((HWND)m_folderBorwseHWND,tiCurrItem, TVM_EXPAND);
        }
          
      }
      else if(memcmp((char*)&foundItem.pszText,(char*)strMyNetworkPlaces,strlen(strMyNetworkPlaces)) == 0)
      {
       // MDConsoleOutput::MDPrintln(2,"Exciting Too Farr!!!");
        break;
      }   
    }
    
    //then we try to get the next one
    //if it has a child TreeView_GetChild((HWND)m_folderBorwseHWND,
    //TreeView_Expand((HWND)m_folderBorwseHWND,tiCurrItem, TVM_EXPAND); 

    if ( ( tiNextItem = TreeView_GetChild((HWND)m_folderBorwseHWND,tiCurrItem) ) )
    { //we get the next one
      tiCurrItem = tiNextItem;
    }
    else
    { /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      //we are on a final item, so we gotta climb up
      //to the parent to get the next one till we get one
      int iSearchChild = 1;
      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

      //if the first item is the last one
      if ( tiCurrItem == tiStart )
      {
        iSearchChild = 0;
        iRoot = 0;
      }

      while ( iSearchChild == 1 )
      {
        if ( ( tiNextItem = TreeView_GetParent((HWND)m_folderBorwseHWND,tiCurrItem) ) )
        { /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
          //nextitem is the father whose we catch childs
          HTREEITEM   tiNextChild = TreeView_GetChild((HWND)m_folderBorwseHWND, tiNextItem );
          /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                     
          //we try to find the next child after curritem
          while ( tiNextChild && tiNextChild != tiCurrItem )
          {
            tiNextChild = TreeView_GetNextSibling((HWND)m_folderBorwseHWND, tiNextChild );
          }

          //if NextChild is not null, it should be
          //on curritem, so we go to the next one
          if ( tiNextChild )
          {
            if ( ( tiNextChild = TreeView_GetNextSibling((HWND)m_folderBorwseHWND,tiNextChild) ) )
            { //if nextchild is ok, it s the next item
              tiCurrItem = tiNextChild;
              iSearchChild = 0;
            }
            else
            { //no more child, we go up and continue
              tiCurrItem = tiNextItem;
            }
          }
          else
          { //no more child, we go up and continue
            tiCurrItem = tiNextItem;
          }
        }//end if
        else
        { //error, no parent found
          iSearchChild = 2;
          iRet = -1;
        }//end else

        //if we r up to the start, we stop
        if ( tiCurrItem == tiStart )
        {
            iRoot = 0;
        }
      }//end while
    }//end else
  }while ( iRoot && iRet == 1);


  return result;
}  
/*-----------------------------------------------------------------------------
Function Name:SearchForItemNamed
Arugments: const TVITEM* treeEntry
Returns: bool
Access:  public
Purpose: To select an item in a tree structure
Created: 3/28/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDSysFileControlWrapper::SelectItem(const TVITEM* treeEntry)
{
  bool result = false;
  TreeView_SelectItem((HWND)m_folderBorwseHWND,treeEntry->hItem);
  return result;
}  

/*-----------------------------------------------------------------------------
Function Name:GetTVITEM
Arugments: const HTREEITEM *itemToGet, TVITEM *returnItem
Returns: bool
Access:  public
Purpose: This code wraps up the nasty stuff that we need to do while sending messages
into someone else's process.
Created: 3/29/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDSysFileControlWrapper::GetTVITEM(const HTREEITEM *itemToGet, TVITEM *returnItem)
{
  bool result = false;
	// Read the column titles
	char buf[sizeof(TVITEM)+1024];
  char* ptr = NULL;

  //Get memory from withen the m_processHandle
  ptr = (char *)VirtualAllocEx(m_processHandle,NULL,sizeof(buf),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

  memset(buf,0,sizeof(buf));
  TVITEM *item  = (TVITEM*)buf;
	item->mask    = TVIF_TEXT; 
	item->cchTextMax=1024;
	item->pszText = ptr+sizeof(TVITEM);
  item->hItem   = *itemToGet;
  //write our object into WINMX's process memory
  int written = m_pMemoryEditor->WriteProcMemory((void*)ptr,(unsigned char*)buf,sizeof(buf));
  //bool res = TreeView_GetItem((HWND)tempStruct.m_processHandle,item);
  bool res = SendMessage((HWND)m_folderBorwseHWND,TVM_GETITEM,0,(LPARAM)ptr);
  //if our message was succesful read back our data
  if(res)
  {
    int numberread = m_pMemoryEditor->ReadProcMemory((void*)ptr,(unsigned char*)buf,sizeof(buf));
    //memcpy(returnItem&buf[sizeof(TVITEM)],sizeof(TVITEM));
    //memcpy(returnItem,item,sizeof(TVITEM)+1024);
    item = (TVITEM*)buf;
    item->pszText = buf + sizeof(TVITEM);
    memcpy(&returnItem->mask,&item->mask,sizeof( UINT ));
    memcpy(&returnItem->hItem,&item->hItem,sizeof(HTREEITEM)); 
    memcpy(returnItem->pszText,(char*)(item->pszText),1024);
    result = true;
  }
  //free memory
  VirtualFreeEx(m_processHandle,ptr,sizeof(buf),MEM_RELEASE);
  return result;
}  

 