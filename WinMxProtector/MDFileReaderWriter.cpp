/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------INCLUDE---------------------------------------*/
#include "MDFileReaderWriter.h"
/*-----------------------------------------------------------------------------
Function Name:
Arugments: 
Returns: 
Access:
Purpose: 
Created:
Modified:
-------------------------------------------------------------------------------*/
MDFileReaderWriter::MDFileReaderWriter()
{
  m_fileHandle =  NULL;
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
MDFileReaderWriter::~MDFileReaderWriter()
{
  //make sure handle was valid
  if(m_fileHandle!=NULL)
  {
    CloseHandle(m_fileHandle);
    m_fileHandle=NULL;
  }
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
bool MDFileReaderWriter::OpenFileAccessor(char* fileName, DWORD access)
{
  bool result = false;
 
  if(m_fileHandle != NULL)
  {
    result = OpenFile(fileName,access);
  }
 
    
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
bool MDFileReaderWriter::CloseFileAccessor()
{
  bool result = false;
  if(m_fileHandle != NULL)
  {
    result = CloseHandle(m_fileHandle);
    m_fileHandle = NULL;
  }
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
bool MDFileReaderWriter::IsFileBusy(char* fileName)
{ 
  bool result = false;
  result = OpenFile(fileName,GENERIC_READ);
  if(result == true)
  {
    //file is not busy
    result = false;
  }
  else
  {
    //file won't let us open it
    result = true;
  }

  //close this data
  CloseFileAccessor();
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
bool MDFileReaderWriter::Write(unsigned char* byteArray,DWORD bytesToWrite)
{
  bool result = false;
  DWORD bytesWritten;
  result = WriteFile(
                        m_fileHandle,         // handle to file
                        byteArray,            // data buffer
                        bytesToWrite,         // number of bytes to write
                        &bytesWritten,        // number of bytes written
                        NULL);                // overlapped buffer

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
bool MDFileReaderWriter::OpenFile(char* fileName,DWORD access)
{
  DWORD fileCreation=0;
  bool result = false;

   switch(access)
  {
  case GENERIC_READ:
    fileCreation = OPEN_EXISTING;
    break;
  case GENERIC_WRITE:
    fileCreation = CREATE_ALWAYS;
    break;
  }

  m_fileHandle = CreateFile(fileName,           // create MYFILE.TXT 
                              access,                // open for writing 
                              0,                            // do not share 
                              NULL,                         // no security 
                              fileCreation,                // overwrite existing 
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);                        // no attr. template 

 
  if (m_fileHandle == INVALID_HANDLE_VALUE) 
  { 
    result = false;// MDConsoleOutput::Println(("Could not open file.");  // process error 
  } 
  else if(m_fileHandle == NULL)
  {
    result = false;
  }
  else
    result = true;

  return result;
}










