#include "StdAfx.h"
#include "deletemodule.h"
#include "MediaManager.h"
#include "fileassociation.h"
#include <io.h>     // for finddata handle
#include <direct.h> // for rmdir

DeleteModule::DeleteModule(void)
{
}

DeleteModule::~DeleteModule(void)
{
}

void DeleteModule::InitParent(MediaManager *parent)
{
	p_parent = parent;	
}

//
//
//

//Removes normal files just given a path
bool DeleteModule::DeleteNormalFile(char * path)
{
	p_parent->KillGift();
	p_parent->KillWinMx();
	if (remove(path) == 0)
	{
		return true;
	}
	return false;
}

//
//
//

//Search through the maps file and find the associated file to delete
bool DeleteModule::DeleteMappedFile(char * filename)
{
	p_parent->ReadMapsFile();
	vector<FileAssociation>::iterator temp_fa = p_parent->v_maps_list.begin();
	while (temp_fa != p_parent->v_maps_list.end());
	{
		if (strcmp(filename,  temp_fa->m_path) == 0)
		{
			if (remove(temp_fa->m_assoc_path) == 0)
			{
				p_parent->v_maps_list.erase(temp_fa);
				p_parent->RewriteMapsFile();
				return true;
			}
		}
		temp_fa++;
	}
	return false;
}

//
//
//  Delete a list of file associations 
bool DeleteModule::DeleteMappedFileList(vector <FileAssociation> vfa)
{
		char buf[1024+1];

		for (int j = 0; j < (int)vfa.size(); j ++)
		{	
			//Remove both the input and associated files to catch any templates with no association
			if (remove(vfa[j].m_assoc_path) == 0)
			{
				sprintf(buf,"Deleted File %s ",vfa[j].m_assoc_path);
				p_parent->DisplayInfo(buf);

				if (remove(vfa[j].m_path) == 0)
				{
					sprintf(buf,"Deleted File %s ",vfa[j].m_path);
					p_parent->DisplayInfo(buf);
				}
					// If we were able to remove the association
					// Take the File Association off of the current maps list
					p_parent->RemoveFileAssociationFromList(vfa[j]);
			}


			
		}
		// Fully re-write the maps file with the current maps list
		p_parent->RewriteMapsFile();
		return true;

}

void DeleteModule::GetDirectoryList(char * folder, vector<string> * dirnames)
{
	char buf[4096];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	string tempname;
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			tempname = folder;
			tempname +=data.name;
			tempname += "\\";
			dirnames->push_back(tempname);
			GetDirectoryList((char *)tempname.c_str(),dirnames);

		}
	}
	else
	{
			//if it's not a directory don't care
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				tempname = folder;
				tempname +=data.name;
				tempname += "\\";
				dirnames->push_back(tempname);
				GetDirectoryList((char *)tempname.c_str(),dirnames);
			}
		}

		else
		{
			//if it's not a directory we don't care yet
		}
	}
	_findclose(handle);


}

bool DeleteModule::DeleteExtraFolders()
{

	bool anydeleted = false;

	vector<string> FTFolders;
	vector<string> SynchFolders;

	GetDirectoryList("C:\\FastTrack Shared\\",&FTFolders);
	GetDirectoryList("C:\\syncher\\rcv\\",&SynchFolders);
	int found = 0;


	char FTFoldername[256];
	char SynchFoldername[256];
	char * ptr;

	for (int i=0; i < (int)FTFolders.size(); i++)
	{
	
		found = 0;

		// Remove the trailing \ from the directory name 
		ptr = strrchr(FTFolders[i].c_str(),'\\');
		*ptr = '\0';
		// Get the actual folder name
		ptr = strrchr(FTFolders[i].c_str(),'\\');
		strcpy(FTFoldername,ptr+1);
		for (int j=0; j < (int)SynchFolders.size(); j++)
		{
			ptr = strrchr(SynchFolders[j].c_str(),'\\');
			*ptr = '\0';
			ptr = strrchr(SynchFolders[j].c_str(),'\\');
			if (ptr != NULL)
			{
				strcpy(SynchFoldername,ptr+1);
				if (strcmp(FTFoldername,SynchFoldername) == 0)
				{
					found = 1;
					break;
				}
			}

		}

		if (found == 0)
		{

			anydeleted = ClearAndRemoveDirectory(FTFolders[i]);
		}

	}

	return anydeleted;
}

bool DeleteModule::ClearAndRemoveDirectory(string directory)
{

	bool anydeleted = false;
	char buf[4096];
	char dirbase[256];
	char tempname[256];
	strcpy(dirbase,directory.c_str());
	strcat(dirbase,"\\");
	strcpy(buf,directory.c_str());
		// Put the trailing \ back
	strcat(buf,"\\*.*");
//	string tempname;
	strcpy(tempname,dirbase);
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return false;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			
			strcat(tempname,data.name);
			anydeleted = ClearAndRemoveDirectory(tempname);

		}
	}
	else
	{
			strcpy(tempname,dirbase);
			strcat(tempname,data.name);
			DeleteNormalFile(tempname);
			anydeleted = true;
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				strcat(tempname,data.name);
				anydeleted = ClearAndRemoveDirectory(tempname);
			}
		}

		else
		{
			strcpy(tempname,dirbase);
			strcat(tempname,data.name);
			DeleteNormalFile(tempname);
			anydeleted = true;
		}
	}
	

	_findclose(handle);

	int success = _rmdir(directory.c_str());
	if (success != 0)
	{
		int crap = errno;
		DWORD damnerror = ::GetLastError();
	}

	return anydeleted;
}