#include "StdAfx.h"
#include "executablesource.h"
#include <Psapi.h>

ExecutableSource::ExecutableSource(void)
{
}

ExecutableSource::~ExecutableSource(void)
{
}

//hack for the keeper and mysql.lib file copy, as well as copying other dlls into the trapper keeper directory.
void ExecutableSource::MapFinishedChanging(const char* source_ip){
	//enumerate the processes and terminate any RackRecoverySystem processes
	try{
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes and find what we're looking for
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++){ //for each of the processes
			DWORD nmod;
			
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);

			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));

			if(nmod>0){
				char name[200];
				GetModuleBaseName(handle,modules[0],name,199);
				
				CString tmp=name;
				tmp.MakeLower();
				if(tmp.Find("rackrecoverysystem")!=-1){  //is this a target process?
					TerminateProcess(handle,0);  //just close it, the dialog timer will reopen the latest version.
				}
			}
			CloseHandle(handle);
		}

		//copy any dlls into the trapper keeper directory
		vector <string> v_files;
		CString dir="c:\\syncher\\rcv\\Executables";
		EnumerateDllsInDirectory(dir,v_files);
		dir+="\\";
		for(int i=0;i<(int)v_files.size();i++){
			CString old_path=dir+v_files[i].c_str();
			CopyFile(old_path,v_files[i].c_str(),TRUE);
		}
	}
	catch(char* error){TRACE("CChildView::Shutdown(void)  Caught exception: %s",error);error=NULL;}//to get rid of compiler warning
}

string ExecutableSource::GetExtension(const char* file_name)
{
	int len=(int)strlen(file_name);
	string extension;
	for(int i=len-1;i>=0;i--){
		if(file_name[i]!='.'){
			string tmp;
			tmp+=file_name[i];
			tmp+=extension;
			extension=tmp;
		}
		else
			return extension;
	}
	return string("");
}

void ExecutableSource::EnumerateDllsInDirectory(const char* directory,vector <string> &v_files)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.GetLength()<3){  //rule out windows and c drive
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			//EnumerateFilesInDirectory(full_name,v_files);
		}
		else{
			string ext=GetExtension((const char*)info.cFileName);
			if(stricmp(ext.c_str(),"dll")!=0)  //must be dll
				continue;
			v_files.push_back((LPCSTR)info.cFileName);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

