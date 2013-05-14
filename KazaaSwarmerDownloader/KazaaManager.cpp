#include "StdAfx.h"
#include "kazaamanager.h"
#include "swarmerdownloader.h"

KazaaManager::KazaaManager(void)
{
}

KazaaManager::~KazaaManager(void)
{
}

void KazaaManager::InitParent(SwarmerDownloader * parent)
{
	p_parent = parent;

}

int KazaaManager::CloseandRelaunchKazaa()
{

	// Kill the old
	p_parent->DisplayInfo("Closing and Relaunching Kazaa");

	HWND kazaa_hwnd = NULL;
	kazaa_hwnd = ::FindWindow("KaZaA",NULL);
	if(kazaa_hwnd==NULL)
	{
		int err = LaunchKazaa();
		return 0;
	}
	::PostMessage(kazaa_hwnd,WM_COMMAND,0x00008066,0);

	// Start the new one
	int err = LaunchKazaa();
	/*
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo,sizeof(processInfo));
	ZeroMemory(&startInfo,sizeof(startInfo));
	startInfo.lpTitle="Kazaa";
	startInfo.wShowWindow=SW_SHOWNORMAL;
	CString kazaadir = "C:\\KazaaLite\\";
	CString kazaaexe = "C:\\KazaaLite\\kazaa.exe";

	int err=CreateProcess(kazaaexe,NULL,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,kazaadir,&startInfo,&processInfo);
	*/
	return err;

}

int KazaaManager::KillKazaa()
{
	p_parent->DisplayInfo("Killing Kazaa");
	HWND kazaa_hwnd = NULL;
	kazaa_hwnd = ::FindWindow("KaZaA",NULL);
	if(kazaa_hwnd==NULL)
	{
		return 0;
	}
	::PostMessage(kazaa_hwnd,WM_COMMAND,0x00008066,0);
	return 1;
}

int KazaaManager::LaunchKazaa()
{

	p_parent->DisplayInfo("Launching Kazaa");
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo,sizeof(processInfo));
	ZeroMemory(&startInfo,sizeof(startInfo));
	startInfo.lpTitle="Kazaa";
	startInfo.wShowWindow=SW_SHOWNORMAL;
	CString kazaadir = "C:\\KazaaLite\\";
	CString kazaaexe = "C:\\KazaaLite\\kazaa.exe";

	int err=CreateProcess(kazaaexe,NULL,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,kazaadir,&startInfo,&processInfo);

	kazaadir = "C:\\Program Files\\Kazaa Lite K++\\";
	kazaaexe = "C:\\Program Files\\Kazaa Lite K++\\klrun.exe";

	err=CreateProcess(kazaaexe,NULL,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,kazaadir,&startInfo,&processInfo);

	return err;

}

void KazaaManager::ClosePopups()
{
	HWND hwnd=FindWindow("#32770","Kazaa Lite");
	if(hwnd!=NULL)
	{
		HWND child=GetWindow(hwnd,GW_CHILD);
		if(child!=NULL)
		{
			char name[1024];
			GetClassName(child,name,sizeof(name));
			if(strcmp(name,"Button")==0)
			{
				GetWindowText(child,name,sizeof(name));
				if(strcmp(name,"OK")==0)
				{
					// Click OK
					Click(child);
				}
			}
		}
	}


}

void KazaaManager::CheckforError()
{
	HWND hwnd;
	hwnd = FindWindow("#32770","Kazaa Lite");
	if(hwnd!=NULL)
	{
		HWND child=GetWindow(hwnd,GW_CHILD);
		if(child!=NULL)
		{
			char name[1024];
			GetClassName(child,name,sizeof(name));
			if(strcmp(name,"Button")==0)
			{
				GetWindowText(child,name,sizeof(name));
				if(strcmp(name,"Don't Send")==0)
				{
					// Click OK
					Click(child);
					LaunchKazaa();
				}
			}
		}
	}
}

void KazaaManager::Click(HWND hwnd)
{

// Find the center point of the HWND to click on
	CRect rect;
	::GetWindowRect(hwnd,&rect);
	LPARAM center=MAKELPARAM(rect.Width()/2,rect.Height()/2);

	::PostMessage(hwnd,WM_LBUTTONDOWN,(WPARAM)MK_LBUTTON,(LPARAM)center);
	::PostMessage(hwnd,WM_LBUTTONUP,(WPARAM)0,(LPARAM)center);
}