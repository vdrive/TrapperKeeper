/*-------------------------------INCLUDE---------------------------------------*/
#include "MDSysListControlWrapper.h"
//#include "commctrl.h"
#include "ProcessUtils.h"
#include "MDMemoryEditor.h"
//#include "MDConsoleOutput.h"
//#include "windows.h"
#include "MDFileReaderWriter.h"
#include "ProcessNameID.h"
/*----------------------------PREPROCESSOR-------------------------------------*/
/*-------------------------------NAMESPACE-------------------------------------*/
using namespace std;
/*-----------------------------------------------------------------------------
Function Name: MDSysListControlWrapper(HANDLE listControl)
Arugments:HANDLE - handle to the list control we're interested in
Returns:  void
Access:   public
Purpose:  Contructor - to give us a nice wrapper to a hijacked control
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDSysListControlWrapper::MDSysListControlWrapper(HANDLE listControl,
                                                 HANDLE header,DWORD processID)
{
  m_listControlHWND = listControl;
  m_processID       = processID;
  m_headerHWND      = header;
}
/*-----------------------------------------------------------------------------
Function Name: ~MDSysListControlWrapper()
Arugments: 
Returns:  void
Access:   public
Purpose:  Destructor
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDSysListControlWrapper::~MDSysListControlWrapper()
{
}
/*-----------------------------------------------------------------------------
Function Name: GetListCount()
Arugments: 
Returns:  void
Access:   public
Purpose:  To get the count of items inside of the list
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
int MDSysListControlWrapper::GetListCount()
{
  // Get the number of columns in the search return
  int num_columns =  ListView_GetItemCount((HWND)m_listControlHWND);
  return num_columns;             
}
/*-----------------------------------------------------------------------------
Function Name: GetHeaderInfo()
Arugments: 
Returns:  void
Access:   public
Purpose:  To get the count of items inside of the list
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
MDSysListControlWrapper::GetHeaderInfo()
{
  // ListView_GetItem((HWND)m_listControlHWND,&tempItem);
  HANDLE handle = ProcessUtils::GetProcessHandle(m_processID);
  MDMemoryEditor *hacker = new MDMemoryEditor(m_processID);

  int headerCount = Header_GetItemCount((HWND)m_headerHWND);

	// Read the column titles
	char buf[sizeof(HDITEM)+1024];
  char* ptr = NULL;
  ptr = (char *)VirtualAllocEx(handle,NULL,sizeof(buf),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

	for(int i=0;i<headerCount;i++)
	{
		memset(buf,0,sizeof(buf));
		HDITEM *hditem=(HDITEM *)buf;

		hditem->mask=HDI_TEXT;
		hditem->cchTextMax=1024;
		hditem->pszText=ptr+sizeof(HDITEM);

    hacker->WriteProcMemory((void*)ptr,(unsigned char*)buf,sizeof(buf));
		::SendMessage((HWND)m_headerHWND,HDM_GETITEM,(WPARAM)i,(LPARAM)ptr);
    int numberread = hacker->ReadProcMemory((void*)ptr,(unsigned char*)buf,sizeof(buf));

		m_headerInfo.push_back(&buf[sizeof(HDITEM)]);
	} 

  //Free Memory
  VirtualFreeEx(handle,ptr,0,MEM_RELEASE);
  
  for(int i = 0;i<(int)m_headerInfo.size();i++)
  {
    //MDConsoleOutput::MDPrintln(1,(char*)m_headerInfo.at(i).data());
  }
  delete hacker;
  return 1;
}
/*-----------------------------------------------------------------------------
Function Name: GetListData(vector<string> data)
Arugments: 
Returns:  void
Access:   public
Purpose:  To get the data inside of the list
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDSysListControlWrapper::GetListData(vector<string> data)
{
  bool result = false;
  //int count = GetListCount();
   
  //get Header Information
  //GetHeaderInfo();
GetListRect();
  //get the handle for this process
  /*HANDLE handle = ProcessUtils::GetProcessHandle(m_processID);
  
  //Create our memory editor to probe this process
  MDMemoryEditor *hacker = new MDMemoryEditor(m_processID);
 
  // Read the column titles
	char buf[sizeof(LVITEM)+1024];
  char* ptr = NULL;

  //Reserve memory in the WINMX process
  ptr = (char *)VirtualAllocEx(handle,NULL,sizeof(buf),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 count = 0xffffffff;
  for(int index=0;index<count;index++)
  {
    memset(buf,0,sizeof(buf));
    LVITEM *item = (LVITEM*)buf;
		item->mask= LVIF_TEXT ;
 //LVIF_IMAGE | LVIF_INDENT   | LVIF_NORECOMPUTE |LVIF_PARAM |LVIF_STATE |L LVIS_CUT
		item->cchTextMax=1024;
		item->pszText= ptr+sizeof(LVITEM);
    //memset(item->pszText,0,1024);
    item->iItem = 0;
    item->iSubItem =index;
    //write our object into WINMX's process memory
    int written = hacker->WriteProcMemory((unsigned char*)buf,(void*)ptr,sizeof(buf));
 
    //Send the Message into WINMX to ask for it's item
  	LRESULT res = ::SendMessage((HWND)m_listControlHWND,
                                LVM_GETITEMTEXT,
                                (WPARAM)index,
                                (LPARAM)(LVITEM*) ptr); 


    //if our message was succesful read back our data
    if(res)
    {
      int numberread = hacker->ReadProcMemory((unsigned char*)buf,(void*)ptr,sizeof(buf));
      data.push_back(&buf[sizeof(LVITEM)]);
    }
  }

  //Free Memory inside of the winmx process
  VirtualFreeEx(handle,ptr,0,MEM_RELEASE);

  if(data.size() > 0)
  {
    result = true;
  }
  for(int i = 0;i<data.size();i++)
  {
    MDConsoleOutput::MDPrintln(1,(char*)data.at(i).data());
  }
  GetListRect();*/
  return result;
} 

/*-----------------------------------------------------------------------------
Function Name: GetListRect(vector<string> data)
Arugments: 
Returns:  void
Access:   public
Purpose:  To get the data inside of the list
Created:  3/18/04 p.faby
Modified:
-------------------------------------------------------------------------------*/
bool MDSysListControlWrapper::GetListRect( )
{
 int count=(int)SendMessage((HWND)m_listControlHWND, LVM_GETITEMCOUNT, 0, 0);
// int i;
 
 ProcessNameIDStruct info;
 ProcessUtils::GetProcInfo(m_processID,&info);

 NMLVDISPINFO displayInfo, *pdisplayInfo;
// LVITEM lvItem, *plvItem;
 const int MAXTEXTSIZE = 1024;
// LPTSTR item[MAXTEXTSIZE];
 char subitem[MAXTEXTSIZE];
 LPTSTR pTextItem;
// unsigned long pid;
 HANDLE process;

 //get the thread processID
 GetWindowThreadProcessId((HWND)m_listControlHWND, &m_processID);
 
 //get the process
 process=OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|
                     PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, m_processID);

  //create memory inside of the winmx process
  //create memory for the NMDisplayInfo..create room for itself and the text hopefully returned
  pdisplayInfo=(NMLVDISPINFO*)VirtualAllocEx(process, NULL, sizeof(NMLVDISPINFO) + MAXTEXTSIZE,
                                            MEM_COMMIT, PAGE_READWRITE);
 
  
  pTextItem = (char*)VirtualAllocEx(process, NULL, MAXTEXTSIZE,   
                                  MEM_COMMIT, PAGE_READWRITE);
//fill in the Display.hdr info struct*************************************************
  //this is who's sending the message...i assume it's the listview control
  displayInfo.hdr.hwndFrom = (HWND)m_listControlHWND;
  //this is the ID of who's sending the message...again i assume it's the listview control
  displayInfo.hdr.idFrom   = GetWindowLong((HWND)m_listControlHWND,GWL_ID);
  //this is the code for what we want...I'm not sure if this should be WM_NOTIFY
  displayInfo.hdr.code     =  LVN_GETDISPINFO;
//*********************************************************************************

//fill in the LVItem data**********************************************************
  //set the size of our receiving buffer
  displayInfo.item.cchTextMax = MAXTEXTSIZE;
  //set our pointer to the buffer we created inside of the process
  displayInfo.item.pszText    = pTextItem;
  //set our flag for the data we wish to receive...let's try to get the text data and the lparam
  displayInfo.item.mask       = LVIF_TEXT | LVIF_PARAM;
//done filling in this data...the index and subitem will be filled inside of the for loops
 
  //write into the process memory our data sturct into the 
  //address given to us from the VirtualAllocEx
  DWORD bytesToWrite  = sizeof(NMLVDISPINFO) + MAXTEXTSIZE;
  DWORD bytesToRead   = sizeof(NMLVDISPINFO) + MAXTEXTSIZE;
  DWORD bytesWritten  = 0;
  DWORD bytesRead     = 0;
  BOOL result = false;
  //iterate through our rows and columns looking for data
  for(int row=0;row<count;row++)
  {
    for(int col=0;col<7;col++)
    { 
      //Fill in the col and row data of the DISPLAYINFO struct
      displayInfo.item.iItem     = row;
      displayInfo.item.iSubItem  = col;
      result = WriteProcessMemory(process, pdisplayInfo, &displayInfo, bytesToWrite, &bytesWritten);
      if((bytesToWrite !=  bytesWritten) || (!result))
      {
        //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"Write to process Failed");
      }
      else
      {//m_listControlHWND
        result = SendMessage((HWND)0x40648 ,WM_NOTIFY,displayInfo.hdr.idFrom,(LPARAM)pdisplayInfo);
 //       result = ReadProcessMemory(process, pdisplayInfo, &displayInfo, bytesToRead, &bytesRead);
        result = ReadProcessMemory(process, pTextItem, subitem, bytesToRead, &bytesRead);
      
        if((bytesToRead !=  bytesRead) || (!result))
        {
          //MDConsoleOutput::MDPrintln(MDConsoleOutput::SERVERITY_ERROR,"Read to process Failed");
        }
      }//end of if/else
    }//end of for col
  }//end of for row
             
  VirtualFreeEx(process, pdisplayInfo, 0, MEM_RELEASE); 
  VirtualFreeEx(process, pTextItem, 0, MEM_RELEASE); 

  return true;
} 



















/* MDFileReaderWriter *fw = new MDFileReaderWriter();
 fw->OpenFile("Memdump.bin",GENERIC_WRITE);

 unsigned char buf[500];
 DWORD offset=0;
 DWORD read;
 while(ReadProcessMemory(process, (void*)(_lvi + offset), &buf,  sizeof(buf)  ,&read))
 {
   fw->Write(buf,sizeof(buf));
   offset+=sizeof(buf);
 }*/
