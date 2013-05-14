#include "StdAfx.h"
#include "WatchDogControllerDestinationDll.h"
#include "DllInterface.h"

//
//
//
WatchDogControllerDestinationDll::WatchDogControllerDestinationDll(void)
{
}

//
//
//
WatchDogControllerDestinationDll::~WatchDogControllerDestinationDll(void)
{
}

//
//
//
void WatchDogControllerDestinationDll::DllInitialize()
{
	m_dlg.Create(IDD_WDCD_DLG, CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
}

//
//
//
void WatchDogControllerDestinationDll::DllUnInitialize()
{
	::DestroyWindow(m_dlg.GetSafeHwnd());
	if (m_com_link != NULL)
	{
		delete m_com_link;
	}
}

//
//
//
void WatchDogControllerDestinationDll::DllStart()
{
	//Create and Register Com Interace:
	m_com_link = new WatchDogComLink;
	m_com_link->Register(this, 9);		
}

//
//
//
void WatchDogControllerDestinationDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
}

//
//
//
void WatchDogControllerDestinationDll::DataReceived(char *source_name, void* data, int data_length)
{
	WatchDogControllerDestinationHeader* header = (WatchDogControllerDestinationHeader*) data;
	switch(header->op)
	{
		// accept ping message then signal back to server with pong message
		case (WatchDogControllerDestinationHeader::op_code::ping):
		{	
			m_dlg.ReturnMessage(source_name);
			SendMessageToSource(source_name);						
			m_dlg.SetTimer(1, 60*60*1000, NULL);
			break;
		}		
		case (WatchDogControllerDestinationHeader::op_code::restart):
		{
			RestartComputer();
			break;
		}
	}
}

//
//
//
bool WatchDogControllerDestinationDll::SendMessageToSource(char *dest_ip)
{
	byte buf[sizeof(WatchDogControllerDestinationHeader)];
	WatchDogControllerDestinationHeader* header = (WatchDogControllerDestinationHeader*)buf;
	header->op = WatchDogControllerDestinationHeader::pong;
	header->size = 0;
	return m_com_link->SendUnreliableData(dest_ip, buf, sizeof(WatchDogControllerDestinationHeader));

}

//
//
//
void WatchDogControllerDestinationDll::RestartComputer(void)
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
	 
	// Get a token for this process. 	 
	if (!OpenProcessToken(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		OutputDebugString("OpenProcessToken"); 
	 
	// Get the LUID for the shutdown privilege. 	 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
			&tkp.Privileges[0].Luid); 
	 
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	 
	// Get the shutdown privilege for this process. 	 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
			(PTOKEN_PRIVILEGES)NULL, 0); 
	 
	// Cannot test the return value of AdjustTokenPrivileges. 	 
	if (GetLastError() != ERROR_SUCCESS) 
		OutputDebugString("AdjustTokenPrivileges"); 
	 
	// Shut down the system and force all applications to close. 	 
	if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) 
		OutputDebugString("ExitWindowsEx"); 
}
