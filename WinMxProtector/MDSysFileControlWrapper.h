/*-----------------------------------------------------------------------------
Class Name: MDSysFileControlWrapper
Purpose:  To give the user a simple interface to search and select items form a tree
Created:  3/28/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDSYSFILECONTROLWRAPPER_H
#define MDSYSFILECONTROLWRAPPER_H
/*-------------------------------INCLUDE---------------------------------------*/
//#include "windows.h"
//#include "commctrl.h"
#include "MDMemoryEditor.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDSysFileControlWrapper
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDSysFileControlWrapper(int processID,HWND browseHWND);
  ~MDSysFileControlWrapper();
  bool SearchForItemNamed(const char* dirTofind,TVITEM* searchItem);
  bool SelectItem(const TVITEM* treeEntry);  
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
private:
  bool GetTVITEM(const HTREEITEM *itemToGet, TVITEM *returnItem);
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
public:
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  MDMemoryEditor* m_pMemoryEditor;
  int           m_processID;
  HWND          m_folderBorwseHWND;
  HANDLE        m_processHandle;
};//end of class
#endif