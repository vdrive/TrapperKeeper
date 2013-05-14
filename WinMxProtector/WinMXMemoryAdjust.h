/*-----------------------------------------------------------------------------
Class Name: WinMXMemoryAdjust
Purpose:  The purpose of this class is to just wrap up the functionality of 
hacking a winmx instance to allow more than one instance to run at a time
Created:  3/1/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#ifndef _WinMXMemoryAdjust_H
#define _WinMXMemoryAdjust_H
/*-------------------------------INCLUDE---------------------------------------*/
#include "MemProjDefs.h"
#include "MDMemoryEditor.h"
/*--------------------------CLASS DECLERATION----------------------------------*/
class WinMXMemoryAdjust
{
/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
public:
  WinMXMemoryAdjust();
  virtual ~WinMXMemoryAdjust(void);
  virtual bool FixWinMXFlag(DWORD procID, DWORD baseAddr);
/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
/*----------------------------PUBLIC MEMBERS-----------------------------------*/
/*----------------------------PRIVATE MEMBERS----------------------------------*/
};
#endif
