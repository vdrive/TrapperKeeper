#include "StdAfx.h"
#include "inputfile.h"
#include "..\tkcom\sha1.h"

InputFile::InputFile(void)
{
}

InputFile::~InputFile(void)
{
}

bool InputFile::HasChanged(void)
{
	SHA1 sha1;
	
	char ba[33];
	ba[0]='\0';
	
	volatile int tmp=0;
	if(sha1.HashFile((char*)"c:\\syncher\\rcv\\KazaaDBManager.input\\input.dat",0,tmp)){
		sha1.Final();
		sha1.Base32Encode(ba);
	}
	string new_hash(ba);
	if(stricmp(new_hash.c_str(),m_current_hash.c_str())!=0){  //see if the hash of the input file has changed
		m_current_hash=new_hash;
		return true;
	}

	return false;
}

void InputFile::RescanInputFile(void)
{
	mv_input_projects.Clear();
	mv_input_meta_data.Clear();

	HANDLE file_handle = CreateFile((LPCSTR)"c:\\syncher\\rcv\\KazaaDBManager.input\\input.dat", // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){ //evil file check
		CloseHandle(file_handle);
		return;
	}

	vector <CString> mv_file_lines;

	byte *buffer=new byte[data_length+1];
	BOOL stat=ReadFile(file_handle,buffer,data_length,(DWORD*)&data_length,NULL);
	CloseHandle(file_handle);	

	buffer[data_length]=(char)0; //make sure its terminated with a null char so we don't crash the computer

	int last_index=0;
	for(int i=0;i<(int)data_length;i++){
		if(buffer[i]=='\n'){
			buffer[i]=(char)0;
			CString line=(char*)(buffer+last_index);
			last_index=i+1;
			line.Replace("\n","");
			line.Replace("\r","");
			line.Replace("\t","");
			int comment_index=line.Find("////",0);  //discard comments
			if(comment_index!=-1){
				line=line.Left(comment_index);
			}
			line=line.Trim();
			if(line.GetLength()>0){
				mv_file_lines.push_back(line);
			}
		}
	}
	//parse out the name extensions
	

	delete []buffer;

	//we now have our file in a vector stored line by line, with comments and blank lines discared.  We can begin parsing it
	bool b_in_project=false;
	bool b_in_metadata=false;
	bool b_in_group=false;

	int version=-1;

	InputProject *ip=NULL;
	InputGroup	 *ig=NULL;
	InputMetaData *imd=NULL;
	

	for(int i=0;i<(int)mv_file_lines.size();i++){
		CString tag,data;
		if(!ExtractLine(mv_file_lines[i],tag,data))
			continue;

		if(b_in_project){
			if(tag.CompareNoCase("/project")==0){
				if(ip->IsValid())
					mv_input_projects.Add(ip);  //put it in this vector for future reference
				else
					delete ip;  //avoid a memory leak
				ip=NULL;
				b_in_project=false;
			}
			else if(tag.CompareNoCase("name")==0){
				ip->m_name=data;
			}
			else if(tag.CompareNoCase("keywords")==0){
				if(data.GetLength()>0)
					ip->mv_keywords.push_back(string((LPCSTR)data));
			}
			else if(tag.CompareNoCase("description")==0){
				if(data.GetLength()>0)
					ip->mv_description.push_back(string((LPCSTR)data));
			}
		}
		else if(b_in_group){
			if(tag.CompareNoCase("/group")==0){
				if(ig->IsValid())
					imd->mv_groups.Add(ig);
				else
					delete ig;
				ig=NULL;
				b_in_group=false;
			}
			else if(tag.CompareNoCase("name")==0){
				ig->m_name=data;
			}
			else if(tag.CompareNoCase("dependent")==0){
				if(data.GetLength()>0){
					ig->mv_dependents.push_back(string((LPCSTR)data));
				}
			}
		}
		else if(b_in_metadata){
			if(tag.CompareNoCase("/metadata")==0){
				if(imd->IsValid())
					mv_input_meta_data.Add(imd);
				else
					delete imd;
				imd=NULL;
				
				b_in_metadata=false;
			}
			else if(tag.CompareNoCase("group")==0){
				b_in_group=true;
				ig=new InputGroup();
			}
			else if(tag.CompareNoCase("name")==0){
				imd->m_name=data;
			}
		}
		else if(tag.CompareNoCase("project")==0){
			b_in_project=true;
			ip=new InputProject();
		}
		else if(tag.CompareNoCase("metadata")==0 ){
			b_in_metadata=true;
			imd=new InputMetaData();
		}
	}

	ASSERT(ip==NULL);  //verify input file is not corrupted
	ASSERT(ig==NULL);	//verify input file is not corrupted
	ASSERT(imd==NULL);	//verify input file is not corrupted
	if(ip!=NULL){	//handle a corrupted input file gracefully
		mv_input_projects.Clear();
		mv_input_meta_data.Clear();
		delete ip;
	}
	if(ig!=NULL){	//handle a corrupted input file gracefully
		mv_input_projects.Clear();
		mv_input_meta_data.Clear();
		delete ig;
	}
	if(imd!=NULL){	//handle a corrupted input file gracefully
		mv_input_projects.Clear();
		mv_input_meta_data.Clear();
		delete imd;
	}
}

//a jackass xml parser, too much work to write a real one yo
bool InputFile::ExtractLine(CString &line,CString& tag, CString& data)
{
	int open_index=line.Find("<",0);
	int close_index=line.Find(">",0);
	if(close_index<open_index || close_index==-1 || open_index==-1)
		return false;

	tag=line.Mid(open_index+1,close_index-open_index-1);
	tag=tag.Trim();
	
	int next_open=line.Find("<",close_index+1);  //find the next open caret and extract the data inbetween
	if(next_open!=-1){
		data=line.Mid(close_index+1,next_open-close_index-1);
		data=data.Trim();
	}

	return true;
}

void InputFile::GetMetaData(const char* project, byte* description_buffer, byte* keyword_buffer)
{
	description_buffer[0]='\0';
	keyword_buffer[0]='\0';

	vector <string> v_description;
	vector <string> v_keywords;

	for(int i=0;i<(int)mv_input_projects.Size();i++){
		InputProject *ip=(InputProject*)mv_input_projects.Get(i);
		if(stricmp(ip->m_name.c_str(),project)==0){
			v_keywords=ip->mv_keywords;
			v_description=ip->mv_description;
			break;
		}
	}

	string use_description_metadata;
	string use_keyword_metadata;
	if(v_description.size()>0){  //randomly choose which metadata to use (allows for multiple sources for each client project)
		use_description_metadata=v_description[rand()%v_description.size()];
	}
	if(v_keywords.size()>0){   //randomly choose which metadata to use (allows for multiple sources for each client project)
		use_keyword_metadata=v_keywords[rand()%v_description.size()];
	}

	vector <string> v_used_groups;  //a vector that will hold groups (artists) as they become used, so we don't put duplicates into both buffers
	GenerateMetaData(use_description_metadata.c_str(),description_buffer,v_used_groups,false);
	GenerateMetaData(use_keyword_metadata.c_str(),keyword_buffer,v_used_groups,true);
}

void InputFile::GenerateMetaData(const char* name, byte* buffer, vector <string> &v_chosen_groups,bool b_test_positive)
{
	int MAX_LENGTH=50;
	buffer[0]='\0';
	CString current_buffer;
	CString saved_buffer;

	for(int i=0;i<(int)mv_input_meta_data.Size();i++){
		InputMetaData *imd=(InputMetaData*) mv_input_meta_data.Get(i);
		if(stricmp(imd->m_name.c_str(),name)==0){
			Vector v_groups;
			v_groups.Copy(&imd->mv_groups);
			while(current_buffer.GetLength()<MAX_LENGTH && v_groups.Size()>0){  //randomly choose an arist
				int group_index=rand()%v_groups.Size();
				InputGroup *ig=(InputGroup*) v_groups.Get(group_index);  

				bool b_used_already=false;
				for(int j=0;j<(int)v_chosen_groups.size();j++){
					if(stricmp(v_chosen_groups[j].c_str(),ig->m_name.c_str())==0){  //we have already used this group in the past for setting another metadata field
						v_groups.Remove(group_index);  //don't bother looking at it again
						b_used_already=true;
						break;
					}
				}

				if(b_used_already)
					continue;

				//running a test where i think each artist/single needs to be comma delimited
				//current_buffer+=ig->m_name.c_str();  //add the artist name
				//current_buffer+=" ";  
				//if(current_buffer.GetLength()>MAX_LENGTH)
				//	break;

				vector <string> v_songs;
				v_songs=ig->mv_dependents;
				int song_count=0;
				bool b_group_used_once=false;
				while(v_songs.size()>0 && song_count++<6){  //choose up to x songs from each artist, in other words we probably shouldn't let a single artist like 50 cent dominate the entire metadata field because the have 20 singles flagged
					int song_index=rand()%(int)v_songs.size();
					current_buffer+=ig->m_name.c_str();  //add the artist name here, to go with the comma delimited theory
					current_buffer+=" ";  
					current_buffer+=v_songs[song_index].c_str();
					current_buffer+=", ";
					if(current_buffer.GetLength()<=MAX_LENGTH){
						saved_buffer=current_buffer; //save our final output buffer everytime we successfully add a song and the buffer length is still valid
						b_group_used_once=true;
					}
					else break;
					v_songs.erase(v_songs.begin()+song_index);  //make sure we don't pick the same song twice
				}
				if(b_group_used_once){  //if we were able to add a song from this artist, then lets mark it as used
					v_chosen_groups.push_back(ig->m_name);
				}
				v_groups.Remove(group_index);  //make sure we don't pick the same artist twice
			}


			break;
		}
	}
	//int buf_len=saved_buffer.GetLength();

	//encode the special keyword to measure hitting
	saved_buffer="";
	if(b_test_positive){
		for(int i=0;i<10;i++){
			CString append_str;
			append_str.Format("darkface%d",i+1);
			saved_buffer+=append_str+" ";
		}
	}
	else{
		for(int i=10;i<20;i++){
			CString append_str;
			append_str.Format("darkface%d",i+1);
			saved_buffer+=append_str+" ";
		}
	}

	//trim off the last comma cus it looks stupid
	if(saved_buffer.GetLength()>1 && saved_buffer[saved_buffer.GetLength()-2]==','){
		saved_buffer=saved_buffer.Left(saved_buffer.GetLength()-2);
	}
	memcpy(buffer,saved_buffer.GetBuffer(),saved_buffer.GetLength());
	buffer[saved_buffer.GetLength()]=0;
}
