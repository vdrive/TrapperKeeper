#include "StdAfx.h"
#include "isoswarmer.h"
#include <mmsystem.h>
// Ty's addition to eliminate the maps file

//The constructor does the iso swarmy stuff.  It is only done at construction time for this object.
IsoSwarmer::IsoSwarmer(void)
{
	CreateDirectory("c:\\FastTrack Shared",NULL);
	CreateDirectory("c:\\syncher",NULL);
	CreateDirectory("c:\\syncher\\rcv",NULL);
	CreateDirectory("c:\\syncher\\rcv\\swarmer.new",NULL);
	CreateDirectory("c:\\syncher\\rcv\\swarmer.distribute",NULL);

	mv_process.clear();

	//wipe the directories
	CleanDirectory("c:\\fasttrack shared\\swarmer.new");
	CleanDirectory("c:\\fasttrack shared\\swarmer.distribute");

	//create them to make sure they exist
	CreateDirectory("c:\\FastTrack Shared\\Swarmer.New",NULL);
	CreateDirectory("c:\\FastTrack Shared\\Swarmer.distribute",NULL);

	//prepare the files
	Mirror("c:\\syncher\\rcv\\swarmer.new\\","c:\\fasttrack shared\\swarmer.new\\");
	Mirror("c:\\syncher\\rcv\\swarmer.distribute\\","c:\\fasttrack shared\\swarmer.distribute\\");
	ScanInExtendedFileNames();
	srand(timeGetTime());
	
}

void IsoSwarmer::Mirror(CString syncher_path , CString swarmer_path)
{
	CString syncher_search=syncher_path+"*";
	CString swarmer_search=swarmer_path+"*";

	vector <string> v_syncher_files;
	vector <string> v_swarmer_files;

	vector <string> v_syncher_directories;
	vector <string> v_swarmer_directories;
	WIN32_FIND_DATA info;
	HANDLE hFind;
	hFind=FindFirstFile(syncher_search,&info);
	if (hFind != INVALID_HANDLE_VALUE) {
		while(FindNextFile(hFind,&info)){ //add all the rest
			if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
				continue;

			CString full_name=syncher_path+info.cFileName;
			if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
				v_syncher_directories.push_back(string(info.cFileName));
				CString swarmer_sub=swarmer_path+info.cFileName;
				swarmer_sub+="\\";
				CString syncher_sub=full_name+"\\";
				Mirror(syncher_sub,swarmer_sub);
				continue;
			}

			v_syncher_files.push_back(string(info.cFileName));
		}
		FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	}	

	/*
	hFind=FindFirstFile(swarmer_search,&info);
	if (hFind != INVALID_HANDLE_VALUE) {
		while(FindNextFile(hFind,&info)){ //add all the rest
			if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
				continue;

			CString full_name=swarmer_path+info.cFileName;
			if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
				v_swarmer_directories.push_back(string(info.cFileName));
				continue;
			}

			v_swarmer_files.push_back(string(info.cFileName));
		}
		FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	}	
	*/

	//clean out files that don't belong in the "c:\\fasttrack shared\\swarmer.x" directory

	/*
	static CString base_str="deflated_";

	//delete any obsolete files
	for(UINT i=0;i<v_swarmer_files.size();i++){
		CString target=base_str+v_swarmer_files[i].c_str();
		
		bool b_found=false;
		for(UINT j=0;j<v_syncher_files.size();j++){
			CString tmp=v_syncher_files[j].c_str();
			if(tmp.CompareNoCase(target)==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			CString tmp=swarmer_path;
			tmp+=v_swarmer_files[i].c_str();
			DeleteFile((LPCSTR)tmp);
			v_swarmer_files.erase(v_swarmer_files.begin()+i);
			i--;	
		}
	}

	//delete any obsolete directories
	for(UINT i=0;i<v_swarmer_directories.size();i++){
		bool b_found=false;
		for(UINT j=0;j<v_syncher_directories.size();j++){  //is this same directory in the syncher\\rcv part?
			if(stricmp(v_syncher_directories[j].c_str(),v_swarmer_directories[i].c_str())==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			CString tmp=swarmer_path+v_swarmer_directories[i].c_str();
			CleanDirectory(tmp);
			v_swarmer_directories.erase(v_swarmer_directories.begin()+i);
			i--;	
		}
	}
	*/


	//go through each of the syncher files and mark for processing anyone that isn't in the fasttrack shared directory.
	for(UINT i=0;i<v_syncher_files.size();i++){
		CString target=v_syncher_files[i].c_str();
		
		bool b_found=false;
		
		//for(UINT j=0;j<v_swarmer_files.size();j++){
		//	CString tmp=base_str+v_swarmer_files[j].c_str();
		//	if(tmp.CompareNoCase(target)==0){
		//		b_found=true;
		//		break;
		//	}
		//}
		if(!b_found){
			CString total_path=syncher_path;
			total_path+=v_syncher_files[i].c_str();
			mv_process.push_back(string((LPCSTR)total_path));
		}
	}
	
}

IsoSwarmer::~IsoSwarmer(void)
{
}

//call this over and over until it returns NULL
byte* IsoSwarmer::GetNextFile(UINT& data_length, CString& file_name)
{
	if(mv_process.size()==0)
		return NULL;

	
	file_name=mv_process[0].c_str();

	//Make sure a valid path exists for this file, or create file is gonna fail


	//CString tmp="c:\\syncher\\rcv\\Swarmer.New\\"+file_name;

	//open the file
	HANDLE m_file = CreateFile((LPCSTR)file_name, // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		TRACE("IsoSwarmer encountered a bad input file.  Failed Open.  filename=%s\n",(LPCSTR)mv_process[0].c_str());
		mv_process.erase(mv_process.begin()+0);  //bad file for some reason, this should never happen, but we can just ignore this evil file.
		return GetNextFile(data_length,file_name);
	}

	DWORD hsize=0;
	data_length=GetFileSize(m_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(data_length<300000 || data_length>600000000){ //evil file check
		TRACE("IsoSwarmer encountered a bad input file.  Bad Size.  filename=%s size=%d\n",(LPCSTR)mv_process[0].c_str(),data_length);
		mv_process.erase(mv_process.begin()+0);  
		CloseHandle(m_file);
		return GetNextFile(data_length,file_name);
	}
	byte *buffer=new byte[data_length];
	BOOL stat=ReadFile(m_file,buffer,data_length,(DWORD*)&data_length,NULL);
	CloseHandle(m_file);
	
	if(!stat){  //this would be rather wierd if this was false.  EVIL FILE.
		TRACE("IsoSwarmer encountered a bad input file.  Failed Read.  filename=%s size=%d\n",(LPCSTR)mv_process[0].c_str(),data_length);
		mv_process.erase(mv_process.begin()+0);  //forget about this evil file
		delete []buffer;
		return GetNextFile(data_length,file_name);
	}

	
	mv_process.erase(mv_process.begin()+0);  //forget about this good file
	file_name=file_name.MakeLower();
	file_name.Replace("c:\\syncher\\rcv\\","c:\\FastTrack Shared\\");
	file_name.Replace("deflated_","");

	//find last index of a '.' for the file extension
	
	if((rand()&1)!=0){ //50% chance we will append stuff from our extensions name_extensions.txt file
		if(mv_extended_file_names.size()>0){  
			int dir_left_index=file_name.ReverseFind('\\');
			if(dir_left_index!=-1){
				CString short_file_name=file_name.Mid(dir_left_index+1);
				CString directory_info=file_name.Left(dir_left_index+1);

				int left_index=short_file_name.ReverseFind('.');
				if(left_index!=-1){
					CString extension=short_file_name.Mid(left_index);
					short_file_name=short_file_name.Left(left_index);
					CString last_file_name=short_file_name;
					short_file_name+=" ";

					bool b_appended_once=false;
					while(true){
						short_file_name+=mv_extended_file_names[rand()%mv_extended_file_names.size()].c_str();
						if(short_file_name.GetLength()+extension.GetLength()<150){
							last_file_name=short_file_name;
							b_appended_once=true;
							short_file_name+=" ";
						}
						else{
							break;
						}
					}
					file_name=directory_info+last_file_name+extension;
				}
			}
		}
	}

	//create the path for the file. 
	int index=0;
	int dcount=0;
	while((index=file_name.Find("\\",index))!=-1){
		CString ltmp=file_name.Left(index);
		index++;
		dcount++;
		if(dcount==1)
			continue;
		//skip the first one, we don't want to create "c:\\" etc.
		CreateDirectory(ltmp,NULL);//we have to retardly build this path up one directory at a time.
	}

	Sleep(2);  //a small break so we don't use 100% of the available cpu during scans

	//WARNING:  MEMORY LEAK POSSIBLITY
	return buffer; //THE USER OF THIS FUNCTION IS RESPONSIBLE FOR DELETING THIS ALLOCATED MEMORY!
}

/*
//call this to write the processed file data back to file
void IsoSwarmer::OutputFileData(CString reference_file_name, byte* data, UINT length)
{
	static int base_len=(int)strlen("deflated_");
	reference_file_name.MakeLower();
	reference_file_name=reference_file_name.Mid(base_len);
	reference_file_name="c:\\FastTrack Shared\\Swarmer.New\\"+reference_file_name;
	HANDLE m_file = CreateFile((LPCSTR)reference_file_name,			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                0,							// no sharing.
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		TRACE("IsoSwarmer couldn't create a file in the fasttrack directory.  filename=%s\n",(LPCSTR)reference_file_name);
	}

	DWORD tmp;  //garbage variable to store how many bytes were actually written out.  
	WriteFile(m_file,data,length,&tmp,NULL);

	CloseHandle(m_file);
}*/

void IsoSwarmer::CleanDirectory(const char* directory_name)
{
	CString tmp=directory_name;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

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
			CleanDirectory(full_name);  //if it is a directory descend into it and clean it out.
		}

		if(b_directory){
			RemoveDirectory(full_name);
		}
		else{
			DeleteFile(full_name);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	RemoveDirectory(directory_name); //get rid of this directory
}

void IsoSwarmer::ScanInExtendedFileNames(void)
{
	mv_extended_file_names.clear();
	HANDLE file_handle = CreateFile((LPCSTR)"c:\\syncher\\rcv\\swarmer.extensions\\name_extensions.txt", // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL || data_length<1){ //evil file check
		CloseHandle(file_handle);
		return;
	}
	byte *buffer=new byte[data_length+1];
	BOOL stat=ReadFile(file_handle,buffer,data_length,(DWORD*)&data_length,NULL);
	CloseHandle(file_handle);	

	buffer[data_length]=(char)0; //make sure its terminated with a null char so we don't crash the computer

	int last_index=0;
	for(int i=0;i<(int)data_length;i++){
		if(buffer[i]=='\n'){
			buffer[i]=(char)0;
			CString big_daddy=(char*)(buffer+last_index);
			last_index=i+1;
			big_daddy.Replace("\n","");
			big_daddy.Replace("\r","");
			big_daddy.Replace("\t"," ");
			big_daddy=big_daddy.Trim();
			if(big_daddy.GetLength()>0){
				mv_extended_file_names.push_back(string((LPCSTR)big_daddy));
			}
		}
	}
	//parse out the name extensions
	

	delete []buffer;
}
