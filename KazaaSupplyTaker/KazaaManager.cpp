// KazaaManager.cpp: implementation of the KazaaManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KazaaSupplyTakerDll.h"
#include "KazaaManager.h"
#include "KazaaSupplyDlg.h"
#include "imagehlp.h"	// for MakeSureDirectoryExists()
#include "KazaaControllerHeader.h"
#include "Shlwapi.h"	// for SHCopyKey()
#include "Psapi.h"		// for EnumProcesses()

#include <algorithm>	// for sort()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KazaaManager::KazaaManager()
{
	v_kazaa.clear();
	v_temp_kazaa.clear();
	v_ptrWorkerGuys.clear();

	p_dlg = NULL;

	m_minimize = false;
	m_processing = false;
//	m_got_sd_ip = false;

	m_search_more_count = 0;
//	m_last_node_element_used = 0;

   /* Seed the random-number generator with current time so that
    * the numbers will be different every time we run.
    */
   srand( (unsigned)time( NULL ) );


}

KazaaManager::~KazaaManager()
{

}

void KazaaManager::InitParent(CKazaaSupplyDlg *parent)
{
	p_dlg = parent;
	m_dlg_hwnd = parent->GetSafeHwnd();

//	ReadInConfigFile();
}


BOOL CALLBACK KazaaManager::enumwndfn(HWND hWnd, LPARAM lParam)
{
	KazaaManager *me = (KazaaManager *)lParam;
	return me->enumwndfn(hWnd);
}

BOOL KazaaManager::enumwndfn(HWND hwnd)
{
	char name[1024+1];
	memset(name,0,sizeof(name));

	// Is this window Kazaa?
	GetClassName(hwnd, name, sizeof(name)-1);
	if(strcmp(name,"KaZaA")==0)
	{
		v_temp_kazaa.push_back(hwnd);
	}

	return true;
}

//
//
//
void KazaaManager::Log(int worker_id, HWND hwnd, char *buf)
{
	p_dlg->Log(worker_id, hwnd, buf);
}

//
//
//
void KazaaManager::Log(int worker_id, int start_ip, int end_ip, char *buf)
{
	p_dlg->Log(worker_id, start_ip, end_ip, buf);
}

//
//	Instanciate a WorkerGuy for this window ptr.  Also push it onto v_WorkerGuys()
//
void KazaaManager::CreateWorkerGuy(int id)
{
	// Instantiate a WorkerGuy object for the handle passed in
	WorkerGuy *new_WorkerGuy = new WorkerGuy;
	new_WorkerGuy->InitParent(this);
	new_WorkerGuy->SetId(id);
	v_ptrWorkerGuys.push_back(new_WorkerGuy);

}

//
//	Free's memory
//
void KazaaManager::FreeWorkerGuys()
{
	UINT num = (UINT)v_ptrWorkerGuys.size();

	for (UINT i=0; i < num; i++)
		delete v_ptrWorkerGuys[i];

	v_ptrWorkerGuys.clear();
}

/*
//
//	Returns true if we tried to start a new Kazaa... False if we found an old process ID
//
bool KazaaManager::StartNewKazaa(int id)
{
	vector<Supernode> supernodes;
	supernodes = GetNextNodeRange();

	// Kill any process which is in v_used_kza_process_ids and still exists. (because it should not, it was closed)
	if ( KillOldProcessIds() == true )
		return false;

	if (supernodes.size() != 0)
		EditRegistry(supernodes);

//	m_launcher.Launch(id);

	SetApproxNodeRange(id, supernodes[0].m_ip, supernodes[supernodes.size()-1].m_ip);

	return true;
}
*/

/*
//
//	Kill any process which is still running, but is in the used kazaa vector.  This should really
//	only happen if the process is hung or crashed or something; because we already tried killing
//	it nicely, now we must forcibly destroy it, so there are no a database conficts.
//
bool KazaaManager::KillOldProcessIds()
{
	bool found = false;

	DWORD process_array[2048];
	DWORD num_bytes_returned = 0;	// this divided by SIZEOF(DWORD) gives number of process.
	UINT num_processes = 0;

	BOOL enum_ret = EnumProcesses(process_array, sizeof(process_array), &num_bytes_returned);
	if (enum_ret == 0)
	{
		// an error occured...
		Log(NULL, NULL, " *** ");
		Log(NULL, NULL, "EnumProcesses FAILED");
		Log(NULL, NULL, " *** ");
	}
	
	num_processes = num_bytes_returned / sizeof(DWORD);

	for (UINT i=0; i < v_used_kza_process_ids.size(); i++)
	{
		// Check to see if we tried closing this guy within 2 minutes.  Perhaps the process just hasn't gone away.
		CTimeSpan ts = CTime::GetCurrentTime() - v_used_kza_process_ids[i].time_closed;
		LONGLONG mins = ts.GetTotalMinutes();
		if (mins < 2)
			continue;

		// So... we only do this if it has been more than 2 minutes since we killed the kazaa
		DWORD pid = v_used_kza_process_ids[i].pid;

		// Is this process still around?
		for (UINT j=0; j < num_processes; j++)
		{
			DWORD this_process = 0;
			this_process = process_array[j];

			if (pid == this_process)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			// Kill this process
			HANDLE process_handle = NULL;
			process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
			BOOL ret = TerminateProcess(process_handle, 0);
			
			if (ret == 0)
			{
				// The termination failed.
				DWORD error_num = GetLastError();
				char msg[1024];
				sprintf(msg, "Process id: %u FAILED TO TERMINATE, error number: %u", pid, error_num);
				Log(NULL,NULL,msg);
			}
			else
			{
				char msg[1024];
				sprintf(msg, "FORCIBLY TERMINATED PROCESS %u SUCCESFULLY", pid);
			}

		}
	}

	return found;
}
*/

/*
//
//
//
void KazaaManager::SetApproxNodeRange(int id, UINT start_ip, UINT end_ip)
{
	for (UINT i=0; i < v_ptrWorkerGuys.size(); i++)
	{
		if (v_ptrWorkerGuys[i]->m_id == id)
		{
			v_ptrWorkerGuys[i]->m_lower_ip = start_ip;
			v_ptrWorkerGuys[i]->m_upper_ip = end_ip;
			break;
		}
	}

}
*/

//
//
/*
vector<Supernode> KazaaManager::GetNextNodeRange()
{
	vector<Supernode> node_vect;
	node_vect.clear();

	if (v_supernodes.size() == 0)
		return node_vect;

	int index =	(int)(rand() % v_supernodes.size()-31);

	// Is the index out of bounds?
	if (index < 0)
		index = 0;

	// Get the next 30 supernodes
	for (int i=0; i < 30; i++)
		node_vect.push_back(v_supernodes[index + i]);

	return node_vect;
}
*/

/*
//
//
//
void KazaaManager::EditRegistry(vector<Supernode> supernodes)
{
	CHAR reg_sub_key[] = TEXT("Software");
	HKEY hkcu, hkazaa, org_key;
	DWORD disposition;


	LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub_key, 0, KEY_QUERY_VALUE|KEY_SET_VALUE , &org_key);


	l = RegOpenCurrentUser(KEY_ALL_ACCESS, &hkcu);
	l = RegCreateKeyEx(hkcu,TEXT("Software\\Kazaa"),0,NULL,REG_OPTION_NON_VOLATILE,
	KEY_ALL_ACCESS,NULL,&hkazaa,&disposition);
	l = SHCopyKey(org_key, "Kazaa", hkazaa, NULL);
//	RandomlyModifyUsername(hkcu);
	l = RegCloseKey(hkazaa);
	l = RegCloseKey(hkcu);

	unsigned char reg_key[2401];	// maximum size of the ConnectionInfo/KazaaNet[] registry key
	memset(reg_key,0,sizeof(reg_key));

	// The first element is always zero
	reg_key[0]=0x01;

	// Set the rest of the elements
	for (UINT i=0; i < supernodes.size(); i++)
	{
		unsigned int ip1,ip2,ip3,ip4;
		ip1 = (supernodes[i].m_ip >> 0)&0xFF;
		ip2 = (supernodes[i].m_ip >> 8)&0xFF;
		ip3 = (supernodes[i].m_ip >> 16)&0xFF;
		ip4 = (supernodes[i].m_ip >> 24)&0xFF;
		
		reg_key[(i*9)+1]=(unsigned char)ip4;
		reg_key[(i*9)+2]=(unsigned char)ip3;
		reg_key[(i*9)+3]=(unsigned char)ip2;
		reg_key[(i*9)+4]=(unsigned char)ip1;
		reg_key[(i*9)+5]=(unsigned char)((supernodes[i].m_port >> 0)&0xFF);
		reg_key[(i*9)+6]=(unsigned char)((supernodes[i].m_port >> 8)&0xFF);
		reg_key[(i*9)+7]=0x01;
		reg_key[(i*9)+8]=0x02;
		reg_key[(i*9)+9]=0x00;
	}

	// Write to registry
	HKEY hKey;
	char subkey[]="SOFTWARE\\KaZaA\\ConnectionInfo";
	RegCreateKeyEx(HKEY_LOCAL_MACHINE,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	DWORD cbData=sizeof(reg_key);
	RegSetValueEx(hKey,"KazaaNet",0,REG_BINARY,reg_key,cbData);
	
	RegCloseKey(hKey);
}
*/

//
//	Enumerate all kazaa window handles
//
void KazaaManager::GetKazaaList()
{
	v_temp_kazaa.clear();

	// Enumerate child windows of the desktop, (top-level windows)
	EnumWindows(enumwndfn, (LPARAM)this);

	//Switch between the other 3 desktops and enumerate all Kazaa Windows
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)enumwndfn,(LPARAM)this);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);

	// v_temp_kazaa is filled with all of the p_kazaa's.
}

//
//	Return NULL when all Kazaa's are assigned to WorkerGuys, or their pid's have been recently.
//
HWND KazaaManager::GetFreeKazaa(int id)
{
	GetKazaaList();	// refresh the Kazaa list

	// If a kazaa is free, then, it's hwnd won't match any of the worker_guys m_kza_hwnd
	for (UINT i=0; i < v_temp_kazaa.size(); i++)
	{
//		DWORD pid;
		bool found = false;

		HWND hwnd1 = v_temp_kazaa[i];

		for (UINT j=0; j < v_ptrWorkerGuys.size(); j++)
		{	
			HWND guyHWND = v_ptrWorkerGuys[j]->m_kza_hwnd;

			if ( hwnd1 == guyHWND )
			{
				found = true;
				break;
			}
		}

/*
		GetWindowThreadProcessId(hwnd1, &pid);
		// Check to see if this kazaa process id is in the USED pid vector:
		for (UINT k=0; k < v_used_kza_process_ids.size(); k++)
		{
			if (v_used_kza_process_ids[k].pid == pid)
			{
				found = true;
				break;
			}
		}

		// Also check to see if it is currently being used:
		for (UINT k=0; k < v_current_kza_process_ids.size(); k++)
		{
			if (v_current_kza_process_ids[k] == pid)
			{
				found = true;
				break;
			}
		}
*/

		if (!found)
		{
			return v_temp_kazaa[i];
		}
	}

	return NULL;
}

//
//	Loads the projects into v_projects and returns the number of projects
//
int KazaaManager::LoadProjects()
{
	// Read in all of the project files
	vector<string> project_files;
	project_files.clear();
	v_projects.clear();

	_finddata_t data;
	unsigned long hFile=(UINT)_findfirst("KazaaSupplyTaker/Projects/*.proj",&data);
	//intptr_t hFile=_findfirst("Projects/*.proj",&data);

	if(hFile!=-1)
	{
		project_files.push_back(data.name);	// the first file

		while(_findnext(hFile,&data)==0)
		{
			project_files.push_back(data.name);	// the rest of them
		}
		_findclose(hFile);
	}

	// Process the project files
	unsigned int i;
	for(i=0;i<project_files.size();i++)
	{
		CStdioFile file;
		CString filename="KazaaSupplyTaker/Projects/";
		filename+=project_files[i].c_str();
		if(file.Open(filename,CFile::typeText|CFile::shareDenyNone|CFile::modeRead)==FALSE)
		{
			char msg[1024];
			sprintf(msg,"Error opening project file %s",project_files[i].c_str());
			Log(0,NULL, msg);
		}
		else
		{
			ProjectKeywords project;
			project.m_find_more = false;	// just once, don't clear this flag once we're running...

			char buf[1024];
			while(file.ReadString(buf,sizeof(buf)))
			{
				char *ptr;
				CString cstring;

				// Check for comment character
				if(strchr(buf,'#')!=NULL)
				{
					*strchr(buf,'#')='\0';
				}
				if(strstr(buf,"<project>")!=NULL)
				{
					ptr=strstr(buf,"<project>")+strlen("<project>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
//					project.m_name=cstring;
					project.m_project_name=cstring;
				}
				else if(strstr(buf,"<query>")!=NULL)
				{
					ptr=strstr(buf,"<query>")+strlen("<query>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeUpper();
//					project.m_query=cstring;
					project.m_supply_keywords.m_search_string = cstring;
					QueryKeyword qw;
					qw.keyword = cstring;
					project.m_query_keywords.v_exact_keywords.push_back(qw);
				}
				else if(strstr(buf,"<min size>")!=NULL)
				{
					ptr=strstr(buf,"<min size>")+strlen("<min size>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
//					project.m_min_size=atoi(cstring);
					project.m_supply_keywords.m_supply_size_threshold=atoi(cstring);
				}

/*				// This is database stuff, it comes from the DC Master now
				else if(strstr(buf,"<ip>") != NULL)
				{
					ptr=strstr(buf,"<ip>")+strlen("<ip>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
					project.m_ip = cstring;
				}
				else if(strstr(buf,"<login>") != NULL)
				{
					ptr=strstr(buf,"<login>")+strlen("<login>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
					project.m_login = cstring;
				}
				else if(strstr(buf,"<pass>") != NULL)
				{
					ptr=strstr(buf,"<pass>")+strlen("<pass>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
					project.m_pass = cstring;
				}
				else if(strstr(buf,"<db>") != NULL)
				{
					ptr=strstr(buf,"<db>")+strlen("<db>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
					project.m_db = cstring;
				}
				else if(strstr(buf,"<table>") != NULL)
				{
					ptr=strstr(buf,"<table>")+strlen("<table>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
					project.m_table = cstring;
				}
*/
				// Required keywords and killwords
				else if(strstr(buf,"<kill>")!=NULL)
				{
					ptr=strstr(buf,"<kill>")+strlen("<kill>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
//					project.v_killwords.push_back((string)cstring);
					SupplyKeyword sk;

					char *str = new char[cstring.GetLength()+1];
					strcpy(str, cstring);

					sk.v_keywords.push_back(str);
					project.m_supply_keywords.v_killwords.push_back(sk);
				}
				else if(strstr(buf,"<req>")!=NULL)
				{
					ptr=strstr(buf,"<req>")+strlen("<req>");
					cstring=ptr;
					cstring.TrimLeft();
					cstring.TrimRight();
					cstring.MakeLower();
//					project.v_required_keywords.push_back((string)cstring);
					SupplyKeyword sk;

					char* str = new char[cstring.GetLength()+1];
					strcpy(str, cstring);

					sk.v_keywords.push_back(str);
					project.m_supply_keywords.v_keywords.push_back(sk);
				}

				else if(strstr(buf,"<find more>") != NULL)
				{
					project.m_find_more = true;
				}
				else if(strstr(buf,"<audio>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Audio;
					project.m_search_type=ProjectKeywords::search_type::audio;
				}
				else if(strstr(buf,"<video>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Video;
					project.m_search_type=ProjectKeywords::search_type::video;
				}
				else if(strstr(buf,"<images>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Images;
					project.m_search_type=ProjectKeywords::search_type::images;
				}
				else if(strstr(buf,"<documents>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Documents;
					project.m_search_type=ProjectKeywords::search_type::documents;
				}
				else if(strstr(buf,"<software>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Software;
					project.m_search_type=ProjectKeywords::search_type::software;
				}
				else if(strstr(buf,"<playlists>")!=NULL)
				{
//					project.m_search_type=Project::SearchType::Playlists;
					project.m_search_type=ProjectKeywords::search_type::playlists;
				}
				else if(strstr(buf,"<artist>")!=NULL)
				{
//					project.m_search_option=Project::SearchOption::Artist;
					project.m_artist_name="search more option by artist";
				}
/*
				else if(strstr(buf,"<mpaa>")!=NULL)
				{
					project.m_is_mpaa_project=true;
				}
				else if(strstr(buf,"<sony>")!=NULL)
				{
					project.m_is_sony_project=true;
				}
*/
				else 
				{
					// Comment / Blank line
				}
			}

			// If the project data structure was initialized, then add it to the vector
			if((project.m_project_name.size()>0) && (project.m_query_keywords.v_exact_keywords.size()>0))
			{
				v_project_keywords.push_back(project);
//				v_projects.push_back(project);

				char msg[1024];
				sprintf(msg,"  Project added : %s",project.m_project_name.c_str());
/*
				if(project.m_is_mpaa_project)
				{
					strcat(msg," [mpaa]");
				}
				if(project.m_is_sony_project)
				{
					strcat(msg," [sony]");
				}
*/
				Log(0,NULL, msg);
			}

			file.Close();
		}
	}

	m_current_project_index=0;

//	MakeSureDirectoryPathExists("out\\");

	return (int)v_project_keywords.size();
}

//
//	Sets the next project for the worker, and sets the m_last_project flag by
//	the return value.  Returns (project.m_name = "") if no more projects, else returns a PROJECT
//
//	If previous_project.m_project_name = "" then we should return the first good project
//
ProjectKeywords KazaaManager::GetNextProject(ProjectKeywords previous_project)
{
	UINT num_projects = (UINT)v_project_keywords.size();

	if (num_projects == 0)
		MessageBox(NULL, "No Projects", "You must add projects", MB_OK);

	CString previous_project_name = previous_project.m_project_name.c_str();
	int iter = 0;	// Start at the first project

	if (strcmp(previous_project_name, "") == 0)
	{
		// Get the first GOOD project
		while (iter != num_projects)
		{
			CString owner = v_project_keywords[iter].m_owner.c_str();
			CString proj = v_project_keywords[iter].m_project_name.c_str();
			owner.MakeLower();
			
			// Must be active and not terminated (the filter)
			if ( (strstr(owner, "terminated")==NULL) && (v_project_keywords[iter].m_project_active==true) )
			{
				return v_project_keywords[iter];
			}
			
			iter++;
		}

		MessageBox(NULL, "Did not find any active projects.", "Project Error", MB_OK);
	}

	// We are looking for the next project
	while (iter != num_projects)
	{
		// Find the last project
		CString this_proj = v_project_keywords[iter].m_project_name.c_str();

		if ( strcmp(this_proj, previous_project_name) == 0)	// find the last project
		{
			iter++;
			while (iter != num_projects)
			{
				CString owner = v_project_keywords[iter].m_owner.c_str();
				owner.MakeLower();

				// Must be active and not terminated (the filter)
				if ( (strstr(owner, "terminated")==NULL) && (v_project_keywords[iter].m_project_active==true) )
					return v_project_keywords[iter];

				iter++;
			}

			// If we get to here, then there are no more good projects, start over...
			ProjectKeywords blank_proj;
			blank_proj.m_project_name = "";
			
			return blank_proj;
		}

		iter++;
	}

	// we should never get here
	MessageBox(NULL, "Get Next Project Error", "Error", MB_OK);
	
	ProjectKeywords blank_proj;
	blank_proj.m_project_name = "";		
	return blank_proj;	// to avoid compiler warning
}

//
//	Returns the next project in the queue.  With this method all kazaas take seperate projects
//	and ideally close and restart when the project is done.
//
ProjectKeywords KazaaManager::GetNextProjectSerial()
{
	int project_loops = 0;
	bool last_project=false;

	for (UINT k=m_current_project_index; k < v_project_keywords.size(); k++)
	{
		// Last project?
		if (k+1 == v_project_keywords.size())
		{
			last_project=true;

			m_current_project_index=0;	// start over

			p_dlg->Log(0,NULL,"Last project reached, starting over...");
			TRACE("Last project reached, starting over...");

			m_current_project_index = 0; // reset
			project_loops++;
			
			for (unsigned int i=0; i < v_ptrWorkerGuys.size(); i++)
				v_ptrWorkerGuys[i]->m_last_project = true;

			if (project_loops == 2)
			{
				// We have no good projects in this project list.
				p_dlg->Log(0,NULL,"No good projects found.  Add projects.");
				break;
			}
		}
		else
		{
			last_project=false;
		}

		// This is the project filter, Uber, !Terminated, etc...
		CString owner = v_project_keywords[k].m_owner.c_str();
		CString proj = v_project_keywords[k].m_project_name.c_str();
		owner.MakeLower();

		// Must be active and not terminated
		if ( (strstr(owner, "terminated")==NULL) && (v_project_keywords[k].m_project_active==true))
		{
			if (last_project)
			{
				m_current_project_index=0;
			}
			else
			{
				m_current_project_index=k+1;
			}

			break;	// otherwise keep iterating through the for() loop until we get a "good / active" project
		}
		else if (last_project)
		{
			// This is the state in question...
			// let's try returning a blank project
			ProjectKeywords p;
			return p;
		}
	}

	ProjectKeywords proj_to_return = v_project_keywords[k];

	return proj_to_return;
}

//
//
//
void KazaaManager::CloseAllKazaas()
{
	// Close the worker guys kazaa windows
	for (UINT i=0; i < v_ptrWorkerGuys.size(); i++)
		v_ptrWorkerGuys[i]->CloseKaZaA();

	// Clean up any extra kazaas we have hanging around
	for (UINT i=0; i < v_temp_kazaa.size(); i++)
		::PostMessage(v_temp_kazaa[i], WM_COMMAND, 0x00008066,0);
}

/*
//
//
void KazaaManager::FilterRegistrySupernodeList()
{
	//
	// Read in the supernode list
	//

	char data[2401];
	DWORD cbData=sizeof(data);
	memset(data,0,sizeof(data));

	HKEY hKey;
	char subkey[]="SOFTWARE\\KaZaA\\ConnectionInfo";

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,subkey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		char szName[1024];
		DWORD cbName=sizeof(szName)/sizeof(szName[0]);
		DWORD dwType;

		DWORD index=0;
		while(RegEnumValue(hKey,index,szName,&cbName,NULL,&dwType,(unsigned char *)data,&cbData)==ERROR_SUCCESS)
		{
			// This should be the only entry
			if(strcmp(szName,"KazaaNet")==0)
			{
				break;
			}

			cbName=sizeof(szName)/sizeof(szName[0]);	// reset the size
			index++;			
		}
	}

	RegCloseKey(hKey);

	//
	// Filter supernodes
	//

	Supernode *sn=(Supernode *)&data[1];

	if (sn->m_ip != 0)	// Don't wack supernodes off of a list more than once per list update.
	{
		for(int i=0;i<200;i++)
		{
			// Check for IP with the same first digit.  If so whack it.  Also, whack 50% of them.  Also whack the first five.
			if(((((sn->m_ip)>>24) & 0xFF)==64) || ((rand()%2)==0) || (i<5))
			{
				sn->Clear();
			}
			else
			{
				// Keep it
			}

			sn++;
		}
	}

	//
	// Write out the supernode list
	//

	LONG ret=RegCreateKeyEx(HKEY_LOCAL_MACHINE,subkey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL);

	cbData=sizeof(data);
	ret=RegSetValueEx(hKey,"KazaaNet",0,REG_BINARY,(unsigned char *)data,cbData);
	
	RegCloseKey(hKey);
}
*/

/*
//
//	Get the path to kpp.exe
//
void KazaaManager::ReadInConfigFile()
{
	CStdioFile file;
	CString line;

	if (file.Open("SupplyConfig.txt", CFile::modeRead, NULL))
	{
		while (file.ReadString(line))
		{
			if (strcmp(line, "<path>") == 0)
			{
				file.ReadString(line);
//				m_launcher.m_kza_dir = line;
			}
		}

		file.Close();
	}
	else
		MessageBox(NULL, "SupplyConfig.txt open error.", "File Open Error", MB_OK);
}
*/

//
//	Supernode vector less than operator for supernodes
//
bool SupernodeLessThan(Supernode node1,Supernode node2)
{
	// Compare the IPs
	if(node1.m_ip < node2.m_ip)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
//
//	First (int) is # of supernodes, after that it is "IP-Port IP-Port" as "(int)(int)" pairs
//
void KazaaManager::SetSuperNodeList(char *data, int data_length)
{
	p_dlg->manager.m_com.m_reading_new_projects = true;	// we're not actually reading projects, but we are resetting the supernodes
														// so setting this flag has the same effect, we don't want to trigger the
														// workers while the projects list or the supernodes list are changing.

	v_supernodes.clear();

	int m_num_supernodes = *(int*)data;

	for (int i=0; i < m_num_supernodes; i++)
	{
		Supernode node;

		data += sizeof(int);
		node.m_ip = *(int*)data;
		data += sizeof(int);
		node.m_port = *(int*)data;

		v_supernodes.push_back(node);
	}

	char msg[1024];
	sprintf(msg, "Sorting %u supernodes...", m_num_supernodes);
	Log(0, NULL, msg);
	
	// Sort the vector here
	sort(v_supernodes.begin(),v_supernodes.end(),SupernodeLessThan);

	Log(0, NULL, "Sorting finished.");

	// ***
	//	Auto Start
	//	Once we get the supernode list we can begin.
	if (m_got_sd_ip == false)
	{
		m_got_sd_ip = true;

		if ( (p_dlg->manager.v_project_keywords.size()>0) && !p_dlg->m_running && !p_dlg->m_stop_pressed)
			p_dlg->OnGo();
	}

	p_dlg->manager.m_com.m_reading_new_projects = false;
}
*/

/*
//
//	Retrive the current list of all known supernodes.
//
//	The first integer in the buffer is the number of supernodes
//		in the list, the IP/Port pairs of integers follow.
//
void KazaaManager::RequestSuperNodeList()
{
	// Now we've got the supernode distributers IP, but we don't want to set the flag until after OnGo() is called...

	KazaaControllerHeader message;

	message.op = KazaaControllerHeader::Request_All_Supernodes;
	message.size = 0;

	char *ip = new char[256];
	strcpy(ip,m_supernode_distributer_ip);

	m_supernode_com.SendReliableData(ip, &message, sizeof(message));
	
	delete ip;
}
*/