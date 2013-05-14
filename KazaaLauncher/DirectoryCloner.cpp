//Author:  Ty Heath
//Date:  7.15.2003
//Purpose:  To clone directories.


#include "StdAfx.h"
#include "directorycloner.h"

DirectoryCloner::DirectoryCloner(void)
{
}

DirectoryCloner::~DirectoryCloner(void)
{
}

//Call to clone a source directory tree onto a dest path.  i.e. CloneDirectory("c:\\kazaa","c:\\onsystems5");
//This class will build the entire destination path, no call to CreateDirectory etc is needed by the user.
void DirectoryCloner::CloneDirectory(const char* source_directory, const char* dest_directory)
{
	
	CString source=source_directory;
	if(source.CompareNoCase("c:\\")==0){
		return;  //we aren't going to allow accidentle c clones.
	}
	if(source.Right(1)!='\\')
		source+="\\";

	CString dest=dest_directory;
	if(dest.Right(1)!='\\')
		dest+="\\";
	CreateDirectoryStructure((LPCSTR)dest);
	CString search_str=source+"*";
	WIN32_FIND_DATA info;
	HANDLE hFind;
	hFind=FindFirstFile(search_str,&info);
	if (hFind != INVALID_HANDLE_VALUE) {
		while(FindNextFile(hFind,&info)){ 
			if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
				continue;

			CString full_name=source+info.cFileName;
			if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){  //is this a directory?
				CString new_dir=dest+info.cFileName;  
				CreateDirectory(new_dir,NULL);
				CloneDirectory(full_name,new_dir);
			}

			CString dest_file=dest+info.cFileName;
			CopyFile(full_name,(LPCSTR)(dest_file),FALSE);
		}
		FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	}	
}

void DirectoryCloner::CreateDirectoryStructure(const char* directory)
{
	//create a directory structure
	int index=0;
	int dcount=0;
	CString tmp=directory;
	while((index=tmp.Find("\\",index))!=-1){
		CString ltmp=tmp.Left(index);
		index++;
		dcount++;
		if(dcount==1)
			continue;
		//skip the first one, we don't want to create "c:\\".
		CreateDirectory(ltmp,NULL);//we have to retardly build this path up one directory at a time.
	}
}
