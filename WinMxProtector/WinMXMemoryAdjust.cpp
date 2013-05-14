/*-----------------------------------------------------------------------------
Class Name:  WinMXMemoryAdjust
Purpose:  To allow us to launch multiple instanciations of WinMX for our evil
Created:  2-27-04 p. faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "WinMXMemoryAdjust.h"
//#include "MDConsoleOutput.h"
/*-----------------------------------------------------------------------------
Function Name: WinMXMemoryAdjust()
Arugments: void
Returns:   void
Purpose:   Constructor
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WinMXMemoryAdjust::WinMXMemoryAdjust(void)
{
}
/*-----------------------------------------------------------------------------
Function Name: ~WinMXMemoryAdjust()
Arugments: void
Returns:   void
Purpose:   Destructor
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
WinMXMemoryAdjust::~WinMXMemoryAdjust(void)
{
}
/*-----------------------------------------------------------------------------
Function Name: FixWinMXFlag(const ProcessNameIDStruct *procID)
Arugments: const ProcessNameIDStruct *procID - this is the WinMX proc we're hackin
Returns:   bool - true if we success
Purpose:   To fix the passed in instance of WinMX
Created:   2/27/04 p.faby 
Modified:
-------------------------------------------------------------------------------*/
bool WinMXMemoryAdjust::FixWinMXFlag(DWORD procID, DWORD baseAddr)
{
  bool result = false;
  if(procID != NULL)
  {
    //MDConsoleOutput::MDPrintln("Fixing ID:",procID);
    try
    {
      //our operator for memory reading and writing
      MDMemoryEditor *memoryHacker = new MDMemoryEditor(procID);
    
      //array we're going to read the bytes into
      unsigned char *readBytes  = new unsigned char[MEMORYSCANSIZE+1];
    
      //calculate the starting address
      DWORD start= STARTADDRESS; //+ procID->m_baseAddr;
      DWORD bytesRead=0;
		  DWORD memOffset=0;
    
      //when we find the signiture this flag get's set to true
		  bool found = false;

      //signiture of the bytes we're looking for 
      unsigned char signiture[] = {0x0f,0x84,0x9e,0x00,0x00,0x00,0x8b,0x50};
      unsigned char signitureHack[] = {0xE9,0x9F,0x00,0x00,0x00,0x50};
 
      unsigned char *ptr=0;
      //MDConsoleOutput::Println("Base Addr:", (DWORD)STARTADDRESS+baseAddr);
      int count=0;
      while(!found)
      {
        //let's attemp our read
        bytesRead = memoryHacker->ReadProcMemory((void*)(STARTADDRESS+baseAddr),readBytes,MEMORYSCANSIZE);
        count++;
        //MDConsoleOutput::Println((".");
        if(bytesRead==MEMORYSCANSIZE)
        {
		      unsigned char *ptr=(unsigned char *)readBytes;  
          for(memOffset=0;memOffset<(bytesRead-sizeof(signiture));memOffset++)
		      { 
            //let's find the culpurate
            if(memcmp(signiture,ptr,sizeof(signiture))==0)
            {
              found = true;
              break;
            }
            ptr++;
           }//endoffor
        }//end of if
        else
        {
         // MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"WinMXMemoryAdjust::Read Failure");
        } 

        if(found)
        {  
         result = memoryHacker->WriteProcMemory((void*)( STARTADDRESS+baseAddr+memOffset),signitureHack,sizeof(signitureHack));
        } 
        else
        {
          Sleep(30);
        }
      }//end of while
      delete memoryHacker;
      delete[] readBytes;
    }//end try
    catch(char * errorString)
    {
    //  MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"We caught an exceptions",errorString);
    }
  }//end if
  return result;
}



