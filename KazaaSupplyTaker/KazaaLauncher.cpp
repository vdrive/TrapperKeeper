#include "StdAfx.h"
#include "kazaalauncher.h"
//#include <Psapi.h>
#include <atlbase.h>

KazaaLauncher::KazaaLauncher(void)
{

}

KazaaLauncher::~KazaaLauncher(void)
{
}

BOOL CALLBACK FindKazaaProc(
  HWND hwnd,      // handle to parent window
  LPARAM lParam   // lParam should be the address of a DWORD variable that initialy holds the processID that we are search for
){
	
	char buf[100];
	GetClassName(hwnd,buf,100);

	if(stricmp(buf,"Kazaa")==0){
		//we have found a kazaa window.
		DWORD pid=0;
		GetWindowThreadProcessId(hwnd,&pid);
		DWORD val=*(DWORD*)lParam;
		if(val==pid){  //we have found our target process ID
			TRACE("FindKazaaProc() : The correct window for the new process has been found, returning its HWND handle\n");
			*(DWORD*)lParam=(DWORD)hwnd;  //set the value that lParam points to hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

//shareDir = Directory that files are shared from and downloaded to
//databaseDir = Directory that contains the database files
//launchDir = Directory that the exe resides in
void KazaaLauncher::Launch(int _directoryIndex)
{
	int userNum=_directoryIndex;  //a variable to use when assigning names to each kazaa

	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo,sizeof(processInfo));
	ZeroMemory(&startInfo,sizeof(startInfo));
	startInfo.lpTitle="Kazaa";
	startInfo.wShowWindow=SW_SHOWNORMAL;
	
	CString username;
	username.Format("defuser%d",userNum);  //specify the user name for this kazaa
	TRACE("Starting Kazaa with UserID of %s\n",username);
	CRegKey userNameKey;
	userNameKey.Open(HKEY_CURRENT_USER,"Software");
	userNameKey.Open(userNameKey.m_hKey,"Kazaa");
	userNameKey.Open(userNameKey.m_hKey,"UserDetails");
	userNameKey.SetValue(username, "UserName"); //set the corresponding user name value in the registry

	CString dir; //set this to the appropriate database directory
	dir.Format("c:\\onsystems%d",_directoryIndex);  
	CreateDirectory(dir,NULL);  //make this directory if it doesn't already exist
	dir+="\\Kazaa\\";
	CreateDirectory(dir,NULL);

	CRegKey databasekey;
	databasekey.Open(HKEY_LOCAL_MACHINE,"SOFTWARE");
	databasekey.Open(databasekey.m_hKey,"KaZaA");
	databasekey.Open(databasekey.m_hKey,"LocalContent");
	databasekey.SetValue(dir, "DatabaseDir");


	//set the registry keys to open the correct kazaas.
	CString shareDir;
	shareDir.Format("c:\\FastTrack Shared%d",_directoryIndex);//set the shared directory for this instance here
//	CreateDirectory(shareDir,NULL);  //make this directory if it doesn't already exist

	//HKEY_CURRENT_USER\Software\Kazaa\Transfer
	CRegKey shareDirectoryKey1;
	shareDirectoryKey1.Open(HKEY_CURRENT_USER,"Software");
	shareDirectoryKey1.Open(shareDirectoryKey1.m_hKey,"Kazaa");
	shareDirectoryKey1.Open(shareDirectoryKey1.m_hKey,"Transfer");
	shareDirectoryKey1.SetValue(shareDir, "DlDir0");

	//HKEY_LOCAL_MACHINE\SOFTWARE\KaZaA\CloudLoad
	CRegKey shareDirectoryKey2;
	shareDirectoryKey2.Open(HKEY_LOCAL_MACHINE,"SOFTWARE");
	shareDirectoryKey2.Open(shareDirectoryKey2.m_hKey,"KaZaA");
	shareDirectoryKey2.Open(shareDirectoryKey2.m_hKey,"CloudLoad");
	shareDirectoryKey2.SetValue(shareDir, "ShareDir");

	//HKEY_LOCAL_MACHINE\SOFTWARE\KaZaA\LocalContent
	CRegKey shareDirectoryKey3;
	shareDirectoryKey3.Open(HKEY_LOCAL_MACHINE,"SOFTWARE");
	shareDirectoryKey3.Open(shareDirectoryKey3.m_hKey,"KaZaA");
	shareDirectoryKey3.Open(shareDirectoryKey3.m_hKey,"LocalContent");
	shareDirectoryKey3.SetValue(shareDir, "DownloadDir");

	//HKEY_CURRENT_USER\Software\Kazaa\LocalContent
	CRegKey shareDirectoryKey4;
	shareDirectoryKey4.Open(HKEY_CURRENT_USER,"Software");
	shareDirectoryKey4.Open(shareDirectoryKey4.m_hKey,"Kazaa");
	shareDirectoryKey4.Open(shareDirectoryKey4.m_hKey,"LocalContent");
	shareDirectoryKey4.SetValue(shareDir, "DownloadDir");

	//launch the process

	CString launch_exe = m_kza_dir;
	launch_exe += "kpp.exe";

	int stat=CreateProcess(launch_exe,NULL,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,m_kza_dir,&startInfo,&processInfo);
	int error=-1;
	if(!stat){
		error=GetLastError();
		char temp[6];
		MessageBox(NULL, itoa(error, temp, 10), "Launcher Error", MB_OK);
	}


//	Sleep(3000);  //allow it to run for a few seconds before we attemp to capture its process info.

	//now we have to search for the window
//	DWORD id=(DWORD)processInfo.dwProcessId;
//	DWORD oldID=id;
//	EnumWindows(FindKazaaProc,(LPARAM)&id);  //id will be set to the right HWND handle upon return

	//if(id!=oldID)
	//	return (HWND)id;

	return;  //window not found.  this is bad.
}
