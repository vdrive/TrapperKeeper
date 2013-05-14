/*-----------------------------------------------------------------------------
Class Name:  MDMemoryEditor
Purpose:  to read into a processes memory
Created:  2-27-04 p. faby
Modified:
-------------------------------------------------------------------------------*/
/*----------------------------PREPROCESSOR-------------------------------------*/
#define DEBUG
/*-------------------------------INCLUDE---------------------------------------*/
#include "MDMemoryEditor.h"
#include "ProcessUtils.h"
//#include "MDConsoleOutput.h"

/*---------------------------PUBLIC FUNCTIONS----------------------------------*/
//void debug(MEMORY_BASIC_INFORMATION* pMemInfo);
/*-----------------------------------------------------------------------------
Function Name: MDMemoryEditor(const ProcessNameIDStruct *procInfo)
Arugments: ProcessNameIDStruct - process information stuff
Returns: 
Purpose:   Constructor
Created:   2-27-04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDMemoryEditor::MDMemoryEditor(DWORD processID)
{
  m_handle  =   ProcessUtils::GetProcessHandle(processID);
  m_processID = processID;
  m_procInstance = (HINSTANCE) m_handle;
}
/*-----------------------------------------------------------------------------
Function Name: MDMemoryEditor(const ProcessNameIDStruct *procInfo)
Arugments: ProcessNameIDStruct - process information stuff
Returns: 
Purpose:   Constructor
Created:   2-27-04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDMemoryEditor::MDMemoryEditor(HANDLE procHandle,HINSTANCE instance)
{
  m_handle  =   (HANDLE)procHandle;
  m_processID = 0;
  m_procInstance = (HINSTANCE) instance;
}
/*-----------------------------------------------------------------------------
Function Name:~MDMemoryEditor()
Arugments: void
Returns:   void
Purpose:   Destructor
Created:   2/27/04
Modified:
-------------------------------------------------------------------------------*/
MDMemoryEditor::~MDMemoryEditor()
{
  CloseHandle(m_handle);
}

/*-----------------------------------------------------------------------------
Function Name: ReadProcMemory(byte *dataBuf,DWORD offset, DWORD sizeToRead)
Arugments: byte* - sized array for data to be placed, DWORD -where to start
reading,   DWORD - how much should we read boss
Returns:   DWORD - bytes read 
Purpose:   to read into the processmemory of someone else
Created:   2/27/04 p.faby
Modified:iostream
-------------------------------------------------------------------------------*/
DWORD MDMemoryEditor::ReadProcMemory(void* offset,unsigned char *dataBuf,
                                       DWORD sizeToRead)
{
	int error=-1;
	DWORD bytesRead=0;

  if(m_procInstance != NULL)
  {
	  //Let's attempt reading into memory
    bytesRead=ProcessUtils::ReadProcMemory(m_procInstance,(void*)offset,dataBuf,sizeToRead);

	  //if we failed, let's get some error messages
    if(bytesRead==0)
    { 
	    error=GetLastError();
	    //MDConsoleOutput::MDPrintln("ReadProcessMemory Failed!  Error Code = ",error);
    }
    else
    {
      //MDConsoleOutput::Println(("Bytes Read:%d\n",bytesRead);
    }

  }
	return bytesRead;
}
/*-----------------------------------------------------------------------------
Function Name: WriteProcMemory(unsigned char *dataBuf,DWORD offset, DWORD sizeToWrite)
Arugments: unsigned char* - array of data to write, DWORD -where to start writing,
DWORD -   number of bytes to write
Returns:   bool - true if successful
Purpose:   to write into the process memory of someone else
Created:   2/27/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDMemoryEditor::WriteProcMemory(void* offset, unsigned char *writeBuf , DWORD sizeToWrite)
{
  bool result = false;
  DWORD bytesWritten = 0;

  if(m_procInstance != NULL)
  {
    //let's try our write
	  result = ProcessUtils::WriteProcMemory(m_procInstance,(void*)(offset),writeBuf,sizeToWrite);

    //call flush, otherwise CPU may run cached instructions
    ProcessUtils::FlushMemory(m_procInstance,(void*)(offset),sizeToWrite);
 
    DWORD get=0;
    if(!result)
    {
      get = GetLastError();
      //MDConsoleOutput::MDPrintln("Error In MemoryEditor:",get);
      int i =0;
    }

  }
  else
  {
    //MDConsoleOutput::MDPrintln("Write failed...handle is messed");
  }
  return result;
}
/*-----------------------------------------------------------------------------
Function Name:MemoryInfoQuery(void* offset, MEMORY_BASIC_INFORMATION* pMemInfo)
Arugments: offset into the process memory, a pointer to a struct which the
memory information will be sent to.
Returns:  true if we succesfully filled the struct
Access:   public
Purpose:  to give us basic infomation about a specified area of memory
Created:  3/24/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDMemoryEditor::MemoryInfoQuery(void* offset, MEMORY_BASIC_INFORMATION* pMemInfo)
{
  bool result = false;
  //this is the size to read, basically the size of the information we want
  int sizeToRead = sizeof(MEMORY_BASIC_INFORMATION);
  //clear memory of struct
  memset(pMemInfo,0,sizeToRead);
  //the api call which does all of the real work
  DWORD areaRead = (DWORD)VirtualQueryEx( m_handle, offset, pMemInfo, sizeToRead);
  //if we didn't fill our struct, fail MEM_COMMIT
  if(areaRead == sizeToRead)
  {
    result = true;
  }

#ifdef DEBUG
  //debug(pMemInfo);
#endif

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
void MDMemoryEditor::GetAllocString(DWORD type, char* memAccessType)
{
  switch(type)
  {
  case 0:
    strcpy(memAccessType,"N/A               ");
  break;
  case PAGE_NOACCESS:
    strcpy(memAccessType,"No Access         ");
  break;//          0x01     
  case PAGE_READONLY:
    strcpy(memAccessType,"Read Only         ");
    break;//         0x02     
  case PAGE_READWRITE:
    strcpy(memAccessType,"ReadWrite         ");
    break;//         0x04     
  case PAGE_WRITECOPY:
    strcpy(memAccessType,"Write Copy        ");
    break;//         0x08     
  case PAGE_EXECUTE:
    strcpy(memAccessType,"Execute           ");
    break;//           0x10     
  case PAGE_EXECUTE_READ:
    strcpy(memAccessType,"Execute Read      ");
    break;//     0x20     
  case PAGE_EXECUTE_READWRITE:
    strcpy(memAccessType,"Execute Read/Write");
    break;// 0x40     
  case PAGE_EXECUTE_WRITECOPY:
    strcpy(memAccessType,"Write Copy        ");
    break;// 0x80     
  case PAGE_GUARD:
    strcpy(memAccessType,"Gaurd             ");
    break;//            0x100     
  case PAGE_NOCACHE:
    strcpy(memAccessType,"No Cache          ");
    break;//          0x200     
  case PAGE_WRITECOMBINE:
    strcpy(memAccessType,"Write Combine     ");
    break;//     0x400    
  default: 
    strcpy(memAccessType,"-                 ");
    break;
  }
 // MDConsoleOutput::MDPrintln(2,memAccessType);
}
/*-----------------------------------------------------------------------------
Function Name:GetMemoryStateString(DWORD state, char* memState)
Arugments:state - from the MEMORY_BASIC_INFO, pointer to valid data
Returns:  void
Access:   public
Purpose:  to give us the string of the memory state via a number
Created:  3/24/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
void MDMemoryEditor::GetMemoryStateString(DWORD state, char* memState)
{
  if(memState != NULL)
  {
    switch(state)
    {
    case MEM_COMMIT:
      strcpy(memState,"Commit  "); //           0x1000 
      break;    
    case MEM_RESERVE:
      strcpy(memState,"Reserve "); //          0x2000 
      break;     
    case MEM_DECOMMIT:
      strcpy(memState,"Decommit"); //        0x4000 
      break;     
    case MEM_RELEASE:
      strcpy(memState,"Release "); //          0x8000 
      break;     
    case MEM_FREE:
      strcpy(memState,"Free    "); //            0x10000 
      break; 
    default: 
      strcpy(memState,"-       ");
      break;
    }
    //MDConsoleOutput::MDPrintln(2,memState);
  }
} 
/*-----------------------------------------------------------------------------
Function Name:GetMemoryTypeString(DWORD memType, char* memType)
Arugments:memtype - from the MEMORY_BASIC_INFO, pointer to a valid buffer
Returns:  void
Access:   public
Purpose:  to give us the string of the memory type via a number
Created:  3/24/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
void MDMemoryEditor::GetMemoryTypeString(DWORD memType, char* memString)
{
  if(memString != NULL)
  {
	  switch(memType)
	  {
	  case MEM_IMAGE:
		  strcpy(memString,"Image  ");
		  break;
	  case MEM_MAPPED:
		  strcpy(memString,"Mapped ");
		  break;
	  case MEM_PRIVATE:
		  strcpy(memString,"Private");
		  break;
	  default:
		  strcpy(memString,"-      ");
		  break;
	  }
   // MDConsoleOutput::MDPrintln(2,memString);
  }
}

/*-------------------------- PRIVATE FUNCTIONS---------------------------------*/
















