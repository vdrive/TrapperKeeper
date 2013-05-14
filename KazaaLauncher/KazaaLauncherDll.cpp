#include "StdAfx.h"
#include "kazaalauncherdll.h"
#include <Psapi.h>
#include "DllInterface.h"
#include "shlwapi.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
//#include "DirectoryCloner.h"


BOOL CALLBACK EnumKazaaWindow(HWND hwnd,LPARAM lParam)
{

	HWND dlg_hwnd = (HWND)lParam;

	//char class_name[1024+1];
	char caption[1024+1];
	char class_name[1024+1];

	::GetClassName(hwnd,class_name,sizeof(class_name)-1);
	::GetWindowText(hwnd, caption, sizeof(caption)-1);

	if( strcmp(caption, "Kazaa.kpp - Application Error")==0)
	{

		// Get the process id for this window	
		DWORD this_process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&this_process_id);
		
		//kill the dialog box
		::PostMessage(hwnd,WM_CLOSE,0,0);
		
		//kill this kazaa
		//parent->KillKazaa(this_process_id);
		char log[128];
		sprintf(log,"Killed Kazaa Crashed message box");
		::SendMessage(dlg_hwnd,WM_LOG,(WPARAM)log,0);
	}
	else if( (strcmp(class_name,"#32770")==0) && (strcmp(caption, "Error")==0) )
	{
		// Get the process id for this window	
		DWORD this_process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&this_process_id);
		
		//kill the dialog box
		::PostMessage(hwnd,WM_COMMAND,IDCANCEL,BN_CLICKED);
		//kill this kazaa
		::SendMessage(dlg_hwnd,WM_KILL_KAZAA,(WPARAM)this_process_id,0);
		char log[128];
		sprintf(log,"Killed DB file access violation message box for process ID: %d",this_process_id);
		::SendMessage(dlg_hwnd,WM_LOG,(WPARAM)log,0);
	}	

	return TRUE;	// continue the enumeration
}

//
//
//
BOOL CALLBACK EnumKazaaCrashedWindow(HWND hwnd,LPARAM lParam)
{

	KazaaLauncherDll* parent = (KazaaLauncherDll*)lParam;

	//char class_name[1024+1];
	char caption[1024+1];

	//GetClassName(hwnd,class_name,sizeof(class_name)-1);
	::GetWindowText(hwnd, caption, sizeof(caption)-1);

	if( strcmp(caption, "Kazaa.kpp - Application Error")==0)
	{

		// Get the process id for this window	
		DWORD this_process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&this_process_id);
		
		//kill the dialog box
		::PostMessage(hwnd,WM_CLOSE,0,0);
		
		//kill this kazaa
		//parent->KillKazaa(this_process_id);
		char log[128];
		sprintf(log,"Killed Kazaa Crashed message box");
		parent->Log(log);
	}	
	return TRUE;	// continue the enumeration
}

//
//
//
BOOL CALLBACK EnumMyDocumentWindow(HWND hwnd,LPARAM lParam)
{
	HWND dlg_hwnd = (HWND)lParam;

	//char class_name[1024+1];
	char caption[1024+1];

	//GetClassName(hwnd,class_name,sizeof(class_name)-1);
	::GetWindowText(hwnd, caption, sizeof(caption)-1);

	if( strcmp(caption, "My Documents")==0)
	{

		// Get the process id for this window	
		DWORD this_process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&this_process_id);
		
		//kill the dialog box
		::PostMessage(hwnd,WM_CLOSE,0,0);
		
		char log[128];
		sprintf(log,"Killed My Documents window");
		::SendMessage(dlg_hwnd,WM_LOG,(WPARAM)log,0);
	}	
	return TRUE;	// continue the enumeration
}

//
//
//
BOOL CALLBACK ProcessIcons(HWND hwnd, LPARAM lParam)
{
	RECT rect_tb;
	int  y;
	TCHAR szClassName[256];
	GetClassName(hwnd, szClassName, sizeof(szClassName)-1);

	if (_tcscmp(szClassName, _T("ToolbarWindow32")) == 0)
	{
		::GetClientRect (hwnd, &rect_tb);
		for (y=1; y< rect_tb.right; y++)
		{
			::SendMessage (hwnd, WM_MOUSEMOVE, 0, MAKELONG(1,y));
		}
		return FALSE;
	}
	return TRUE;
}


//
//
//
BOOL CALLBACK ProcessTray(HWND hwnd, LPARAM lParam)
{
	char name[1024+1];
	memset(name,0,sizeof(name));

	// Is this window Kazaa?
	GetClassName(hwnd, name, sizeof(name)-1);
	if(strcmp(name,"Shell_TrayWnd")==0)
	{
		HWND child = ::GetWindow(hwnd, GW_CHILD);
		while (child != NULL)
		{
			char buf[1024];
			memset(buf,0,sizeof(buf));
			GetClassName(child,buf,sizeof(buf));
			if(strcmp(buf,"TrayNotifyWnd")==0)
			{
				EnumChildWindows (child, ProcessIcons, 0);
				return FALSE;
			}
			child = ::GetWindow(child, GW_HWNDNEXT);	// get next sibling
		}
	}
	return TRUE;
}

//
//
//
BOOL CALLBACK ProcessTrayChildren(HWND hwnd, LPARAM lParam)
{
	TCHAR szClassName[256];
	GetClassName(hwnd, szClassName, sizeof(szClassName)-1);

	// Did we find the Tray? If so, enum that, then stop.
	if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)
	{
		EnumChildWindows (hwnd, ProcessIcons, 0);
		return FALSE;
	} 
	return TRUE;
}

void KillProcess(const char* process_name)
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(strcmp(process_name,name)==0)
			{
				/*
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;

				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);

				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
				*/

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				if(hHandle != NULL)
				{
					TerminateProcess(hHandle,0);
					CloseHandle(hHandle);
				}

				//break;  //no break, kill them all
			}
		}
		CloseHandle(handle);
	}
}

//
//
//
UINT KillingKazaaBoxesThreadProc(LPVOID pParam)
{
	KazaaLauncherDll* parent = (KazaaLauncherDll*)pParam;
	HWND hwnd = parent->m_dlg_hwnd;

	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);

	UINT loops = 0;
	HWND hTray = ::FindWindow ("Shell_TrayWnd", NULL);

	while(!parent->m_ending_thread)
	{
		loops++;
		if(desk1 == NULL)
			desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		if(desk2 == NULL)
			desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		if(desk3 == NULL)
			desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		if(hTray == NULL)
			hTray = ::FindWindow ("Shell_TrayWnd", NULL);

		EnumWindows((WNDENUMPROC)EnumKazaaWindow,(LPARAM)hwnd);
		EnumDesktopWindows(desk1,(WNDENUMPROC)EnumKazaaWindow,(LPARAM)hwnd);	
		EnumDesktopWindows(desk2,(WNDENUMPROC)EnumKazaaWindow,(LPARAM)hwnd);
		EnumDesktopWindows(desk3,(WNDENUMPROC)EnumKazaaWindow,(LPARAM)hwnd);
		
		KillProcess("dwwin.exe"); //killing the error report thingy

		//refresh system tray
		if(hTray != NULL)
		{
			EnumChildWindows(hTray, ProcessTrayChildren, 0);
			EnumDesktopWindows(desk1,(WNDENUMPROC)ProcessTray,0);
			EnumDesktopWindows(desk2,(WNDENUMPROC)ProcessTray,0);
			EnumDesktopWindows(desk3,(WNDENUMPROC)ProcessTray,0);
		}
		
		for(int i=0; i<20 && !parent->m_ending_thread; i++)
			Sleep(50);
	}

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);
	return 0;
}

UINT GetRealProcessIDThreadProc(LPVOID pParam)
{
	GetRealProcessIDThreadData *pThread_data = (GetRealProcessIDThreadData*)pParam;
	DWORD process_id=0;
	bool found_real_id = false;
	bool found_kpp = true; //if we don't see kpp.exe is running, we break the thread
	int trial = 0;
	while(!found_real_id && trial <= 300 && found_kpp && !pThread_data->m_ending_thread) //looking for the real process id for 300 seconds
	{
		if(trial > 30) //for the first 30 seconds, we don't care whether we find kpp.exe
			found_kpp = false;
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++)
		{ //for each of the processes
			DWORD nmod;
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));
			if(nmod>0)
			{
				char name[100];
				GetModuleBaseName(handle,modules[0],name,99);
				if(stricmp("Kazaa.kpp",name)==0)
				{
					bool found = false;
					for(unsigned int j=0; j<pThread_data->v_kazaa_processes.size();j++)
					{
						if(pThread_data->v_kazaa_processes[j].m_process_id == ids[i])
						{
							found = true;
							break;
						}
					}
					if(!found) //this is the new process id created by K++
					{
						pThread_data->m_process_id = ids[i];
						CloseHandle(handle);
						found_real_id = true;
						/*
						char msg[128];
						sprintf(msg,"Found kazaa.kpp id in %d seconds\n",trial);
						TRACE(msg);
						*/
						trial = 300;
						break;
					}
				}
				else if(stricmp("Kpp.exe",name)==0)
					found_kpp = true;
			}
			CloseHandle(handle);
		}
		trial++;
		if(trial < 301)
			Sleep(1000);
	}

	//if(pThread_data->m_process_id != 0) //delay the message for 3 sec
	//	Sleep(3*1000);
	::PostMessage(pThread_data->m_hwnd,WM_THREAD_DONE,0,0);	// the thread data is ready
	return 0;	// exit the thread
}

//
//
//
// callback function for killing all kazaas
BOOL CALLBACK EnumWindowProc(HWND hwnd,LPARAM lParam)
{
	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		//Kill kazaa by posting a message to kazaa's hwnd
		::PostMessage(hwnd,WM_COMMAND,0x00008066,0);
	}
	
	return TRUE;	// continue the enumeration
}

//
//
//callback function for killing the kazaa with process id
BOOL CALLBACK EnumWindowProcID(HWND hwnd,LPARAM lParam)
{
	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{

		// Get the process id for this window	
		DWORD this_process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&this_process_id);
		if(this_process_id == lParam)
		{
			//Kill kazaa by posting a message to kazaa's hwnd
			::PostMessage(hwnd,WM_COMMAND,0x00008066,0);

            return FALSE; //break the enumeration
		}
	}	
	return TRUE;	// continue the enumeration
}

//
//
//callback function for minimizing all kazaa windows
BOOL CALLBACK EnumMinimizeKazaaWindow(HWND hwnd,LPARAM lParam)
{
	//DWORD* process_id = (DWORD*)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		CRect rect;
		::GetWindowRect(hwnd, &rect);
		if(rect.bottom > 0 || rect.top > 0 || rect.left > 0 || rect.right > 0) //window is not minimize/trayed
			::PostMessage(hwnd,WM_CLOSE,0,0);
	}	
	return TRUE;	// continue the enumeration
}

//
//
//callback function for minimizing all kazaa windows
BOOL CALLBACK EnumRestoreKazaaWindow(HWND hwnd,LPARAM lParam)
{
	//DWORD* process_id = (DWORD*)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		::PostMessage(hwnd,WM_CLOSE,0,0);
	}	
	return TRUE;	// continue the enumeration
}

//callback function for minimizing all kazaa windows
BOOL CALLBACK EnumRestoreAndMinimizeKazaaWindow(HWND hwnd,LPARAM lParam)
{
	//DWORD* process_id = (DWORD*)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		CRect rect;
		::GetWindowRect(hwnd, &rect);
		if(rect.bottom < 0 && rect.top < 0 && rect.left < 0 && rect.right < 0)
		{
			::PostMessage(hwnd,WM_CLOSE,0,0); //restore
			Sleep(10);
			::PostMessage(hwnd,WM_CLOSE,0,0); //minimize
		}
	}	
	return TRUE;	// continue the enumeration
}

//
//
//
KazaaLauncherDll::KazaaLauncherDll(void)
{
	m_kazaa_instances = 0;
//	m_desktop_num = 0;
	m_request_supernodes_again = false;
	m_stop_kazaa = false;
	m_disable_minimizing_kazaa_windows = false;
}

//
//
//
KazaaLauncherDll::~KazaaLauncherDll(void)
{
}

//
//
//
void KazaaLauncherDll::DllInitialize()
{
	m_restart_desktop_control_threashold=0;
	m_msg_box_killing_thread=NULL;
	m_registry_monitoring_thread=NULL;
	m_kazaa_real_process_id_thread=NULL;

	srand((unsigned)time(NULL));
	srand(rand());

	ReadInUsernames();
    GetKazaaPathFromRegistry();

#ifndef KAZAA_KPP
	DeleteDatabaseDirOption();
#endif

	ChangeDefaultShareFolder();
	ChangeIgnoreAllMessageOption();
	ChangeMaxUploadOption();
	ChangeSupernodeOption();
	ChangeNoUploadLimitWhenIdleOption();

#ifdef KAZAA_KPP
	DisablePort1214();
	ChangeKppForceSupernodeOption();
	ChangeKppHDDScanOption();
	ChangeApplicationNameInRegistry("C:\\Program Files\\Kazaa Lite K++\\");
	ChangeExeDirInRegistry("C:\\Program Files\\Kazaa Lite K++\\");
	ChangeApplicationNameInCurrentUserRegistry("C:\\Program Files\\Kazaa Lite K++\\");
#endif
	GetKazaaPathFromRegistry();
	CString fasttrack = "C:\\FastTrack Shared";
	CString fucked = "C:\\syncher\\rcv\\Media.Fucked";
	vector<CString>folders;
	folders.push_back(fasttrack);
	folders.push_back(fucked);
	ReceivedSharedFolders(folders);

	m_dlg.Create(IDD_KAZAA_LAUNCHER_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg_hwnd = m_dlg.GetSafeHwnd();
	m_get_real_process_id_thread_data.m_hwnd = m_dlg_hwnd;
	

	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		m_sockets[i].InitParent(this);
	}

	m_next_port = 1215;
	
	m_disable_minimizing_kazaa_windows = true;
	m_getting_real_id = false;
	m_ending_thread = false;

	m_dlg.Log("Kazaa Launcher started");
}

//
//
//
void KazaaLauncherDll::DllUnInitialize()
{
	m_ending_thread = true;
	m_get_real_process_id_thread_data.m_ending_thread = true;

	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int j=0;j<num_sockets;j++)
	{
		m_sockets[j].Close();
	}

	if(p_com_interface != NULL)
	{
		delete p_com_interface;
	}
	StopMonitoringRegistryKey();

	//waiting for the threads to end
	if(m_getting_real_id)
	{
		if(WaitForSingleObject(m_kazaa_real_process_id_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
//			ASSERT(0);
		}
	}
	if(m_registry_monitoring_thread != NULL)
	{
		if(WaitForSingleObject(m_registry_monitoring_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
//			ASSERT(0);
		}
	}
	if(m_msg_box_killing_thread != NULL)
	{
		if(WaitForSingleObject(m_msg_box_killing_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
//			ASSERT(0);
		}
	}
}

//
//
//
void KazaaLauncherDll::DllStart()
{
	RestartDesktopControl();
	WSocket::Startup();
	//EnumAllKazaaProcesses();
	//GetAllKazaaCurrentSupernodes();


	//Create and register the com interface
	p_com_interface = new KLComInterface();
	//KazaaController uses op_code 11 for communication
	p_com_interface->Register(this, 11);

	m_syncher.Register(this, "Kazaa-Launcher");
	m_syncher.InitParent(this);


	//m_dlg.SetTimer(1,60*1000,0); //1 min restore and minimize all kazaa windows
	m_dlg.SetTimer(2, 5*60*1000,NULL); //check kazaa connection status every 5 min
	m_dlg.SetTimer(3, 15*1000, NULL); //launch kazaa every 15 seconds
	m_dlg.SetTimer(5, 60*1000,NULL); //60 sec minimize all kazaa windows & check number of supernodes
	m_dlg.SetTimer(6, 30*60*1000,NULL); //kill all My Documents Windows every 30 mins

	ReadMaxKazaaNumber();

	KillProcess("dwwin.exe"); //kill the error report thingy
	KillProcess("Kpp.exe"); //kill the kazaa starter app
	KillAllKazaaProcesses(); //kill all kazaa at the start of the program

	m_msg_box_killing_thread = AfxBeginThread(KillingKazaaBoxesThreadProc,(LPVOID)this,THREAD_PRIORITY_NORMAL);
	KillAllMyDocumentaWindows();
	//debug testing
}

//
//
//
// get all kazaa listening tcp ports from netstatp
void KazaaLauncherDll::UpdateKazaaListeningPort()
{
	v_kazaa_listening_tcp_ports.clear();
	m_netstatp.StartEnumerating(v_kazaa_listening_tcp_ports);
}
//
//
//
void KazaaLauncherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void KazaaLauncherDll::ReportStatus(HttpSocket* socket, char* ip,int port,int error,NetInfo& net_info,char *username,char *supernode_ip,char *supernode_port)
{
	socket->Close();

	if(net_info.m_process_id != 0) //this socket was for getting local supernodes
	{
		if(error==0)
		{
			//check to see if this port reports it's current kazaa supernode
			if(strlen(supernode_ip)!=0 && strlen(supernode_port)!=0)
			{
				bool found = false;
				//for(unsigned int j=0; j<v_temp_kazaa_processes.size();j++) //look for it's PID
				for(unsigned int j=0; j<v_kazaa_processes.size();j++) //look for it's PID
				{
					//if(v_temp_kazaa_processes[j].m_process_id == net_info.m_process_id)
					if(v_kazaa_processes[j].m_process_id == net_info.m_process_id)
					{
						found = true;
						int int_port = atoi(supernode_port);
						//v_temp_kazaa_processes[j].m_supernode_ip = supernode_ip;
						v_kazaa_processes[j].m_connected_supernode_ip = supernode_ip;
						//v_temp_kazaa_processes[j].m_supernode_port = port;
						v_kazaa_processes[j].m_connected_supernode_port = int_port;
						//m_dlg.UpdateSupernodeList(v_temp_kazaa_processes[j]);
						m_dlg.UpdateSupernodeList(v_kazaa_processes[j]);

						//check to see if the supernode jumped
						if(strcmp(v_kazaa_processes[j].m_connecting_supernode_ip.c_str(),supernode_ip)!=0)
							//|| v_kazaa_processes[j].m_connecting_supernode_port != int_port)
						{
							UpdatedIPAndPort jumped_supernode;	//supernode that jumped and send Dest_Check_New_Supernode message
							jumped_supernode.m_old_ip = GetIntIPFromStringIP(v_kazaa_processes[j].m_connecting_supernode_ip);
							jumped_supernode.m_old_port = v_kazaa_processes[j].m_connecting_supernode_port;
							jumped_supernode.m_new_ip = GetIntIPFromStringIP(v_kazaa_processes[j].m_connected_supernode_ip);
							jumped_supernode.m_new_port = int_port;
							TRACE("SendCheckNewSupernode for process ID: %d\n",v_kazaa_processes[j].m_process_id);
							SendCheckNewSupernode(jumped_supernode);
						}
						else
						{
							if(v_kazaa_processes[j].m_has_reported_to_kazaa_supply_taker == false)
							{
								AppID app_id;
								app_id.m_app_id = 7; //kazaa supply taker
								bool ret = false;
								SendLocalDllMessage(app_id,(byte*)&v_kazaa_processes[j].m_process_id, sizeof(DWORD),
									KazaaControllerHeader::New_Kazaa_Launched, ret);
								v_kazaa_processes[j].m_has_reported_to_kazaa_supply_taker = ret;
								m_dlg.UpdateSupernodeList(v_kazaa_processes[j]);
								TRACE("Sent local Dll msg New_Kazaa_Launched for process ID: %d, reply: %d\n",v_kazaa_processes[j].m_process_id,ret);

								/*
								char msg[256];
								sprintf(msg,"Sent process ID: %d to Kazaa Collector. Reply: ", v_kazaa_processes[j].m_process_id);
								if(ret)
									strcat(msg, "true");
								else
									strcat(msg,"false");
								m_dlg.Log(msg);
								*/
							}
						}
						break;
					}
				}
				//if(!found) //push this process into the vector if not found
				if(!found)//shouldn't happen, well, we will kill this process
				{
					char msg[256];
					sprintf(msg,"Killing Kazaa PID: %d - Kazaa is not found in internal process list (ReportStatus)", net_info.m_process_id);
					m_dlg.Log(msg);
					KillKazaa(net_info.m_process_id);
				}
			}
			else //this process appears to be either a supernode itself or still connecting
			{
				//check how long this process has been running
				if(GetProcessRunningTime(net_info.m_process_id) > 60) //more than 1 min, kill it
				{
					//notify the controller if it is in process list
					for(unsigned int j=0; j<v_kazaa_processes.size();j++) //look for it's PID
					{
						if(v_kazaa_processes[j].m_process_id == net_info.m_process_id)
						{
							IPAndPort ip_port;
							ip_port.m_ip = GetIntIPFromStringIP(v_kazaa_processes[j].m_connecting_supernode_ip);
							ip_port.m_port = v_kazaa_processes[j].m_connecting_supernode_port;
							SendRequestNewSupernode(ip_port);
							break;
						}
					}
					char msg[256];
					sprintf(msg,"Killing Kazaa PID: %d - Kazaa didn't connect in 60 sec (ReportStatus)", net_info.m_process_id);
					m_dlg.Log(msg);
					KillKazaa(net_info.m_process_id);
				}
			}
		}
	}
}

//
//
//
bool KazaaLauncherDll::SendCheckNewSupernode(vector<UpdatedIPAndPort>& ips)
{
	if(m_controller_source.length()!=0)
	{
		for(UINT i=0;i<ips.size();i++)
		{
			UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*4;
			byte* buf = new byte[data_length];
			KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
			header->op = KazaaControllerHeader::Dest_Check_New_Supernode;
			header->size = data_length - sizeof(KazaaControllerHeader);

			byte* ptr = &buf[sizeof(KazaaControllerHeader)];
			*((int*)ptr) = ips[i].m_old_ip;
			ptr+=sizeof(int);
			*((int*)ptr) = ips[i].m_old_port;
			ptr+=sizeof(int);
			*((int*)ptr) = ips[i].m_new_ip;
			ptr+=sizeof(int);
			*((int*)ptr) = ips[i].m_new_port;
			ptr+=sizeof(int);

			char dest[32];
			strcpy(dest, m_controller_source.c_str());
			p_com_interface->SendReliableData(dest, buf, data_length);
			//TRACE("Kazaa Launcher: Sent message \"Dest_Check_New_Supernode\" to %s\n",m_controller_source);
			delete [] buf;
		}
		return true;
	}
	return false;
}

//
//
//
bool KazaaLauncherDll::SendCheckNewSupernode(UpdatedIPAndPort& ip)
{
	if(m_controller_source.length()!=0)
	{
		UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*4;
		byte* buf = new byte[data_length];
		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Dest_Check_New_Supernode;
		header->size = data_length - sizeof(KazaaControllerHeader);

		byte* ptr = &buf[sizeof(KazaaControllerHeader)];
		*((int*)ptr) = ip.m_old_ip;
		ptr+=sizeof(int);
		*((int*)ptr) = ip.m_old_port;
		ptr+=sizeof(int);
		*((int*)ptr) = ip.m_new_ip;
		ptr+=sizeof(int);
		*((int*)ptr) = ip.m_new_port;
		ptr+=sizeof(int);

		char dest[32];
		strcpy(dest, m_controller_source.c_str());
		p_com_interface->SendReliableData(dest, buf, data_length);
		//TRACE("Kazaa Launcher: Sent message \"Dest_Check_New_Supernode\" to %s\n",m_controller_source);
		delete [] buf;
		return true;
	}
	return false;
}

//
//
//
bool KazaaLauncherDll::SendRemoveSupernode(vector<IPAndPort>& ips)
{
	if(m_controller_source.length()!=0)
	{
		for(UINT i=0;i<ips.size();i++)
		{
			UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*2;
			byte* buf = new byte[data_length];
			KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
			header->op = KazaaControllerHeader::Dest_Remove_Supernode;
			header->size = data_length - sizeof(KazaaControllerHeader);

			byte* ptr = &buf[sizeof(KazaaControllerHeader)];
			*((int*)ptr) = ips[i].m_ip;
			ptr+=sizeof(int);
			*((int*)ptr) = ips[i].m_port;
			ptr+=sizeof(int);
			
			char dest[32];
			strcpy(dest, m_controller_source.c_str());
			p_com_interface->SendReliableData(dest, buf, data_length);
			//TRACE("Kazaa Launcher: Sent message \"Dest_Remove_Supernode\" to %s\n",m_controller_source);
			delete [] buf;
		}
		return true;
	}
	return false;
}

//
//
//
bool KazaaLauncherDll::SendRemoveSupernode(IPAndPort& ip)
{
	if(m_controller_source.length()!=0)
	{
		UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*2;
		byte* buf = new byte[data_length];
		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Dest_Remove_Supernode;
		header->size = data_length - sizeof(KazaaControllerHeader);

		byte* ptr = &buf[sizeof(KazaaControllerHeader)];
		*((int*)ptr) = ip.m_ip;
		ptr+=sizeof(int);
		*((int*)ptr) = ip.m_port;
		ptr+=sizeof(int);

		char dest[32];
		strcpy(dest, m_controller_source.c_str());
		p_com_interface->SendReliableData(dest, buf, data_length);
		//TRACE("Kazaa Launcher: Sent message \"Dest_Remove_Supernode\" to %s\n",m_controller_source);
		delete [] buf;
		return true;
	}
	return false;
}

//
//
//
bool KazaaLauncherDll::SendRequestNewSupernode(vector<IPAndPort> & ips)
{
	if(m_controller_source.length()!=0)
	{
		for(UINT i=0;i<ips.size();i++)
		{
			UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*2;
			byte* buf = new byte[data_length];
			KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
			header->op = KazaaControllerHeader::Dest_Request_New_Supernode;
			header->size = data_length - sizeof(KazaaControllerHeader);

			byte* ptr = &buf[sizeof(KazaaControllerHeader)];
			*((int*)ptr) = ips[i].m_ip;
			ptr+=sizeof(int);
			*((int*)ptr) = ips[i].m_port;
			ptr+=sizeof(int);

			char dest[32];
			strcpy(dest, m_controller_source.c_str());
			p_com_interface->SendReliableData(dest, buf, data_length);
			//TRACE("Kazaa Launcher: Sent message \"Dest_Request_New_Supernode\" to %s\n",m_controller_source);
			delete [] buf;
		}
		return true;
	}
	return false;
}

//
//
//
bool KazaaLauncherDll::SendRequestNewSupernode(IPAndPort & ip)
{
	if(m_controller_source.length()!=0)
	{
		UINT data_length = sizeof(KazaaControllerHeader)+sizeof(int)*2;
		byte* buf = new byte[data_length];
		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Dest_Request_New_Supernode;
		header->size = data_length - sizeof(KazaaControllerHeader);

		byte* ptr = &buf[sizeof(KazaaControllerHeader)];
		*((int*)ptr) = ip.m_ip;
		ptr+=sizeof(int);
		*((int*)ptr) = ip.m_port;
		ptr+=sizeof(int);

		char dest[32];
		strcpy(dest, m_controller_source.c_str());
		p_com_interface->SendReliableData(dest, buf, data_length);
		//TRACE("Kazaa Launcher: Sent message \"Dest_Request_New_Supernode\" to %s\n",m_controller_source);
		delete [] buf;
		return true;
	}
	return false;
}

//
//
//
void KazaaLauncherDll::GetAllKazaaCurrentSupernodes()
{
	UpdateKazaaListeningPort();
	for(unsigned int i=0; i<v_kazaa_listening_tcp_ports.size(); i++)
	{
		int j;

		int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
		int socket_index=-1;
//		while(true)
//		{
			// See if we can find another idle socket
			for(j=0;j<num_sockets;j++)
			{
				if(m_sockets[j].IsSocket()==false)
				{
					socket_index=j;
					break;
				}
			}
			// We found an idle socket
			if(socket_index!=-1)
			{
//				break;
			}
			else
			{
				Sleep(50);
				KillAllIdleSockets();
			}
//		}
		if(socket_index!=-1)
		{
			m_sockets[socket_index].Create();
			m_sockets[socket_index].Connect("127.0.0.1",v_kazaa_listening_tcp_ports[i],CTime::GetCurrentTime());
		}

	}
}

//
//
//
void KazaaLauncherDll::EnumAllKazaaProcesses()
{
	//v_temp_kazaa_processes.clear();
	ResetAllKazaaProcessesAliveFlag();
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
#ifdef KAZAA_KPP
			if(stricmp("Kazaa.kpp",name)==0)//if process is named kazaa, add it to the kazaa process list, K++ version
#else
			if(stricmp("Kazaa.exe",name)==0)//if process is named kazaa, add it to the kazaa process list
#endif
			{
				bool found = false;
				for(unsigned int j=0; j<v_kazaa_processes.size();j++)
				{
					if(v_kazaa_processes[j].m_process_id == ids[i])
					{
						found = true;
						m_dlg.UpdateSupernodeList(v_kazaa_processes[j]);
						v_kazaa_processes[j].m_alive=true;
						break;
					}
				}
				if(!found) //kill it
				{
					char msg[256];
					sprintf(msg,"Killing Kazaa PID: %d - Kazaa is not found in internal process list (EnumAllKazaaProcesses)", ids[i]);
					m_dlg.Log(msg);
					KillKazaa(ids[i]);
				}
			}
		}
		CloseHandle(handle);
	}
	RemoveAllGoneProcesses();
}

//
//
//
void KazaaLauncherDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote SD Dll data
	KazaaControllerHeader* header = (KazaaControllerHeader*)data;
	byte *pData=(byte *)data;
	pData+=sizeof(KazaaControllerHeader);
	
	switch(header->op)
	{
		case(KazaaControllerHeader::Source_Init):
		{
			TRACE("Kazaa Launcher: Received message \"Source_Init\" from %s\n",source_name);
			m_controller_source = source_name;
			//Send init response here
			SendGenericMessage(source_name, KazaaControllerHeader::Dest_Init_Response);
			//TRACE("Kazaa Launcher: Sent message \"Dest_Init_Response\" to %s",source_name);
			break;
		}
		/* This functionality is controlled by KazaaLauncherInterface now
		case(KazaaControllerHeader::Source_Number_Of_Kazaa_To_Run):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				m_kazaa_instances = *((UINT*)pData);
				TRACE("Kazaa Launcher: Received message \"Source_Number_Of_Kazaa_To_Run\", Number of Kazaa Instances: %u\n", m_kazaa_instances);
				//Check to see if we have enough supernodes
				CheckNumberOfSupernodes();
			}
			else
			{
				TRACE("Kazaa Launcher: Received message \"Source_Number_Of_Kazaa_To_Run\" from a non original source");
			}
			break;
		}
		*/
		case(KazaaControllerHeader::Source_Ran_Out_Of_Supernodes):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Ran_Out_Of_Supernodes\" from %s\n",source_name);
				m_request_supernodes_again = true;
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Ran_Out_Of_Supernodes\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Request_Current_Supernodes_list):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Request_Current_Supernodes_list\" from %s\n",source_name);
				SendAllCurrentSupernodes(source_name);
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Request_Current_Supernodes_list\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Supernodes_Reply):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Supernodes_Reply\" from %s\n",source_name);
				int *ip_num = (int*)pData;
				pData+=sizeof(int);
				for (int i = 0; i<(*ip_num); i++)
				{
					KazaaStatusInfo info;
					info.m_ip = *((int*)pData);
					pData+=sizeof(int);
					info.m_port = *((int*)pData);
					pData+=sizeof(int);
					v_available_info.push_back(info);
				}
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Supernodes_Reply\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Renew_Supernode):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				TRACE("Kazaa Launcher: Received message \"Source_Renew_Supernode\" from %s\n",source_name);
				KazaaStatusInfo info;

				int old_ip =*((int*)pData);		//old ip and port
				pData+=sizeof(int);
				int old_port = *((int*)pData);
				pData+=sizeof(int);

				string ip = GetIPStringFromIPInt(old_ip);
				
				for(UINT i=0;i<v_kazaa_processes.size();i++)
				{
					if(v_kazaa_processes[i].m_connected_supernode_ip.compare(ip) == 0
						&& v_kazaa_processes[i].m_connected_supernode_port == old_port)
					{
						char msg[256];
						sprintf(msg,"Killing Kazaa PID: %d - Renewing supernode", v_kazaa_processes[i].m_process_id);
						m_dlg.Log(msg);
						KillKazaa(v_kazaa_processes[i].m_process_id);
						break;
					}
				}		

				info.m_ip = *((int*)pData);		//new ip and port
				pData+=sizeof(int);
				info.m_port = *((int*)pData);
				pData+=sizeof(int);				
				v_available_info.push_back(info);	//push it to supernodes pool
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Renew_Supernode\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Kill_All_Kazaa):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Kill_All_Kazaa\" from %s\n",source_name);
				char msg[256];
				sprintf(msg, "Received remote message from %s, stop launching and killing all kazaas",source_name);
				m_dlg.Log(msg);

				StopLaunchingKazaa();
				KillAllKazaaProcesses();
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Kill_All_Kazaa\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Launch_All_Kazaa):
		{
			//check to see if this message was from our source

			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Launch_All_Kazaa\" from %s\n",source_name);
				char msg[256];
				sprintf(msg, "Received remote message from %s, resume launching all kazaas",source_name);
				m_dlg.Log(msg);
				ResumeLaunchingKazaa();
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Launch_All_Kazaa\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Restart_All_Kazaa):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Restart_All_Kazaa\" from %s\n",source_name);
				char msg[256];
				sprintf(msg, "Received remote message from %s, restart all kazaas",source_name);
				m_dlg.Log(msg);
				StopLaunchingKazaa();
				KillAllKazaaProcesses();
				ResumeLaunchingKazaa();
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Restart_All_Kazaa\" from a non original source");
			}
			break;
		}
		case(KazaaControllerHeader::Source_Shared_Folders):
		{
			vector<CString> folders;
			UINT num_folders = *(UINT*)pData;
			pData += sizeof(UINT);
			for(UINT i=0; i<num_folders; i++)
			{
				CString folder = (char*)pData;
				pData+=folder.GetLength()+1;
				folders.push_back(folder);
			}
			ReceivedSharedFolders(folders);
		}
		case(KazaaControllerHeader::Current_Supernode_Is_Ok):
		{
			//check to see if this message was from our source
			if(strcmp(m_controller_source.c_str(), source_name)==0)
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Renew_Supernode\" from %s\n",source_name);
				//KazaaStatusInfo info;

				int connected_ip =*((int*)pData);		//old ip and port
				pData+=sizeof(int);
				int connected_port = *((int*)pData);
				pData+=sizeof(int);

				string ip = GetIPStringFromIPInt(connected_ip);
				
				for(UINT i=0;i<v_kazaa_processes.size();i++)
				{
					if(v_kazaa_processes[i].m_connected_supernode_ip.compare(ip) == 0
						&& v_kazaa_processes[i].m_connected_supernode_port == connected_port)
					{
						v_kazaa_processes[i].m_connecting_supernode_ip = v_kazaa_processes[i].m_connected_supernode_ip;
						v_kazaa_processes[i].m_connecting_supernode_port = v_kazaa_processes[i].m_connected_supernode_port;
						if(v_kazaa_processes[i].m_has_reported_to_kazaa_supply_taker == false)
						{
							AppID app_id;
							app_id.m_app_id = 7; //kazaa supply taker
							bool ret = false;
							SendLocalDllMessage(app_id,(byte*)&v_kazaa_processes[i].m_process_id, sizeof(DWORD),
								KazaaControllerHeader::New_Kazaa_Launched, ret);
							v_kazaa_processes[i].m_has_reported_to_kazaa_supply_taker = ret;
							TRACE("Sent local Dll msg New_Kazaa_Launched for process ID: %d, reply: %d\n",v_kazaa_processes[i].m_process_id,ret);
						}
						m_dlg.UpdateSupernodeList(v_kazaa_processes[i]);
						break;
					}
				}		
			}
			else
			{
				//TRACE("Kazaa Launcher: Received message \"Source_Renew_Supernode\" from a non original source");
			}
			break;
		}
	}
}

//
//
//
bool KazaaLauncherDll::SendAllCurrentSupernodes(char* dest)
{
	bool ret;
	byte* buf;
	UINT data_length = sizeof(KazaaControllerHeader)
						+sizeof(UINT) //number of processes
						+sizeof(UINT) //number of IPs
						+(2*sizeof(int)*v_kazaa_processes.size())
						+(2*sizeof(int)*v_available_info.size()); //supernodes that we haven't used

	buf = new byte[data_length]; 

	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Dest_Current_Supernodes_list;
	header->size = data_length - sizeof(KazaaControllerHeader);
	
	UINT *num = (UINT *)&buf[sizeof(KazaaControllerHeader)];
	*num = (UINT)v_kazaa_processes.size();

	byte* ptr = &buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];

	*((UINT*)ptr) = (UINT)v_kazaa_processes.size()+(UINT)v_available_info.size(); //number of supernodes
	ptr+=sizeof(UINT);
	
	UINT i=0;
	UINT j=0;
	
	for(i=0; i<v_kazaa_processes.size();i++)
	{
		int ip = GetIntIPFromStringIP(v_kazaa_processes[i].m_connected_supernode_ip);
		if(ip==0)
			ip = GetIntIPFromStringIP(v_kazaa_processes[i].m_connecting_supernode_ip);
		*((int *)ptr) = ip;
		ptr += sizeof(int);
		int port = v_kazaa_processes[i].m_connected_supernode_port;
		if(port==0)
			port = v_kazaa_processes[i].m_connecting_supernode_port;
		*((int *)ptr) = port;
		ptr += sizeof(int);
	}
	for(i=0; i<v_available_info.size();i++)
	{
		*((int *)ptr) = v_available_info[i].m_ip;
		ptr += sizeof(int);
		*((int *)ptr) = v_available_info[i].m_port;
		ptr += sizeof(int);
	}
	ret =  p_com_interface->SendReliableData(dest,buf,data_length);
	//TRACE("Kazaa Launcher: Send message \"Dest_Current_Supernodes_list\" to %s\n",dest);
	delete [] buf;
	return ret;
}
//
//
//
bool KazaaLauncherDll::SendGenericMessage(char* dest, KazaaControllerHeader::op_code op_code)
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = op_code;
	header->size = 0;
	return p_com_interface->SendReliableData(dest,buf,sizeof(KazaaControllerHeader));
}


//
//
//
bool KazaaLauncherDll::SendSupernodesRequest(char* dest, int num_request)
{
	if(num_request > 0 && num_request < 200) //not allowing to request more than 200 supernodes
	{
		byte buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];
		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Dest_Supernode_Request;
		header->size = sizeof(UINT);

		UINT* num = (UINT*)&buf[sizeof(KazaaControllerHeader)];
		*num = (UINT)num_request;

		//TRACE("Kazaa Launcher: Sent message \"Dest_Supernode_Request\"\n");
		return p_com_interface->SendReliableData(dest,buf,sizeof(KazaaControllerHeader)+sizeof(UINT));
	}
	return false;
}

//
//
//
void KazaaLauncherDll::OnTimer(int nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:
		{
//			CheckMemoryAndWhackIt(20*1024*1024); //whack all kazaa uses more than 20MB memory
			/*
			EnumWindows((WNDENUMPROC)EnumRestoreAndMinimizeKazaaWindow,(LPARAM)0);
			HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
			EnumDesktopWindows(desk1,(WNDENUMPROC)EnumRestoreAndMinimizeKazaaWindow,NULL);	
			HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
			EnumDesktopWindows(desk2,(WNDENUMPROC)EnumRestoreAndMinimizeKazaaWindow,NULL);
			HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
			EnumDesktopWindows(desk3,(WNDENUMPROC)EnumRestoreAndMinimizeKazaaWindow,NULL);

			//Close all Desktop Handles
			CloseDesktop(desk1);
			CloseDesktop(desk2);
			CloseDesktop(desk3);
			*/
			break;
		}
		case 2:
		{
//			m_dlg.Log("Timer 2 fired");
			m_dlg.m_supernode_list.DeleteAllItems();
			EnumAllKazaaProcesses();
			GetAllKazaaCurrentSupernodes();
			break;
		}
		case 3:
		{
//			m_dlg.Log("Timer 3 fired");
			LaunchKazaa();
			break;
		}
		case 5:
		{
//			m_dlg.Log("Timer 5 fired");
			if(m_disable_minimizing_kazaa_windows == false)
				MinimizeAllKazaaWindows();
			CheckNumberOfSupernodes();
			break;
		}
		case 6:
		{
//			m_dlg.Log("Timer 6 fired");
			KillAllMyDocumentaWindows;
			break;
		}
	}
}

//
//
//
int KazaaLauncherDll::GetIntIPFromStringIP(string ip)
{
	int ip_int = 0;
	if(ip.length()==0)
		return ip_int;
	
	int ip1,ip2,ip3,ip4;
	sscanf(ip.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	return ip_int;
}

//
//
//
void KazaaLauncherDll::GetKazaaPathFromRegistry()
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Cloudload");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE, &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			BYTE temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = MAX_PATH;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				temp, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ExeDir") == 0)
			{
				m_kazaa_path = (char*)temp;
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
	char* ptr = strstr(m_kazaa_path.c_str(), ".exe");
	if(ptr == NULL)
	{
#ifdef KAZAA_KPP
		m_kazaa_path += "\\kpp.exe"; //K++ version
		//m_kazaa_path += "\\klrun.exe"; //K++ version
#else
		m_kazaa_path += "\\Kazaa.exe";
#endif
	}
	else
	{
		CString kpp_filename = m_kazaa_path.c_str();
		int index = kpp_filename.ReverseFind('\\');
		kpp_filename.Delete(index, kpp_filename.GetLength()-index);
		kpp_filename+="\\kpp.exe";
		m_kazaa_path = kpp_filename;
	}
	//MessageBox(NULL,m_kazaa_path.c_str(),"Kazaa Path", MB_OK);

}

//
//
//
void KazaaLauncherDll::LaunchKazaa()
{

	if(!m_stop_kazaa && !m_getting_real_id)
	{
		if((int)v_kazaa_processes.size() < m_kazaa_instances && (int)v_available_info.size() > 0)
		{
			KazaaProcess process;
			//First Change Registry for the Supernode
			EditRegistry(v_available_info[0].m_ip,v_available_info[0].m_port);
			//process.m_supernode_port = v_available_info[0].m_port;
			process.m_connecting_supernode_port = v_available_info[0].m_port;
			//process.m_supernode_ip = GetIPStringFromIPInt(v_available_info[0].m_ip);
			process.m_connecting_supernode_ip = GetIPStringFromIPInt(v_available_info[0].m_ip);

			string run_in_dir = GetFreeRunInDir();

			int port = GetFreePort();
			ChangeKazaaPortInRegistry(port);
			
			if(m_registry_monitoring_thread == NULL)
				StartMonitoringRegistry(run_in_dir.c_str());
			else
			{
				CSingleLock singleLock(&m_critical_section);
				singleLock.Lock();
				if(singleLock.IsLocked())
				{
					m_thread_data.m_database_dir = run_in_dir.c_str();
					singleLock.Unlock();
				}
			}

			ChangeDataBaseDirOptionInRegistry(run_in_dir);

			LPCTSTR lpszExePath;
			LPCTSTR lpszRunIn;
			LPTSTR lpszCommand;
			lpszExePath = m_kazaa_path.c_str();
			CString kpp_filename = m_kazaa_path.c_str();
			int index = kpp_filename.ReverseFind('\\');
			kpp_filename.Delete(index, kpp_filename.GetLength()-index);
			kpp_filename+="\\Kazaa.kpp";
			char command[256+1];

			strcpy(command, "\"");
			strcat(command, m_kazaa_path.c_str());
			strcat(command, "\"");

			strcat(command, " ");
			strcat(command, "\"");
			strcat(command, kpp_filename); //K++ edition
			strcat(command, "\"");

			lpszCommand =  command;

			lpszRunIn = run_in_dir.c_str();

			PROCESS_INFORMATION pi;
			STARTUPINFOA si = {sizeof si};
			
			// Select which Desktop to run on
			process.m_desktop_num = GetLeastKazaaDesktopNum();
			process.m_running_dir = run_in_dir.c_str();
			switch(process.m_desktop_num)
			{
			case(0):
				{
					si.lpDesktop = "Default";
					break;
				}
			case(1):
				{
					si.lpDesktop = "Desktop1";
					break;
				}
			case(2):
				{
					si.lpDesktop = "Desktop2";
					break;
				}
			case(3):
				{
					si.lpDesktop = "Desktop3";
					break;
				}
			default:
				{
					break;
				}
			}
			BOOL bret = CreateProcess(
				NULL,
				lpszCommand,
				NULL,
				NULL,
				FALSE,
				BELOW_NORMAL_PRIORITY_CLASS,
				NULL,
				lpszRunIn,
				&si,
				&pi);

			if(bret != 0)
			{
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);

				m_get_real_process_id_thread_data.m_process_id = 0;
				m_get_real_process_id_thread_data.v_kazaa_processes = v_kazaa_processes;
				v_kazaa_processes.push_back(process);
				m_getting_real_id = true;
				m_thread_begins_at = CTime::GetCurrentTime();
				m_kazaa_real_process_id_thread = AfxBeginThread(GetRealProcessIDThreadProc,(LPVOID)&m_get_real_process_id_thread_data,THREAD_PRIORITY_NORMAL);
				m_dlg.Log("scanning kazaa.kpp thread started");
			}
			else
			{
				DWORD err = GetLastError();
				char msg[256+1];
				sprintf(msg,"Fail to call CreateProcess() in %s on Desktop %d", process.m_running_dir, process.m_desktop_num);
				m_dlg.Log(msg);
			}
		}
		else
		{
			char msg[256];
			sprintf(msg,"NOT launching Kazaa - ");
			if((int)v_kazaa_processes.size() >= m_kazaa_instances)
				strcat(msg,"v_kazaa_processes.size() >= m_kazaa_instances - ");
			if(v_available_info.size() == 0)
				strcat(msg,"v_available_info == 0");

			if((int)v_kazaa_processes.size() + (int)v_available_info.size() < m_kazaa_instances)
			{
				if(m_controller_source.length()!=0)
				{
					char dest[32];
					strcpy(dest, m_controller_source.c_str());
					SendSupernodesRequest(dest,m_kazaa_instances-(int)v_kazaa_processes.size()-(int)v_available_info.size());
				}
			}
			m_dlg.Log(msg);
		}
	}
	else
	{
		char msg[256];
		sprintf(msg,"NOT launching Kazaa - ");
		if(m_stop_kazaa)
			strcat(msg, "stop launching flag is ON - ");
		if(m_getting_real_id)
			strcat(msg, "getting real ID flag is ON");
		m_dlg.Log(msg);
	}
}

//
//
//
void KazaaLauncherDll::EditRegistry(int ip, int port)
{

	CHAR reg_sub_key[] = TEXT("Software");
	HKEY hkcu, hkazaa, org_key;
	DWORD disposition;


	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &org_key);


	l = RegOpenCurrentUser(KEY_ALL_ACCESS, &hkcu);
	l = RegCreateKeyEx(hkcu,TEXT("Software\\Kazaa"),0,NULL,REG_OPTION_NON_VOLATILE,
	KEY_ALL_ACCESS,NULL,&hkazaa,&disposition);
	l = SHCopyKey(org_key, "Kazaa", hkazaa, NULL);
	RandomlyModifyUsername(hkcu);
	l = RegCloseKey(hkazaa);
	l = RegCloseKey(hkcu);


	unsigned int ip1,ip2,ip3,ip4;
	ip1 = (ip>>0)&0xFF;
	ip2 = (ip>>8)&0xFF;
	ip3 = (ip>>16)&0xFF;
	ip4 = (ip>>24)&0xFF;
	unsigned char reg_key[10];
	memset(reg_key,0,sizeof(reg_key));
	reg_key[0]=0x00;
	reg_key[1]=(unsigned char)ip4;
	reg_key[2]=(unsigned char)ip3;
	reg_key[3]=(unsigned char)ip2;
	reg_key[4]=(unsigned char)ip1;
	reg_key[5]=(unsigned char)((port>>0)&0xFF);
	reg_key[6]=(unsigned char)((port>>8)&0xFF);
	reg_key[7]=0x01;
	reg_key[8]=0x02;
	reg_key[9]=0x00;

	// Write to registry
	HKEY hKey;
	char subkey[]="SOFTWARE\\KaZaA\\ConnectionInfo";
	RegCreateKeyEx(HKEY_LOCAL_MACHINE,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(reg_key);
	RegSetValueEx(hKey,"KazaaNet",0,REG_BINARY,reg_key,cbData);
	
	RegCloseKey(hKey);
}

//
//
//
void KazaaLauncherDll::RandomlyModifyUsername(HKEY hkcu)
{
	if(v_usernames.size()>0)
	{
		int length = (rand()%7)+4;
		char username[32+1];
		char email[64];
		memset(&username, 0, sizeof(username));
		memset(&email, 0, sizeof(email));

		UINT randuser;
		randuser = rand()%(UINT)v_usernames.size();
		strcpy(username,v_usernames[randuser]);

		//change the username to fit our naming scheme
		ChangeUsername(username);

		strcpy(email,username);
		strcat(email, "@kazaa.com");

		CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\UserDetails");
		HKEY hkey;
		LONG l = RegOpenKeyEx(hkcu, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
		if( l == ERROR_SUCCESS )
		{
			int i= 0;
			int count=0;

			while( true)
			{
				TCHAR szName[MAX_PATH];
				//TCHAR temp[MAX_PATH];
				::ZeroMemory(&szName, MAX_PATH);
				DWORD szName_size = sizeof(szName);
				DWORD data_size = 0;

				l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
					NULL, &data_size);

				if(l == ERROR_NO_MORE_ITEMS)
					break;

				if(stricmp(szName, "Email") == 0)
				{
					DWORD new_value = 1;
					// Write to registry
					TCHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Email");
					TCHAR szName[] = TEXT("Email");
					l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)email,sizeof(email));
					count++;
					if(count == 2)
						break;
				}
				if(stricmp(szName, "UserName") == 0)
				{
					DWORD new_value = 1;
					// Write to registry
					TCHAR reg_sub_key[] = TEXT("Software\\Kazaa\\UserDetails");
					TCHAR szName[] = TEXT("UserName");
					l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)username,sizeof(username));
					count++;
					if(count == 2)
						break;
				}
				i++;
			}
		}
		RegCloseKey(hkey);
	}
}

//
//
//
void KazaaLauncherDll::ReadInUsernames()
{
	CStdioFile file;
	CString filestring;
	char tempstring[256+1];
	if(file.Open("c:\\syncher\\rcv\\Kazaa-Launcher\\namelist.txt", CFile::modeRead|CFile::typeText) != 0)
	{
		v_usernames.clear();
		LPTSTR data = file.ReadString(tempstring, 256);
		while (data!= NULL)
		{
			filestring = tempstring;
			filestring.TrimRight();
			filestring.TrimLeft();
			v_usernames.push_back(filestring);
			data = file.ReadString(tempstring, 256);
		}
	}
	else
	{
//		MessageBox(NULL, "Coudn't find the file \"namelist.txt\"", "ERROR", MB_OK|MB_ICONERROR);
		return;
	}
	file.Close();
}

//
//
//
void KazaaLauncherDll::ChangeUsername(char * username)
{
	CString string = username;
	char tempuser[32+1];
	int len;
	int num;
	int randnum;
	char numchar[1+1];

	//Remove all the numbers
	string.Remove('0');
	string.Remove('1');
	string.Remove('2');
	string.Remove('3');
	string.Remove('4');
	string.Remove('5');
	string.Remove('6');
	string.Remove('7');
	string.Remove('8');
	string.Remove('9');

	len = string.GetLength();
	num = (len%10);
	randnum = rand()%4;

	strcpy(tempuser,string);
	_itoa(num,numchar,10);
	strcat(tempuser,numchar);
	for (int i = 0;i<randnum;i++)
	{
		num = rand()%10;
		_itoa(num,numchar,10);
		strcat(tempuser,numchar);
	}
	
	strcpy(username,tempuser);
}

//
//
//
void KazaaLauncherDll::CheckNumberOfSupernodes()
{
	if(m_kazaa_instances >= (int)v_kazaa_processes.size())
	{
/*		int additional_supernodes_required = m_kazaa_instances - (int)v_kazaa_processes.size() - (int)v_available_info.size();
		if(additional_supernodes_required > 0)
		{
			if(m_controller_source.length() > 0)
			{
				char dest[256];
				strcpy(dest, m_controller_source.c_str());
				SendSupernodesRequest(dest, additional_supernodes_required);
			}
		}
*/
	}
	else
	//we are running too many kazaa
	{
		vector<IPAndPort> remove_ip;
		UINT num_to_kill = (UINT)v_kazaa_processes.size() - m_kazaa_instances;
		//UINT index = (UINT)v_kazaa_processes.size() - 1;
		for(UINT i=0; i<num_to_kill; i++)
		{
			IPAndPort ip;
			if(v_kazaa_processes.back().m_connected_supernode_ip.length()!=0)
			{
				ip.m_ip = GetIntIPFromStringIP(v_kazaa_processes.back().m_connected_supernode_ip.c_str());
				ip.m_port = v_kazaa_processes.back().m_connected_supernode_port;
			}
			else
			{
				ip.m_ip = GetIntIPFromStringIP(v_kazaa_processes.back().m_connecting_supernode_ip.c_str());
				ip.m_port = v_kazaa_processes.back().m_connecting_supernode_port;
			}
			
			remove_ip.push_back(ip);
			char msg[256];
			DWORD id = v_kazaa_processes.back().m_process_id;
			sprintf(msg,"Killing Kazaa PID: %d - Running too many Kazaas", id);
			m_dlg.Log(msg);
			KillKazaa(id);
		}
		SendRemoveSupernode(remove_ip); //send the supernodes to controller that we are no longer using
	}
	//Check if we have too many unused supernodes
	int total_supernodes = (int)v_kazaa_processes.size()+(int)v_available_info.size();
	if(total_supernodes > m_kazaa_instances)
	{
		int num_to_remove = total_supernodes - m_kazaa_instances;
		for(int i=0; i<num_to_remove; i++)
		{
			if(v_available_info.size() > 0)
			{
				KazaaStatusInfo info = v_available_info.back();
				IPAndPort ip_port;
				ip_port.m_ip = info.m_ip;
				ip_port.m_port = info.m_port;
				v_available_info.pop_back();
				SendRemoveSupernode(ip_port);
			}
		}
	}
}

//
//
//
bool KazaaLauncherDll::KillKazaa(DWORD process_id)
{
	bool found = false;
	vector<KazaaProcess>::iterator iter = v_kazaa_processes.begin();
	while(iter != v_kazaa_processes.end())
	{
		if(iter->m_process_id == process_id)
		{
			found = true;
			break;
		}
		iter++;
	}
	if(found)
	{
		switch(iter->m_desktop_num)
		{
		case 0:
			{
				//Enum the default Desktop
				EnumWindows((WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);
				break;
			}
		case 1:
			{
				HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
				EnumDesktopWindows(desk1,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
				CloseDesktop(desk1);
				break;
			}
		case 2:
			{
				HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
				EnumDesktopWindows(desk2,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
				CloseDesktop(desk2);
				break;
			}
		case 3:
			{
				HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
				EnumDesktopWindows(desk3,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
				CloseDesktop(desk3);
				break;
			}
		}
		//try to remove this process from the process list
		v_kazaa_processes.erase(iter);
	}
	else // we will try to kill it for all desktop
	{
		EnumWindows((WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);
		HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		EnumDesktopWindows(desk1,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
		CloseDesktop(desk1);
		HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		EnumDesktopWindows(desk2,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
		CloseDesktop(desk2);
		HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
		EnumDesktopWindows(desk3,(WNDENUMPROC)EnumWindowProcID,(LPARAM)process_id);	
		CloseDesktop(desk3);
	}

	//remove it from the dialog
	m_dlg.RemoveKazaa(process_id);
	
	HANDLE hHandle;
	hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,process_id);
	if(hHandle != NULL)
	{
		TerminateProcess(hHandle,0);
		CloseHandle(hHandle);
	}
	
	return found;
}

//
//
//
string KazaaLauncherDll::GetIPStringFromIPInt(int ip_int)
{
	string ip;
	char ip_str[16];
	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
	ip = ip_str;
	return ip;
}

//
//
//
string KazaaLauncherDll::GetFreeRunInDir()
{
	string run_in_dir;
/*
	for(UINT i=0; i<m_kazaa_instances;i++)
	{
		run_in_dir = "C:\\OnSystems";
		char num[16];
		sprintf(num, "%u", i);
		run_in_dir += num;
		run_in_dir += "\\Kazaa\\";
		bool being_used = false;
		CFile file;
		string filename1 = run_in_dir;
		string filename2 = run_in_dir;
		filename1 += "data256.dbb";
		filename2 += "data1024.dbb";
		if(file.Open(filename1.c_str(),CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyRead)==0)
			being_used = true;
		else
			file.Close();
		if(file.Open(filename2.c_str(),CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyRead)==0)
			being_used = true;
		else
			file.Close();
		if(being_used==false)
			break;
	}
*/
	int index = 0;
	while(true)
	{
		run_in_dir = "C:\\OnSystems";
		char num[4];
		sprintf(num, "%d", index);
		run_in_dir += num;
		run_in_dir += "\\Kazaa Lite K++\\";
		bool being_used = false;

		//if(m_processes_directories.IsAlreadyExisted(run_in_dir) == false)
		
		if(IsFileWritable(run_in_dir.c_str()))
		{
			break;
		}		
		index++;
	}
	MakeSureDirectoryPathExists(run_in_dir.c_str());
	return run_in_dir;
}

//
//
//
void KazaaLauncherDll::KillAllIdleSockets(void)
{
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		if(m_sockets[i].IsSocket())
		{
			CTime current_time = CTime::GetCurrentTime();
			CTimeSpan elapse_time = current_time - m_sockets[i].m_start_time;
			LONGLONG seconds = elapse_time.GetTotalSeconds();
			if(seconds > 60)
			{
				m_sockets[i].TimeOut();
			}
		}
	}
	
}

//
//
//
long KazaaLauncherDll::GetProcessRunningTime(DWORD process_id)
{
	long running_time = 0;
	HANDLE hHandle;
	FILETIME CreationTime,ExitTime,KernelTime,UserTime;
	hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,process_id);
	if(hHandle != NULL)
	{
		BOOL ret = GetProcessTimes(hHandle,
					&CreationTime,
					&ExitTime,
					&KernelTime,
					&UserTime);
		if(ret != 0)
		{
			CTime cCreationTime(CreationTime);
			CTimeSpan ts = CTime::GetCurrentTime() - cCreationTime;
			running_time = (long)ts.GetTotalSeconds();
		}
		CloseHandle(hHandle);
	}
	return running_time;
}

//
//
//
void KazaaLauncherDll::DeleteDatabaseDirOption()
{
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	l = RegDeleteValue(hkey, "DatabaseDir");
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeDefaultShareFolder()
{
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "DownloadDir") == 0)
			{
				unsigned char path[20] = "C:\\FastTrack Shared";
				// Write to registry
				TCHAR szName[] = TEXT("DownloadDir");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, path,sizeof(path));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::KillAllKazaaProcesses()
{
	//Enum the default Desktop
	EnumWindows((WNDENUMPROC)EnumWindowProc,NULL);
	
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumWindowProc,NULL);	
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumWindowProc,NULL);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumWindowProc,NULL);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);

	v_kazaa_processes.clear(); //clear the processes vector
	v_available_info.clear(); //clear all unused supernodes

	//check to see if we have killed them all
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
#ifdef KAZAA_KPP
			if(stricmp("Kazaa.kpp",name)==0)//if process is named kazaa, add it to the kazaa process list, K++ version
#else
			if(stricmp("Kazaa.exe",name)==0)//if process is named kazaa, add it to the kazaa process list
#endif
			{
				/*
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;

				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);

				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
				*/
				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				if(hHandle != NULL)
				{
					TerminateProcess(hHandle,0);
					CloseHandle(hHandle);
				}
			}
		}
		CloseHandle(handle);
	}
}

//
//
//
void KazaaLauncherDll::StopLaunchingKazaa()
{
	m_stop_kazaa = true;
}

//
//
//
void KazaaLauncherDll::ResumeLaunchingKazaa()
{
	if(m_stop_kazaa)
	{
		if(m_controller_source.length()>0)
		{
			char* dest = new char[m_controller_source.length()+1];
			strcpy(dest,m_controller_source.c_str());
			SendSupernodesRequest(dest,m_kazaa_instances);
			delete [] dest;
		}
	}
	m_stop_kazaa = false;
}

//
//
//
void KazaaLauncherDll::ResetAllKazaaProcessesAliveFlag()
{
	for(UINT i=0;i<v_kazaa_processes.size();i++)
	{
		if(v_kazaa_processes[i].m_process_id != 0)
			v_kazaa_processes[i].m_alive=false;
		else
			v_kazaa_processes[i].m_alive=true;
	}
}

//
//
//
void KazaaLauncherDll::RemoveAllGoneProcesses()
{
	for(UINT i=0;i<v_kazaa_processes.size();i++)
	{
		if(v_kazaa_processes[i].m_alive == false && v_kazaa_processes[i].m_process_id != 0)
		{
			IPAndPort ip;
			ip.m_ip = GetIntIPFromStringIP(v_kazaa_processes[i].m_connecting_supernode_ip.c_str());
			ip.m_port = v_kazaa_processes[i].m_connecting_supernode_port;

			char msg[256];
			sprintf(msg,"Killing Kazaa PID: %d - Kazaa process gone", v_kazaa_processes[i].m_process_id);
			m_dlg.Log(msg);
			KillKazaa(v_kazaa_processes[i].m_process_id);
			//SendRequestNewSupernode(ip);
			SendRemoveSupernode(ip);
		}
	}
}

//
//
//
void KazaaLauncherDll::NewNameListArrived(const char* source)
{
	//Reload the name list
	ReadInUsernames();
}

//
//
//
//overriding the base Dll function to receive data from my other dlls
bool KazaaLauncherDll::ReceivedDllData(AppID from_app_id, void* input_data, void* output_data)
{
	KazaaControllerHeader* header = (KazaaControllerHeader*)input_data;
	byte *pData=(byte *)input_data;
	pData+=sizeof(KazaaControllerHeader);

	switch(header->op)
	{
		case KazaaControllerHeader::Request_Kazaa_Number:
		{
			//TRACE("Kazaa Launcher: Received message \"Request_Kazaa_Number\" from AppID: %d\n",from_app_id.m_app_id);
			int *kazaa_number = (int*)output_data;
			*kazaa_number = m_kazaa_instances;
			return true;
		}
		case KazaaControllerHeader::Source_Kill_All_Kazaa:
		{
			//TRACE("Kazaa Launcher: Received message \"Source_Kill_All_Kazaa\" from AppID: %d\n",from_app_id.m_app_id);
			char msg[256];
			sprintf(msg, "Received local DLL message from AppID %d, killing all kazaas",from_app_id.m_app_id);
			m_dlg.Log(msg);
			StopLaunchingKazaa();
			KillAllKazaaProcesses();
			return true;
		}
		case KazaaControllerHeader::Source_Launch_All_Kazaa:
		{
			//TRACE("Kazaa Launcher: Received message \"Source_Launch_All_Kazaa\" from AppID: %d\n",from_app_id.m_app_id);
			char msg[256];
			sprintf(msg, "Received local DLL message from AppID %d, resume launching all kazaas",from_app_id.m_app_id);
			m_dlg.Log(msg);
			ResumeLaunchingKazaa();
			return true;
		}
		case KazaaControllerHeader::Source_Number_Of_Kazaa_To_Run:
		{
			m_kazaa_instances = *((UINT*)pData);
			SaveMaxKazaaNumber();
			//Check to see if we have enough supernodes
			CheckNumberOfSupernodes();
			return true;
		}
		case KazaaControllerHeader::Kazaa_Launching_Interval:
		{
			int interval = *((int*)pData);
			m_dlg.KillTimer(3);
			m_dlg.SetTimer(3, interval*1000,NULL);
			char msg[256];
			sprintf(msg, "Received local DLL message from AppID %d, resetting launching interval to %d seconds",from_app_id.m_app_id,interval);
			m_dlg.Log(msg);
			return true;
		}
		case KazaaControllerHeader::Disable_Minimizing_Kazaa_Windows:
		{
			m_disable_minimizing_kazaa_windows = true;
			return true;
		}
		case KazaaControllerHeader::Check_Kazaa_Connection_Status:
		{
			int interval = *((int*)pData);
			m_dlg.KillTimer(2);
			m_dlg.SetTimer(2, interval*1000,NULL);
			char msg[256];
			sprintf(msg, "Received local DLL message from AppID %d, resetting checking Kazaa Connection Status interval to %d seconds",from_app_id.m_app_id,interval);
			m_dlg.Log(msg);
			return true;
		}
	}
	return false;
}

//
//
//
void KazaaLauncherDll::GetRealProcessID(KazaaProcess& process)
{
	bool found_real_id = false;
	int trial = 0;
	while(!found_real_id && trial <= 4)
	{
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++)
		{ //for each of the processes
			DWORD nmod;
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));
			if(nmod>0)
			{
				char name[100];
				GetModuleBaseName(handle,modules[0],name,99);
				if(stricmp("Kazaa.kpp",name)==0)
				{
					bool found = false;
					for(unsigned int j=0; j<v_kazaa_processes.size();j++)
					{
						if(v_kazaa_processes[j].m_process_id == ids[i])
						{
							found = true;
							break;
						}
					}
					if(!found) //this is the new process id created by K++
					{
						process.m_process_id = ids[i];
						CloseHandle(handle);
						found_real_id = true;
						char msg[128];
						sprintf(msg,"Found kazaa.kpp id in %d seconds\n",trial);
						TRACE(msg);
						trial = 10;
						break;
					}
				}
			}
			CloseHandle(handle);
		}
		trial++;
		if(trial < 11)
			Sleep(1000);
	}
}

//
//
//
void KazaaLauncherDll::SaveMaxKazaaNumber(void)
{
	CFile file;
	if(file.Open("max_kazaa.dat",CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)!=0)
	{
		file.Write(&m_kazaa_instances, sizeof(int));
		file.Close();
	}
}

//
//
//
void KazaaLauncherDll::ReadMaxKazaaNumber(void)
{
	CFile file;
	if(file.Open("max_kazaa.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)!=0)
	{
		file.Read(&m_kazaa_instances, sizeof(int));
		file.Close();
	}
	else
		m_kazaa_instances = 30; //default
}

//
//
//
void KazaaLauncherDll::MinimizeAllKazaaWindows(void)
{
	EnumWindows((WNDENUMPROC)EnumMinimizeKazaaWindow,(LPARAM)0);

	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumMinimizeKazaaWindow,NULL);	
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumMinimizeKazaaWindow,NULL);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumMinimizeKazaaWindow,NULL);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);
}

//
//
//
void KazaaLauncherDll::RestoreAllKazaaWindows(void)
{
	EnumWindows((WNDENUMPROC)EnumRestoreKazaaWindow,(LPARAM)0);

	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumRestoreKazaaWindow,NULL);	
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumRestoreKazaaWindow,NULL);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumRestoreKazaaWindow,NULL);


	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);
}

//
//
// add a share folder for kazaa sharing
void KazaaLauncherDll::AddShareFolder(const char* share_folder)
{
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;
		int max_num_dir = -1;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			char* last_dir = strstr(szName, "Dir");
			if(last_dir != NULL)
			{
				if(last_dir+3 != '\0')
				{
					last_dir += 3;
					sscanf(last_dir, "%d", &max_num_dir);
				}
			}
			i++;
		}

		max_num_dir++;
		char subkeyname[32];
		sprintf(subkeyname, "Dir%d", max_num_dir);
		unsigned char* path = new unsigned char[strlen(share_folder)+1+7];
		memset(path, 0, (int)strlen(share_folder)+1+7);
		memcpy(path, "012345:", 7);
		memcpy(&path[7], share_folder, strlen(share_folder)+1);
		//unsigned char path[20] = "C:\\FastTrack Shared";
		// Write to registry
		//TCHAR szName[] = TEXT("DownloadDir");
		l=RegSetValueEx(hkey, subkeyname, 0, REG_SZ, path,(DWORD)strlen(share_folder)+1+7);
		//l=RegSetValueEx(hKey,"Minimum",0,REG_DWORD,(unsigned char *)&min,cbData);
		delete [] path;
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ReceivedSharedFolders(vector<CString>& folders)
{
	RemoveOldSharedFoldersFromRegistry();
	for(UINT i=0; i<folders.size(); i++)
	{
		AddShareFolder(folders[i]);
	}
}

//
//
//
/*
bool KazaaLauncherDll::IsShareAlready(CString folder)
{
	bool is_share_already = false;
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		char val[256];
		memset(&val, 0 , sizeof(val));
		DWORD cbData=sizeof(val);

		DWORD index=0;
		
		while(RegEnumValue(hkey,index,szName,&cbName,NULL,&dwType,(unsigned char *)val,&cbData)==ERROR_SUCCESS)
		{
			folder.MakeLower();
			if(strlen(val)!=0)
			{
				if(strstr(folder, strlwr(val))!=NULL)
				{
					if(strstr(val,"012345")!=NULL)
					{
						is_share_already = true;
						break;
					}
				}
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			cbData=sizeof(val);
			index++;
		}

	}
	RegCloseKey(hkey);
	return is_share_already;
}
*/

//
//
//
void KazaaLauncherDll::RemoveOldSharedFoldersFromRegistry()
{
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		char val[256];
		memset(&val, 0 , sizeof(val));
		DWORD cbData=sizeof(val);

		int index=0;
		
		while(RegEnumValue(hkey,index,szName,&cbName,NULL,&dwType,(unsigned char *)val,&cbData)==ERROR_SUCCESS)
		{
			if(strlen(val)!=0)
			{
				if(strstr(szName, "Dir")!=NULL)
				{
					if(strstr(val,"012345")!=NULL)
					{
						RegDeleteValue(hkey,szName);
						index = -1;
					}
				}
			}
			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			cbData=sizeof(val);
			index++;
		}

	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeDataBaseDirOptionInRegistry(string run_in)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "DatabaseDir") == 0)
			{
				char path[256+1];
				strcpy(path, run_in.c_str());
				strcat(path, "db");
				// Write to registry
				TCHAR szName[] = TEXT("DatabaseDir");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)path,strlen(path)+1);
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeApplicationNameInRegistry(string run_in)
{
	CHAR reg_sub_key[] = TEXT("Software\\K++");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ApplicationName") == 0)
			{
				char path[256+1];
				strcpy(path, run_in.c_str());
				strcat(path, "kazaa.kpp");
				// Write to registry
				TCHAR szName[] = TEXT("ApplicationName");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)path,strlen(path)+1);
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeApplicationNameInCurrentUserRegistry(string run_in)
{
	CHAR reg_sub_key[] = TEXT("Software\\K++");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ApplicationName") == 0)
			{
				char path[256+1];
				strcpy(path, run_in.c_str());
				strcat(path, "kazaa.kpp");
				// Write to registry
				TCHAR szName[] = TEXT("ApplicationName");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)path,strlen(path)+1);
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeExeDirInRegistry(string run_in)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Cloudload");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ExeDir") == 0)
			{
				char path[256+1];
				strcpy(path, run_in.c_str());
				strcat(path, "kpp.exe");
				// Write to registry
				TCHAR szName[] = TEXT("ExeDir");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)path,strlen(path)+1);
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

/*
//
//
//
void KazaaLauncherDll::CopyKazaaFolder(CString from, CString to)
{
	DirectoryCloner cloner;
	cloner.CloneDirectory(from, to);
}
*/

//
//
//
void ChangeDataBaseDirOptionInRegistry(CString run_in)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\LocalContent");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "DatabaseDir") == 0)
			{
				char path[256+1];
				strcpy(path, run_in);
				strcat(path, "db");
				// Write to registry
				TCHAR szName[] = TEXT("DatabaseDir");
				l= RegSetValueEx(hkey, szName, 0, REG_SZ, (unsigned char*)path,strlen(path)+1);
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
UINT RegistryMonitoringThreadProc(LPVOID pParam)
{
	RegistryMonitorThreadData* thread_data = (RegistryMonitorThreadData*)pParam;

	// Create the events
	HANDLE events[RegistryMonitorThreadData::NumberOfEvents];
	UINT i;
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data->m_events[i]=events[i];
	}

	// Start the thread
	DWORD num_events=2;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired


	while(1)
	{
		CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\LocalContent");
		HKEY hkey;
		long ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_NOTIFY , &hkey);
		ret = RegNotifyChangeKeyValue(hkey,TRUE,REG_NOTIFY_CHANGE_NAME|REG_NOTIFY_CHANGE_ATTRIBUTES|REG_NOTIFY_CHANGE_LAST_SET,thread_data->m_events[1],TRUE);

		event=WaitForMultipleObjects(num_events,events,wait_all,timeout);

		// Check to see if this is the kill thread events (event 0)
		if(event==0)
		{
			ResetEvent(events[event]);
			RegCloseKey(hkey);
			break;
		}
		// something just changed the reg key
		if(event==1)
		{
			RegCloseKey(hkey);
			CSingleLock singleLock(thread_data->p_critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				ChangeDataBaseDirOptionInRegistry(thread_data->m_database_dir);
				singleLock.Unlock();
			}
		}
		
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}

	return 0;	// exit the thread
}

//
//
//
void KazaaLauncherDll::StartMonitoringRegistry(CString database_dir)
{
	
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		m_thread_data.m_database_dir = database_dir;
		singleLock.Unlock();
	}

	m_thread_data.p_critical_section = &m_critical_section;
    m_registry_monitoring_thread = AfxBeginThread(RegistryMonitoringThreadProc,(LPVOID)&m_thread_data,THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
void KazaaLauncherDll::StopMonitoringRegistryKey()
{
	if(m_thread_data.p_critical_section!=NULL)
	{
		SetEvent(m_thread_data.m_events[0]);
	}
}

//
//
//
int KazaaLauncherDll::GetFreePort(void)
{
	SOCKET hSocket;
	int ret=0;
	// Create a socket handle that is not overlapped
	hSocket=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,0);
	int err = WSAGetLastError();
	if(hSocket==INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	// bind socket to the port they entered in
	sockaddr_in addr;
	ZeroMemory(&addr,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=0;
	addr.sin_port=htons(m_next_port);

	// Bind the socket, in preparation for listening
	ret=bind(hSocket,(sockaddr *)&addr,sizeof(sockaddr_in));
	while(ret!=0)
	{
		m_next_port++;
		if(m_next_port>=65000)
			m_next_port = 1215;
		addr.sin_port=htons(m_next_port);
		ret=bind(hSocket,(sockaddr *)&addr,sizeof(sockaddr_in));
	}
	
	//kill the socket
	ret=closesocket(hSocket);
	return m_next_port;
}

//
//
//
void KazaaLauncherDll::ChangeKazaaPortInRegistry(int port)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ListenPort") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("ListenPort");
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&port,sizeof(port));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::DisablePort1214(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\K++");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "DisablePort1214Listen") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("DisablePort1214Listen");
				int val = 1;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeIgnoreAllMessageOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\InstantMessaging");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "IgnoreAll") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("IgnoreAll");
				int val = 1;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeMaxUploadOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Transfer");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ConcurrentUploads") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("ConcurrentUploads");
				int val = 0;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeSupernodeOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Advanced");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "SuperNode") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("SuperNode");
				int val = 1;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeKppForceSupernodeOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\K++");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "SupernodeForce") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("SupernodeForce");
				int val = 0;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeKppHDDScanOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\K++");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ScanForNewFilesTimeMilliSecs") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("ScanForNewFilesTimeMilliSecs");
				int val = 3600000;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void KazaaLauncherDll::ChangeNoUploadLimitWhenIdleOption(void)
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Transfer");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			//TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "NoUploadLimitWhenIdle") == 0)
			{
				// Write to registry
				TCHAR szName[] = TEXT("NoUploadLimitWhenIdle");
				int val = 1;
				l= RegSetValueEx(hkey, szName, 0, REG_DWORD, (CONST BYTE*)&val,sizeof(val));
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
/*
void KazaaLauncherDll::CheckMemoryAndWhackIt(UINT max_mem)
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	vector<DWORD> kill_kazaas;
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
#ifdef KAZAA_KPP
			if(stricmp("Kazaa.kpp",name)==0)//if process is named kazaa, add it to the kazaa process list, K++ version
#else
			if( (stricmp("Kazaa.exe",name)==0))//if process is named kazaa, add it to the kazaa process list
#endif
			{
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;
				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				// Measure the processes mem usage
				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,ids[i]);
				
				PROCESS_MEMORY_COUNTERS pmc;
				pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
				GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));
				
				CloseHandle(hHandle);
				CloseHandle(token_handle);

				if( (UINT)pmc.WorkingSetSize >= max_mem )
					kill_kazaas.push_back(ids[i]);
			}
		}
		CloseHandle(handle);
	}
	for(i=0;i<(int)kill_kazaas.size();i++)
	{
		KillKazaa(kill_kazaas[i]);
	}
}
*/
//
//
//
void KazaaLauncherDll::SendLocalDllMessage(AppID app_id, byte* data, UINT data_len, KazaaControllerHeader::op_code op_code, bool& ret)
{
	int buf_len = data_len + sizeof(KazaaControllerHeader);
	byte* buf = new byte[buf_len];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf[0];
	header->op = op_code;
	header->size = data_len;
	memcpy(&buf[sizeof(KazaaControllerHeader)], data, data_len);
	DllInterface::SendData(app_id,buf, &ret);
	delete [] buf;
}

//
//
//
int KazaaLauncherDll::GetLeastKazaaDesktopNum()
{
	//int desktop0_num_kazaa,desktop1_num_kazaa,desktop2_num_kazaa,desktop3_num_kazaa;
	//desktop0_num_kazaa=desktop1_num_kazaa=desktop2_num_kazaa=desktop3_num_kazaa=0;
	vector<int> num_kazaas;
	num_kazaas.push_back(0);
	num_kazaas.push_back(0);
	num_kazaas.push_back(0);
	num_kazaas.push_back(0);
	for(int i=0; i<(int)v_kazaa_processes.size(); i++)
	{
		switch(v_kazaa_processes[i].m_desktop_num)
		{
			case 0:
			{
				num_kazaas[0] = num_kazaas[0]+1;
				//desktop0_num_kazaa++;
				break;
			}
			case 1:
			{
				num_kazaas[1] = num_kazaas[1]+1;
				//desktop1_num_kazaa++;
				break;
			}
			case 2:
			{
				num_kazaas[2] = num_kazaas[2]+1;
				//desktop2_num_kazaa++;
				break;
			}
			case 3:
			{
				num_kazaas[3] = num_kazaas[3]+1;
				//desktop3_num_kazaa++;
				break;
			}
		}
	}
	int least_kazaa_desktop_num=0;
	int least_kazaa = num_kazaas[0];

	for(int i=1; i<num_kazaas.size();i++)
	{
		if(num_kazaas[i] < least_kazaa)
		{
			least_kazaa_desktop_num = i;
			least_kazaa = num_kazaas[i];
		}
	}

	return least_kazaa_desktop_num;
}

//
//
//
void KazaaLauncherDll::RestartDesktopControl()
{
	m_dlg.Log("Restarting DesktopControl");
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp("DesktopControl_v2.exe",name)==0)
			{
				/*
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;

				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);

				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
				*/
				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				if(hHandle != NULL)
				{
					TerminateProcess(hHandle,0);
					CloseHandle(hHandle);
				}
			}
		}
		CloseHandle(handle);
	}
	
	PROCESS_INFORMATION pi;
	STARTUPINFOA si = {sizeof si};
	si.lpDesktop = "Default";
	BOOL bret = CreateProcess(
					NULL,
					"C:\\DesktopControl_v2.exe",
					NULL,
					NULL,
					FALSE,
					NORMAL_PRIORITY_CLASS,
					NULL,
					"C:\\",
					&si,
					&pi);
}

//
//
//
void KazaaLauncherDll::GotThreadDoneMessage()
{
	m_thread_ends_at = CTime::GetCurrentTime();
	int elapse_seconds = (m_thread_ends_at - m_thread_begins_at).GetTotalSeconds();
	char msg[128];
	UINT i=0;
	if(m_get_real_process_id_thread_data.m_process_id != 0)
	{
		sprintf(msg,"found kazaa.kpp process ID in %d seconds",elapse_seconds);
		m_dlg.Log(msg);
		for(i=0;i<v_kazaa_processes.size();i++)
		{
			if(v_kazaa_processes[i].m_process_id == 0)
			{
				v_kazaa_processes[i].m_process_id = m_get_real_process_id_thread_data.m_process_id;
				//m_processes_directories.InsertRealID(m_get_real_process_id_thread_data.m_process_id);
				//Update dialog
				m_dlg.UpdateSupernodeList(v_kazaa_processes[i]);
				if(v_available_info.size() > 0)
				{
					vector<KazaaStatusInfo>::iterator v_iter = v_available_info.begin();
					v_available_info.erase(v_iter);
				}
				break;
			}
		}

	}
	else
	{
		sprintf(msg,"couldn't find the kazaa.kpp process ID in %d seconds",elapse_seconds);
		m_dlg.Log(msg);
		vector<KazaaProcess>::iterator iter = v_kazaa_processes.begin();
		while(iter != v_kazaa_processes.end())
		{
			if(iter->m_process_id == 0)
			{
				v_kazaa_processes.erase(iter);
				break;
			}
			iter++;
		}
		//m_processes_directories.RemoveDirectory(0);
		KillProcess("Kpp.exe");
		m_restart_desktop_control_threashold++;
		if(m_restart_desktop_control_threashold >= 10)
		{
			RestartDesktopControl();
			m_restart_desktop_control_threashold = 0;
		}
	}
	

	//reset kazaa launching timer
	m_dlg.KillTimer(3);
	m_dlg.SetTimer(3, 15*1000, NULL);
	m_getting_real_id = false;
}

//
//
//
void KazaaLauncherDll::KillProcess(const char* process_name)
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(strcmp(process_name,name)==0)
			{
				/*
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;

				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);

				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
				*/
				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				if(hHandle != NULL)
				{
					TerminateProcess(hHandle,0);
					CloseHandle(hHandle);
				}
				//break;  //no break, kill them all
			}
		}
		CloseHandle(handle);
	}
}

//
//
//
bool KazaaLauncherDll::IsFileWritable(CString run_in_dir)
{
	run_in_dir += "db\\";
	MakeSureDirectoryPathExists(run_in_dir);
	for(UINT i=0; i<v_directory_cached.size(); i++)
	{
		if(run_in_dir.Compare(v_directory_cached[i])==0)
			return false;
	}
	CString file1_name = run_in_dir + "data256.dbb";
	CString file2_name = run_in_dir + "data1024.dbb";
	CFileException e;
	CFile file1,file2;
	bool file1_writable = false;
	bool file2_writable = false;
	if (file1.Open( file1_name, CFile::modeRead|CFile::modeWrite|CFile::shareDenyNone, &e ) != 0)
	{
		file1.Close();
		file1_writable = true;
	}
	else
	{
		if(e.m_cause == 2) //not found
			file1_writable = true;
	}	
	if (file2.Open( file2_name, CFile::modeRead|CFile::modeWrite|CFile::shareDenyNone, &e ) != 0)
	{
		file2.Close();
		file2_writable = true;
	}
	else
	{
		if(e.m_cause == 2) //not found
			file2_writable = true;
	}
	if(file1_writable && file2_writable)
	{
		v_directory_cached.push_back(run_in_dir);
		if(v_directory_cached.size() > 20) //caching only last 20 directories
			v_directory_cached.erase(v_directory_cached.begin());
		return true;
	}
	return false;
}

//
//
//
void KazaaLauncherDll::Log(const char* log)
{
	m_dlg.Log(log);
}

//
//
//
void KazaaLauncherDll::KillAllMyDocumentaWindows()
{
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);

	EnumWindows((WNDENUMPROC)EnumMyDocumentWindow, (LPARAM)m_dlg_hwnd);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumMyDocumentWindow,(LPARAM)m_dlg_hwnd);	
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumMyDocumentWindow,(LPARAM)m_dlg_hwnd);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumMyDocumentWindow,(LPARAM)m_dlg_hwnd);
	
	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);
}