/*-------------------------------INCLUDE---------------------------------------*/
#include "MDVirtualMemoryScanner.h"
#include "ProcessUtils.h"
#include "ProcessNameID.h"
//#include "MDConsoleOutput.h"
//#include <vector>
//#include <string>
using namespace std;
vector<char*> data;
/*----------------------------PREPROCESSOR-------------------------------------*/
#define TOTALVMRESERVE	0x00100000
#define PAGESIZE			  0x1000

struct _Song_User_Data
{
  char SongName[1000];
  char UserIP[4];
} Song_User_Data, PSong_User_Data;

/*-----------------------------------------------------------------------------
Function Name: MDVirtualMemoryScanner(DWORD processID)
Arugments:DWORD - processID
Returns:  void
Access:   public
Purpose:  Constructor
Created:  3/23/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDVirtualMemoryScanner::MDVirtualMemoryScanner(DWORD processID)
{
  m_processID = processID;
  m_memoryHacker = new MDMemoryEditor(m_processID);
}
/*-----------------------------------------------------------------------------
Function Name: ~MDVirtualMemoryScanner()
Arugments:void
Returns:  void
Access:   public
Purpose:  Destructor
Created:  3/23/04 p.faby
Modified: 
-------------------------------------------------------------------------------*/
MDVirtualMemoryScanner::~MDVirtualMemoryScanner()
{
}
/*-----------------------------------------------------------------------------
Function Name: ScanVirtualMemorySpace()
Arugments:void
Returns:  void
Access:   public
Purpose:  To cruise through the virtual memory space of a process, handy
Created:  3/23/04 p.faby
Modified: 
-------------------------------------------------------------------------------*/
bool MDVirtualMemoryScanner::ScanVirtualMemorySpace()
{
  bool result = false;
 
  SYSTEM_INFO	si;
	MEMORY_BASIC_INFORMATION memInfo;
  ProcessNameIDStruct pnID;
  //Get the base address of this process as a helper
  ProcessUtils::GetProcInfo(m_processID,&pnID);
  DWORD offset = pnID.m_baseAddr; 
  int memsize = ProcessUtils::GetProcessMemUsage(m_processID);
  //MDConsoleOutput::MDPrintln(2,"BaseAddr:",offset);
  MDFileReaderWriter* rw = new MDFileReaderWriter();
  rw->OpenFile("ProcessDump.bin",GENERIC_WRITE);
  MDFileReaderWriter* dump = new MDFileReaderWriter();
  dump->OpenFile("Dump.txt",GENERIC_WRITE);
 // /* Get maximum address range from system info */
  GetSystemInfo(&si); 

  printf("Address      Type        State           Protection      Base        Range\n");
  //Loop through to get the information about this process
  DWORD count=0;

  while (offset < (DWORD)si.lpMaximumApplicationAddress)
	{		   
    if(m_memoryHacker->MemoryInfoQuery((void*)offset,&memInfo))
    {    
      char type[100];
      char state[100];
      char protec[100];
    
      m_memoryHacker->GetAllocString(memInfo.AllocationProtect,protec);
      m_memoryHacker->GetMemoryTypeString(memInfo.Type,type);
      m_memoryHacker->GetMemoryStateString(memInfo.State,state);
printf("0x%x      %s    %s   %s   0x%x   0x%x\n",
      offset,type,state,protec,memInfo.AllocationBase,memInfo.RegionSize);          

      unsigned char* dataArray=NULL;
 
      if((memInfo.State == MEM_COMMIT) &&
        (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY))
 
      {
        //create an array the size of the region
        dataArray = new unsigned char[memInfo.RegionSize];
        //Read the memory
        bool result = m_memoryHacker->ReadProcMemory((void*)offset,dataArray,memInfo.RegionSize);
        //write out to a file for inspection

        //.//printf("Page %d",count++);
        findData((char*)dataArray,memInfo,rw);
        dump->Write(dataArray,memInfo.RegionSize);
        // rw->Write(dataArray,memInfo.RegionSize);
        delete[] dataArray;
      
      } 
      //offset += (DWORD)((DWORD)pnID.m_baseAddr + (DWORD)memInfo.RegionSize); 

      offset   =  (DWORD)((DWORD)memInfo.BaseAddress + memInfo.RegionSize); 
    }
  }
  for(int i=0;i<(int)data.size();i++)
  {
    rw->Write((unsigned char*)data.at(i),strlen((char*)data.at(i)));
    rw->Write((unsigned char*)"\n",1);
  }
  dump->CloseFileAccessor();
  rw->CloseFileAccessor();
  return result;
}
/*-----------------------------------------------------------------------------
Function Name: ScanVirtualMemorySpace()
Arugments:void
Returns:  void
Access:   public
Purpose:  To cruise through the virtual memory space of a process, handy
Created:  3/23/04 p.faby
Modified: 
-------------------------------------------------------------------------------*/
bool MDVirtualMemoryScanner::ClearVirtualMemorySpace()
{
  bool result = false;
 
  SYSTEM_INFO	si;
	MEMORY_BASIC_INFORMATION memInfo;
  ProcessNameIDStruct pnID;
  //Get the base address of this process as a helper
  ProcessUtils::GetProcInfo(m_processID,&pnID);
  int offset = pnID.m_baseAddr; 
  int memsize = ProcessUtils::GetProcessMemUsage(m_processID);
  //MDConsoleOutput::MDPrintln(2,"BaseAddr:",offset);
  MDFileReaderWriter* rw = new MDFileReaderWriter();
  rw->OpenFile("ProcessDump.bin",GENERIC_WRITE);
 // /* Get maximum address range from system info */
  GetSystemInfo(&si); 

  printf("Address      Type        State           Protection      Base        Range\n");
  //Loop through to get the information about this process
  DWORD count=0;

  while ((offset < (DWORD)si.lpMaximumApplicationAddress)
          &&  (offset < offset + memsize))
	{		   
    if(count>0x400) return -1;
  
    if(m_memoryHacker->MemoryInfoQuery((void*)offset,&memInfo))
    {    
      char type[100];
      char state[100];
      char protec[100];
    
      m_memoryHacker->GetAllocString(memInfo.AllocationProtect,protec);
      m_memoryHacker->GetMemoryTypeString(memInfo.Type,type);
      m_memoryHacker->GetMemoryStateString(memInfo.State,state);
printf("0x%x      %s    %s   %s   0x%x   0x%x\n",
      offset,type,state,protec,memInfo.AllocationBase,memInfo.RegionSize);          

      unsigned char* dataArray=NULL;
 
       if((memInfo.State == MEM_COMMIT) &&
        (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY))
      {
        //create an array the size of the region
        dataArray = new unsigned char[memInfo.RegionSize];
        //Read the memory
        bool result = m_memoryHacker->ReadProcMemory((void*)offset,dataArray,memInfo.RegionSize);
        //write out to a file for inspection

        //.//printf("Page %d",count++);
        ClearData((char*)dataArray,memInfo,rw);
        // rw->Write(dataArray,memInfo.RegionSize);
        delete[] dataArray;
      
      } 
      offset = (DWORD)((DWORD)memInfo.BaseAddress +(DWORD)memInfo.RegionSize); 
    }
  }
  rw->CloseFileAccessor();
  return result;
}
bool MDVirtualMemoryScanner::ClearData(char* dataArray,MEMORY_BASIC_INFORMATION memInfo,MDFileReaderWriter* rw)
{
  bool result = false;
  int stringLen = 0;
  int lastFound = 0;
  for(int i = 0;i<(int)memInfo.RegionSize;i++)
  {
    //if not a blank spot continue
    if(dataArray[i] != 0x00)
    {
      //try to find ".mp3"
      if(memcmp(".mp3",dataArray+i,4)==0)
      {
        //if we found ".mp3", check to see if the preceeding char was a blank
        //if((dataArray[i-1]) != 0)
        //{
          //if it was valid string continue
          for(int back=i;back>(i-100);back--)
          {
            //if we found the begining of the string, this is our Dir\song
            if( (memcmp((unsigned char*)&dataArray[back],":\\",2) == 0)// ||
                //(memcmp((unsigned char*)&dataArray[back],"D:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"E:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"F:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"H:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"D:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"E:\\",3) == 0) ||
                //(memcmp((unsigned char*)&dataArray[back],"F:\\",3) == 0) ||
                
            )
            {
              //the begining(the end of ".mp3" i + 4 // back,the beginning of the string...-1 since we overstep
              stringLen = i - back  + 4; 
              //count += stringLen;
              char* dir = new char[stringLen];
              memset(dir,0,stringLen);
              

bool result = m_memoryHacker->WriteProcMemory((void*)(back +  (DWORD)memInfo.BaseAddress),(unsigned char*)dir,stringLen);

              lastFound = i;
              break; 
            }//end if
          }//end for
          //    }
            }//end cmp
          }//if
         }//for
  return result;
}

bool MDVirtualMemoryScanner::findData(char* dataArray,MEMORY_BASIC_INFORMATION memInfo,MDFileReaderWriter* rw)
{
  bool result = false;
  int stringLen = 0;
  int lastFound = 0;
  bool first=true;

  for(int i = 0;i<(int)memInfo.RegionSize;i++)
  {
    //if not a blank spot continue
    if(dataArray[i] != 0x00)
    {
      //try to find ".mp3"
      if(strcmp(".mp3",dataArray+i)==0)//,4)==0)
      {
        //if we found ".mp3", check to see if the preceeding char was a blank
         if((dataArray[i-1]) != 0)
         {
          
          if(first)
          {
            lastFound = i - 50;
            first = false;
          }
          //if it was valid string continue
          for(int back=i;back>(lastFound);back--)
          {
            if(dataArray[back]== 0) break;
            //if we found the begining of the string, this is our Dir\song
            if( (memcmp((unsigned char*)&dataArray[back],"C:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"D:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"E:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"F:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"G:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"H:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"I:\\",3) == 0) ||
                (memcmp((unsigned char*)&dataArray[back],"J:\\",3) == 0)  
                
            )
            {
              //the begining(the end of ".mp3" i + 4 // back,the beginning of the string...-1 since we overstep
              stringLen = i - back  + 4; 
              //count += stringLen;
              char* dir = new char[stringLen ];
              memset(dir,0,stringLen);

              memcpy(dir,&dataArray[back],stringLen);
              //dir[stringLen] = (char)'\n';
bool found = false;
for(int blah=0;blah<(int)data.size();blah++)
{
  char* foo = data.at(blah);

  if(strcmp(dir,foo)==0)
  {
    found = true;
  }
}
if(!found)
{
  int len = (int)strlen(dir);
  for(int r=0;r<len;r++)
  {
    if(!((dir[r] > 0x19) && (dir[r] < 0x7f)))
    {
      dir[r] = 0x20;
    }
     
  }
 
  data.push_back(dir);
}
             // rw->Write((unsigned char*)dir,stringLen);
             // rw->Write((unsigned char*)"\n",1);
//IP
             // rw->Write((unsigned char*)"IP:",3);
              char Oct1[10] = {0x00};
              char Oct2[10] = {0x00};
              char Oct3[10] = {0x00};
              char Oct4[10] = {0x00};              
   
              itoa((unsigned char)dataArray[i+4+152],Oct1,10);
              itoa((unsigned char)dataArray[i+4+153],Oct2,10);              
              itoa((unsigned char)dataArray[i+4+154],Oct3,10);
              itoa((unsigned char)dataArray[i+4+155],Oct4,10);


             // rw->Write((unsigned char*)Oct1,strlen(Oct1));
             // rw->Write((unsigned char*)".",1);
             // rw->Write((unsigned char*)Oct2,strlen(Oct2));
             // rw->Write((unsigned char*)".",1);
             // rw->Write((unsigned char*)Oct3,strlen(Oct3));
             // rw->Write((unsigned char*)".",1);
             // rw->Write((unsigned char*)Oct4,strlen(Oct4));      
              //blank out that name 
             // rw->Write((unsigned char*)"\n",1);
              //memset(dir,0x20,stringLen);
              //memcpy(dir,"PDogg Rules this string",23);
              //m_memoryHacker->WriteProcMemory((void*)(offset+back),(unsigned char*)dir,stringLen);
              //printf( (char*)dir);
              //printf("\n");
              lastFound = back + stringLen;
              break; 
            }//end if
          }//end for
         }
            }//end cmp
          }//if
         }//for
  return result;
}
















