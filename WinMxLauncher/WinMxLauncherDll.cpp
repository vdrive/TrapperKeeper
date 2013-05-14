#include "StdAfx.h"
#include "winmxlauncherdll.h"
#include "WinMxLauncherHeader.h"
#include <Psapi.h>

WinMxLauncherDll::WinMxLauncherDll(void)
{
}

//
//
//
WinMxLauncherDll::~WinMxLauncherDll(void)
{
}

//
//
//
void WinMxLauncherDll::DllInitialize()
{
}

//
//
//
void WinMxLauncherDll::DllUnInitialize()
{
}

//
//
//
void WinMxLauncherDll::DllStart()
{
	m_dlg.Create(IDD_WinMx_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);

	m_run_winmx = true;
	m_launching_interval=10;
	m_max_num_winmx = 50;
	m_winmx_path = "c:\\program files\\winmx\\winmx.exe";

	ReadInConfig();
	m_memory_scanner.StartThread();
	m_dlg.SetTimer(1,m_launching_interval*1000,0);

}

//
//
//
void WinMxLauncherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void WinMxLauncherDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:
		{
			LaunchWinmx();
			break;
		}
	}
}

//
//
//
void WinMxLauncherDll::ReadInConfig(void)
{
	CStdioFile file;
	if(file.Open("winmx.ini",CFile::modeRead|CFile::typeText)!= 0)
	{
		char config_data[4000];
		char * temp;
		LPTSTR more_data = file.ReadString(config_data, 256);

		while(more_data!=NULL)
		{
			config_data_type entry_type = GetConfigEntryType(config_data);
			char* entry_string = strchr(config_data, '>');
			entry_string++;
			temp = strchr(config_data, '\n');
			if(temp!=NULL)
			{
				temp[0] = '\0';
			}
			switch(entry_type)
			{
				case(Winmx_Path):
				{
					m_winmx_path = entry_string;
					break;
				}
				case(Num_Instances):
				{
					sscanf(entry_string, "%d", &m_max_num_winmx);
					//UpdateData(FALSE);
					break;
				}
				case(Launching_Interval):
				{
					sscanf(entry_string,"%d",&m_launching_interval);
					break;
				}
			}
			more_data = file.ReadString(config_data, 256);
		}
		file.Close();
	}
}

//
//
//
void WinMxLauncherDll::WriteConfig(void)
{
	CStdioFile file;
	if(file.Open("winmx.ini",CFile::modeWrite|CFile::modeCreate|CFile::typeText)!= 0)
	{
		file.WriteString("<winmx-path>");
		file.WriteString(m_winmx_path);
		file.WriteString("\n");
		file.WriteString("<num-of-instances>");
    	char num_str[8];
		memset(&num_str, 0, sizeof(num_str));
		CString cs_num;
		_itoa(m_max_num_winmx, num_str,10);
		cs_num = num_str;
		file.WriteString(cs_num);
		file.WriteString("\n");
		file.WriteString("<launching-interval>");
		memset(&num_str, 0, sizeof(num_str));
		_itoa(m_launching_interval, num_str,10);
		cs_num = num_str;
		file.WriteString(cs_num);
		file.WriteString("\n");	
		file.Close();
	}
}

//
//
//
WinMxLauncherDll::config_data_type WinMxLauncherDll::GetConfigEntryType(char * entry_string)
{
	if(strlen(entry_string) != 0)
	{
		if(entry_string[0] == '#')
			return Unknown;
	}
	if(strstr(entry_string, "<winmx-path>")!=NULL)
	{
		return Winmx_Path;
	}
	if(strstr(entry_string, "<num-of-instances>")!=NULL)
	{
		return Num_Instances;
	}
	if(strstr(entry_string, "<launching-interval>")!=NULL)
	{
		return Launching_Interval;
	}	
	return Unknown;
}

//
//
//
void WinMxLauncherDll::LaunchWinmx(void)
{
	if(m_run_winmx)
	{
		if(m_max_num_winmx > GetNumberOfWinmxRunning())
		{
			PROCESS_INFORMATION pi;
			STARTUPINFOA si = {sizeof si};
			char winmx_path[256+1];
			strcpy(winmx_path, m_winmx_path);
			char* path = strrchr(winmx_path, '\\');
			path++;
			path[0]='\0';
			//char command[256];
			//sprintf(command, "\"%s\" -m",m_winmx_path);
			CreateProcess(
						m_winmx_path,
						NULL,
						NULL,
						NULL,
						FALSE,
						BELOW_NORMAL_PRIORITY_CLASS,
						NULL,
						winmx_path,
						&si,
						&pi);
		}
	}
}

//
//
//
int WinMxLauncherDll::GetNumberOfWinmxRunning(void)
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	int winmx_count = 0;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));

		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp("WinMx.exe",name)==0)
				winmx_count++;
		}
	}
	return winmx_count;
}

//
//
//
//overriding the base Dll function to receive data from my other dlls
bool WinMxLauncherDll::ReceivedDllData(AppID from_app_id, void* input_data, void* output_data)
{
	WinMxLauncherHeader* header = (WinMxLauncherHeader*)input_data;
	byte *pData=(byte *)input_data;
	pData+=sizeof(WinMxLauncherHeader);

	switch(header->op)
	{
		case WinMxLauncherHeader::Request_WinMx_Number:
		{
			int *number = (int*)output_data;
			*number = GetNumberOfWinmxRunning();
			return true;
		}
		case WinMxLauncherHeader::Restart_All_WinMx:
		{
			m_run_winmx = true;
			KillAllWinMx();
			return true;
		}
		case WinMxLauncherHeader::Set_Number_Of_WinMx_To_Run:
		{
			m_max_num_winmx = *((UINT*)pData);
			WriteConfig();
			CheckWinMxNum();
			return true;
		}
		case WinMxLauncherHeader::Stop_Running_WinMx:
		{
			m_run_winmx = false;
			KillAllWinMx();
			return true;
		}
	}
	return false;
}

//
//
//
void WinMxLauncherDll::CheckWinMxNum()
{
	int running = GetNumberOfWinmxRunning();
	if( running > m_max_num_winmx)
		KillWinMx(running-m_max_num_winmx);
}

//
//
//
void WinMxLauncherDll::KillWinMx(int num_to_kill)
{
	int num_killed = 0;
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	int winmx_count = 0;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));

		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp("WinMx.exe",name)==0)
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

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
				num_killed++;
				if(num_killed >= num_to_kill)
					break;
			}
		}
	}
}

//
//
//
void WinMxLauncherDll::KillAllWinMx()
{
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	int winmx_count = 0;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));

		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp("WinMx.exe",name)==0)
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

				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ids[i]);
				TerminateProcess(hHandle,0);
				CloseHandle(hHandle);
			}
		}
	}
}