/*-----------------------------------------------------------------------------
Class Name: MDSysListControlWrapper  
Purpose:  
Created:  
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDSYSLISTCONTROLWRAPPER_H
#define MDSYSLISTCONTROLWRAPPER_H
/*-------------------------------INCLUDE---------------------------------------*/
//#include "windows.h"
//#include <vector>
#include "stdafx.h"
/*-------------------------------NAMESPACe-------------------------------------*/
//using namespace std;
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDSysListControlWrapper
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDSysListControlWrapper(HANDLE listControl,HANDLE header,DWORD processID);
  ~MDSysListControlWrapper();
  int GetListCount();
  bool GetListData(vector<string> data);
  bool GetListRect();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
private:
  int GetHeaderInfo();
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
private:
  HANDLE m_listControlHWND;
  HANDLE m_headerHWND;
  vector<string> m_headerInfo;
  DWORD  m_processID;
};
#endif