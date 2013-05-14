#include "StdAfx.h"
#include "isoswarmer.h"

//The constructor does the iso swarmy stuff.  It is only done at construction time for this object.
IsoSwarmer::IsoSwarmer(void)
{
	CreateDirectory("c:\\FastTrack Shared",NULL);
	CreateDirectory("c:\\FastTrack Shared\\Swarmer.New",NULL);

	static CString syncher_path="c:\\syncher\\rcv\\swarmer.new\\";
	static CString swarmer_path="c:\\fasttrack shared\\swarmer.new\\";
	
	static CString syncher_search=syncher_path+"*";
	static CString swarmer_search=swarmer_path+"*";

	mv_process.clear();
	vector <string> v_syncher_files;
	vector <string> v_swarmer_files;
	WIN32_FIND_DATA info;
	HANDLE hFind;
	hFind=FindFirstFile(syncher_search,&info);
	if (hFind != INVALID_HANDLE_VALUE) {
		while(FindNextFile(hFind,&info)){ //add all the rest
			if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
				continue;

			CString full_name=syncher_path+info.cFileName;
			if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
				continue;
			}

			v_syncher_files.push_back(string(info.cFileName));
		}
		FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	}	

	hFind=FindFirstFile(swarmer_search,&info);
	if (hFind != INVALID_HANDLE_VALUE) {
		while(FindNextFile(hFind,&info)){ //add all the rest
			if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
				continue;

			CString full_name=swarmer_path+info.cFileName;
			if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
				continue;
			}

			v_swarmer_files.push_back(string(info.cFileName));
		}
		FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
	}	

	//clean out files that don't belong in the "c:\\fasttrack shared\\swarmer.new" directory
	static CString base_str="deflated_";

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
			CString tmp="c:\\FastTrack Shared\\Swarmer.New\\";
			tmp+=v_swarmer_files[i].c_str();
			DeleteFile(tmp);
			v_swarmer_files.erase(v_swarmer_files.begin()+i);
			i--;	
		}
	}

	//go through each of the syncher files and mark for processing anyone that isn't in the fasttrack shared directory.
	for(UINT i=0;i<v_syncher_files.size();i++){
		CString target=v_syncher_files[i].c_str();
		
		bool b_found=false;
		for(UINT j=0;j<v_swarmer_files.size();j++){
			CString tmp=base_str+v_swarmer_files[j].c_str();
			if(tmp.CompareNoCase(target)==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			mv_process.push_back(v_syncher_files[i]);
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
	CString tmp="c:\\syncher\\rcv\\Swarmer.New\\"+file_name;

	//open the file
	HANDLE m_file = CreateFile((LPCSTR)tmp, // open file at local_path 
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

	//WARNING:  MEMORY LEAK POSSIBLITY
	return buffer; //THE USER OF THIS FUNCTION IS RESPONSIBLE FOR DELETING THIS ALLOCATED MEMORY!
}

//call this to write the data back to file
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
}
