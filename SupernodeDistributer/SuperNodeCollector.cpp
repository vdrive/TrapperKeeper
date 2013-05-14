#include "StdAfx.h"
#include "supernodecollector.h"

SuperNodeCollector::SuperNodeCollector(void)
{
}

//
//
//
SuperNodeCollector::~SuperNodeCollector(void)
{
}

//
//
//
void SuperNodeCollector::ImportSupernodeList(vector<IPAddress>& list)
{
	CHAR reg_sub_key[] = TEXT("Software\\KaZaA\\ConnectionInfo");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			TCHAR temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			unsigned char* data;
			DWORD data_size = 0;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				NULL, &data_size);

			data = new unsigned char[data_size];
			::ZeroMemory(data, data_size);

			l = RegEnumValue(hkey, i, temp, &szName_size, NULL, NULL,
				data, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
			{
				delete [] data;
				break;
			}
		
			if(stricmp(szName, "KazaaNet") == 0)
			{
				int ip1,ip2,ip3,ip4,port;
				IPAddress ip;
				char tmp[5];
				for(unsigned int i=1; i<data_size;i+=12)
				{
					ip4 = (int)data[i];
					ip3 = (int)data[i+1];
					ip2 = (int)data[i+2];
					ip1 = (int)data[i+3];
					sprintf(tmp, "%x%x", data[i+5], data[i+4]);
					sscanf(tmp, "%x", &port);
					ip.m_ip = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
					ip.m_port = port;
					list.push_back(ip);
				}
				delete [] data;
				break;
			}
			i++;
			delete [] data;
		}
	}
	RegCloseKey(hkey);
	sort(list.begin(),list.end());
}

//
//
//
BOOL CALLBACK EnumWindowProc(HWND hwnd,LPARAM lParam)
{
//	vector<KazaaProcess> *processes=(vector<KazaaProcess> *)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		// Get the process id for this window	
		DWORD process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&process_id);

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
		hHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,process_id);
		
		TerminateProcess(hHandle,0);
		CloseHandle(hHandle);
		CloseHandle(token_handle);

	}
	
	return TRUE;	// continue the enumeration
}

//
//
//
void SuperNodeCollector::KillAllKazaaProcesses()
{
	EnumWindows((WNDENUMPROC)EnumWindowProc,NULL);
}

//
//
//
void SuperNodeCollector::LaunchKazaa(const char* kazaa_path, const char* run_in)
{
	char command[256];
	strcpy(command, kazaa_path);
	strcat(command, " /SYSTRAY");
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {sizeof si};
	BOOL bret;
	bret = CreateProcess(kazaa_path,command,NULL,NULL,TRUE,
		BELOW_NORMAL_PRIORITY_CLASS,NULL,run_in,&si,&pi);
	if(bret != 0)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else
		DWORD err = GetLastError();
}

//
//
//
void SuperNodeCollector::DisconnectKazaa(void)
{
	CWnd* pKazaaWnd;
	if(pKazaaWnd = CWnd::FindWindow(_T("KaZaA"), NULL))
		pKazaaWnd->PostMessage(WM_COMMAND, 0x807b, 0);
}

//
//
//
void SuperNodeCollector::ConnectKazaa(void)
{
	CWnd* pKazaaWnd;
	if(pKazaaWnd = CWnd::FindWindow(_T("KaZaA"), NULL))
		pKazaaWnd->PostMessage(WM_COMMAND, 0x807a, 0);
}

//
//
//
void SuperNodeCollector::ReconnectKazaa(void)
{
	DisconnectKazaa();
	ConnectKazaa();
}
