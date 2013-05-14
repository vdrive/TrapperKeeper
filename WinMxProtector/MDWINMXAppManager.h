/*-----------------------------------------------------------------------------
Class Name: MDWINMXAppManager
Purpose:  This is the Wrapper for all of our functionality for manage a number
of winmx instances
Created:  3/15/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef MDWINMXAPPMANAGER_H
#define MDWINMXAPPMANAGER_H
/*-------------------------------INCLUDE---------------------------------------*/
/*--------------------------CLASS DECLERATION----------------------------------*/
class MDWINMXAppManager
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  MDWINMXAppManager();
  ~MDWINMXAppManager();
  bool SetNumberOfInstancesToManage(int numOfApps);
  bool GO();
  bool IsBusy();
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
};
#endif