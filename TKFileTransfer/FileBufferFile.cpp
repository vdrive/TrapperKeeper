#include "StdAfx.h"
#include "filebufferfile.h"
#include "TKFileTransferInterface.h"
#include "..\tkcom\sha1.h"


FileBufferFile::SORTMODE FileBufferFile::sm_sort_mode=BYTIME;


FileBufferFile::FileBufferFile(const char* hash,UINT position,UINT file_length,CTime creation_time, CTime last_access_time)
{
	m_file_hash=hash;
	m_total_length=file_length;
	m_position=position;

	m_creation_time=creation_time;
	m_last_access_time=last_access_time;

	//setup completion flags
	int num_parts=m_total_length/PARTSIZE;
	if(num_parts==0 || (m_total_length%PARTSIZE)!=0){
		num_parts++;
	}
	
	for(int i=0;i<num_parts/8;i++){
		mv_completed_parts.push_back(0x00);  //push back a 0 byte indicating we haven't completed a single part.
	}

	if((num_parts%8)!=0){
		if(file_length!=0)
			mv_completed_parts.push_back(0x00);  //push back a 0 byte indicating we haven't completed a single part.
		else
			mv_completed_parts.push_back(0xff);  //push back a 0 byte indicating we haven't completed a single part.
	}

	if(file_length==0)
		mb_complete=true;
	else
		mb_complete=false;
}

FileBufferFile::~FileBufferFile(void)
{
}

bool FileBufferFile::IsComplete(bool b_initial,HANDLE buffer_file, HANDLE log_file)
{
	if(mb_complete){
		return true;
	}

	if(m_total_length==0){  //special handling for zero length files
		mb_complete=true;
		return true;
	}

	int count=m_total_length/PARTSIZE;
	if(count==0 || (m_total_length%PARTSIZE)!=0){
		count++;
	}

	for(int i=0;i<count;i++){
		byte the_byte=mv_completed_parts[i/8];
		if((the_byte & Utility::PowerOf2(i%8))==0){  //check each appropriate bit of this byte
			return false;
		}
	}

	//since very part is marked as complete, we need to do a final check to make sure that the hash that we have downloaded, matches the hash that it should be
	if(!b_initial && buffer_file!=NULL  && log_file!=NULL){
		if(!ValidateFile(buffer_file,log_file)){
			for(int i=0;i<(int)mv_completed_parts.size();i++){  //make it so we haven't received a single part of this file, since we need to start all over again
				mv_completed_parts[i]=0;  
			}
			return false;
		}
	}

	mb_complete=true;
	mv_sources.Clear();
	return true;
}


void FileBufferFile::SortByPosition(void)
{
	sm_sort_mode=FileBufferFile::BYPOSITION;
}

void FileBufferFile::SortByTime(void)
{
	sm_sort_mode=FileBufferFile::BYTIME;
}

//called to reposition a file within the global buffer
bool FileBufferFile::MoveFile(HANDLE file, UINT new_position,HANDLE log_file)
{
	CString log_str;
	log_str.Format("FileBufferFile::MoveFile() Called for file %s %d.",m_file_hash.c_str(),m_total_length);
	WriteToLogFile(log_file,log_str);
	UINT start_pos=m_position;
	DWORD num_read=0;
	byte buffer[(1<<13)];  //an 8k buffer
	int trial=0;
	while(num_read<m_total_length && trial<1000){
		DWORD tmp1=0;
		DWORD tmp2=0;

		LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
		LONG low_move=start_pos+num_read;  
		DWORD dwPtr=SetFilePointer(file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

		DWORD error;
		if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
			error=GetLastError();
			
			log_str.Format("FileBufferFile::MoveFile() ERROR moving p2p file %s, size %d, old position was %d, new position is %d.  Error was %d in SetFilePointer[0] win32 call.",m_file_hash.c_str(),m_total_length,start_pos,new_position,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		if(!ReadFile(file,buffer,min((1<<13),m_total_length-num_read),&tmp1,NULL)){
			error=GetLastError();
			
			log_str.Format("FileBufferFile::MoveFile() ERROR moving p2p file %s, size %d, old position was %d, new position is %d.  Error was %d in ReadFile win32 call.",m_file_hash.c_str(),m_total_length,start_pos,new_position,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}

		if(tmp1==0){
			trial++;
		}

		low_move=new_position+num_read;  
		dwPtr=SetFilePointer(file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

		if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
			error=GetLastError();
			
			log_str.Format("FileBufferFile::MoveFile() ERROR moving p2p file %s, size %d, old position was %d, new position is %d.  Error was %d in SetFilePointer[1] win32 call.",m_file_hash.c_str(),m_total_length,start_pos,new_position,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		num_read+=tmp1;
		if(!WriteFile(file,buffer,tmp1,&tmp2,NULL)){
			error=GetLastError();
			
			log_str.Format("FileBufferFile::MoveFile() ERROR moving p2p file %s, size %d, old position was %d, new position is %d.  Error was %d in WriteFile win32 call.",m_file_hash.c_str(),m_total_length,start_pos,new_position,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		if(tmp2!=tmp1){  //we better of written as much as we read or this operation is a failure
			return false;
		}
	}
	if(trial==1000){  //tried to do it a whole lot of times and failed
		return false;
	}

	m_position=new_position;  //remember the new position of this file in the buffer
	return true;  //we must of successfully moved the file if we got here
}

//returns true if the file is complete, false otherwise
bool FileBufferFile::DecodeCompletionFromString(string str)
{
	Utility::DecodeCompletionFromString(str,mv_completed_parts);
	return IsComplete(true,NULL,NULL);  //set the mb_complete status flag
}

string FileBufferFile::EncodeCompletionToString(void)
{
	string str;
	for(int i=0;i<(int)mv_completed_parts.size();i++){
		byte the_val=mv_completed_parts[i];
		byte the_val1=(the_val>>4)&0xf;
		byte the_val2=(the_val)&0xf;
		char ch1=Utility::ByteToChar(the_val1);
		char ch2=Utility::ByteToChar(the_val2);
		str+=ch1;
		str+=ch2;
	}
	return str;
}

int FileBufferFile::GetFilePart(HANDLE file, byte* buffer,UINT part)
{
	UINT start=part*PARTSIZE;
	if(start>m_total_length){
		return -1;
	}

	//create our status reference info
	int byte_offset=part/8;
	int bit_offset=part%8;

	byte the_byte=mv_completed_parts[byte_offset];
	int has_part=( the_byte & Utility::PowerOf2(bit_offset) );  //see if we have this part yet
	if(has_part){
		//UINT offset=part*PARTSIZE;
		UINT amount_to_read=min(PARTSIZE,m_total_length-start);  //pick whichever is smaller, the standard partsize, or whatever is left of the end of the file
		DWORD num_read;

		LONG high_move=0;
		LONG low_move=(UINT)(m_position+((UINT)PARTSIZE)*part);
		DWORD dwPtr=SetFilePointer(file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
		DWORD error;
		if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
			error=GetLastError();
			ASSERT(0);
			return false;
		}


		ReadFile(file,buffer,amount_to_read,&num_read,NULL);
		if(num_read!=amount_to_read){
			return -1;
		}
		else{
			m_last_access_time=CTime::GetCurrentTime();
			return num_read;
		}
	}
	else{
		return -1;
	}
}

//returns true if the file is now complete, or false if it still isn't complete
bool FileBufferFile::WriteFilePart(HANDLE file,UINT part, byte* buffer, UINT data_length,HANDLE log_file)
{
	CString log_str;
	UINT start=part*PARTSIZE;
	if(start>m_total_length){
		return mb_complete;
	}

	if(m_position>((1<<32)-1)-m_total_length){
		ASSERT(0);
	}
	else if(part*PARTSIZE+data_length>m_total_length){
		ASSERT(0);
	}
	//create our status reference info
	int byte_offset=part/8;
	int bit_offset=part%8;

	byte the_byte=mv_completed_parts[byte_offset];
	int has_part=( the_byte & Utility::PowerOf2(bit_offset) );  //see if we have this part yet
	if(!has_part){  //make sure we need to write 
		DWORD num_written=0;
		//set the buffer file pointer to the appropriate position in the file.
		LONG high_move=0;
		LONG low_move=(UINT)(m_position+((UINT)PARTSIZE)*part);
		DWORD dwPtr=SetFilePointer(file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
		DWORD error;
		if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
			error=GetLastError();
			
			log_str.Format("FileBufferFile::WriteFilePart() ERROR writing p2p file part for file %s, size %d, part %d, data length %d.  Error was %d in SetFilePointer win32 call.",m_file_hash.c_str(),m_total_length,part,data_length,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		if(!WriteFile(file,buffer,data_length,&num_written,NULL)){
			error=GetLastError();
	
			log_str.Format("FileBufferFile::WriteFilePart() ERROR writing p2p file part for file %s, size %d, part %d, data length %d.  Error was %d in WriteFile win32 call.",m_file_hash.c_str(),m_total_length,part,data_length,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		if(num_written==data_length){
			//encode the fact that we have now completed this part
			m_last_access_time=CTime::GetCurrentTime();
			mv_completed_parts[byte_offset] = the_byte | Utility::PowerOf2(bit_offset);
		
			log_str.Format("FileBufferFile::WriteFilePart() Finished writing file part for file %s, size %d, part %d, data length %d.",m_file_hash.c_str(),m_total_length,part,data_length);
			WriteToLogFile(log_file,log_str);
			ReceivedPart(part);

			//tmp debug code

				//vector <int> v_tmp;
				//GetIncompleteParts(v_tmp);
				//int num_parts=m_total_length/PARTSIZE;
				//num_parts++;
			//TRACE("TKFileTransfer FileBufferFile::WriteFilePart()  File %s, part %d.\n",m_file_hash.c_str(),part);
		}
		else{
			//goddamnit, this should never happen, but if it does the system will recover.
		}
	}

	return IsComplete(false,file,log_file);
}

//call to remove old sources and monitor the requesting
void FileBufferFile::Update(void)
{
	if(!mb_complete){

		//remove old sources
		for(int i=0;i<(int)mv_sources.Size();i++){
			FileSource* source=(FileSource*)mv_sources.Get(i);
			if(source->IsOld()){
				mv_sources.Remove(i);
				i--;
			}	
		}

		if(m_previous_request_timer.HasTimedOut(5)){
			if(mv_previous_requests.size()>40){  //if it gets large, lets just clear it to increase part harvest rate on large files
				mv_previous_requests.clear();
			}
			else if(mv_previous_requests.size()>0){
				mv_previous_requests.erase(mv_previous_requests.begin()+0);
			}
			m_previous_request_timer.Refresh();
		}
		//check to see enough time has passed that we can safely decrement our request_counter, to ditch old requests that were never answered
		//if(m_request_count_timer.HasTimedOut(60)){  //decrement our request count every minute automatically because we can assume someone was too busy to answer it
		//	if(m_request_count>0){
		//		m_request_count--;
		//	}
		//	m_last_part_requested=-1;
		//	m_request_count_timer.Refresh();
		//}
	}
}

//called when a source specifies how much of a file it has
void FileBufferFile::UpdateSource(const char* source_ip)
{
	for(int i=0;i<(int)mv_sources.Size();i++){
		FileSource* source=(FileSource*)mv_sources.Get(i);
		if(source->IsSource(source_ip)){
			source->Refresh();
			return;
		}
	}
	FileSource *ns=new FileSource(source_ip);
	//ns->UpdateParts(complete);
	mv_sources.Add(ns);
}

void FileBufferFile::MakePartRequest(TKComInterface* p_com, HANDLE log_file)
{
	if(mb_complete)
		return;
	//if(m_request_count>1 || mv_sources.Size()==0)  //allow up to two simultaneous part requests to be taking place.
	//	return;

	if(!m_request_count_timer.HasTimedOut(1)){
		return;
	}
	//m_request_count=0;
	m_request_count_timer.Refresh();

	CString log_str;
	//log_str.Format("FileBufferFile::MakePartRequest() BEGIN Make part request for %s.",m_file_hash.c_str());
	//WriteToLogFile(log_file,log_str);

	bool b_found_part=false;
	int attempt=0;

	int num_parts=m_total_length/PARTSIZE;
	if(num_parts==0 || (m_total_length%PARTSIZE)!=0){
		num_parts++;
	}

	//WriteToLogFile(log_file,"FileBufferFile::MakePartRequest() PHASE 1.");
	vector<int> v_needed_parts;
	GetIncompleteParts(v_needed_parts);

	//log_str.Format("FileBufferFile::MakePartRequest() BEGIN Make part request for %s.  %d parts needed.",m_file_hash.c_str(),v_needed_parts.size());
	//WriteToLogFile(log_file,log_str);

	for(int i=0;i<(int)v_needed_parts.size();i++){
		for(int j=0;j<(int)mv_previous_requests.size();j++){
			if(mv_previous_requests[j]==v_needed_parts[i]){  //have we already requested this part recently?
				v_needed_parts.erase(v_needed_parts.begin()+i);
				i--;
				break;
			}
		}
	}

	if(v_needed_parts.size()==0){  //this would be wierd as it should not be complete
		//WriteToLogFile(log_file,"FileBufferFile::MakePartRequest() END[-1] Make part request.");
		log_str.Format("FileBufferFile::MakePartRequest() END [-1] Make part request for %s, failed to find any needed parts.",m_file_hash.c_str());
		WriteToLogFile(log_file,log_str);
		//ASSERT(0);
		return;
	}

	//WriteToLogFile(log_file,"FileBufferFile::MakePartRequest() PHASE 2.");

	//make sure we don't request a part thats probably already on its way to us.
	//if(m_request_count>0){
	//	for(int i=0;i<(int)v_needed_parts.size();i++){
	//		if(v_needed_parts[i]==m_last_part_requested){
	//			v_needed_parts.erase(v_needed_parts.begin()+i);
	//			break;
	//		}
	//	}
	//}

	//if(v_needed_parts.size()==0){
	//	log_str.Format("FileBufferFile::MakePartRequest() END Make part request for %s, failed to find any needed parts we haven't already requested recently.  Last needed part was %d.",m_file_hash.c_str(),m_last_part_requested);
	//	WriteToLogFile(log_file,log_str);
	//	return;
	//}


	//Vector v_tmp;
	//v_tmp.Copy(&mv_sources);  //copy the list of sources into a temporary vector

	//our goal is to request a random part from a random source

	//query random sources for all the parts we still need
	int num_requests=0;
	while(v_needed_parts.size()>0 && num_requests++<1 && mv_sources.Size()>0){
		int get_part_index=rand()%(int)v_needed_parts.size(); 
		int get_part=v_needed_parts[get_part_index];
		mv_previous_requests.push_back(get_part);
		v_needed_parts.erase(v_needed_parts.begin()+get_part_index);  //lets not ask for this part again
		//pick a random source to ask
		int source_index=rand()%mv_sources.Size();
		FileSource *source=(FileSource *)mv_sources.Get(source_index);
	
		TKFileTransferInterface::IPInterface part_request;
		part_request.m_type=part_request.P2PREQUESTPART;
		part_request.v_ints.push_back(m_total_length);
		part_request.v_strings.push_back(m_file_hash);
		part_request.v_ints.push_back(get_part);
		byte buf[1024];
		
		int nw=part_request.WriteToBuffer(buf);
		p_com->SendReliableData((char*)source->GetIP(),buf,nw);  //let the com system worry about getting this data out to our destinations		
	}
}

//build a vector of parts that this file does not yet have
void FileBufferFile::GetIncompleteParts(vector<int> &v_parts)
{
	int num_parts=m_total_length/PARTSIZE;
	if(num_parts==0 || (m_total_length%PARTSIZE)!=0){
		num_parts++;
	}
	for(int part=0;part<num_parts;part++){
		//create our status reference info
		int byte_offset=part/8;
		int bit_offset=part%8;

		byte the_byte=mv_completed_parts[byte_offset];
		int has_part=( the_byte & Utility::PowerOf2(bit_offset) );  //see if we have this part yet
		if(!has_part){
			v_parts.push_back(part);
		}
	}
}

bool FileBufferFile::ManifestOutOfBuffer(HANDLE buffer_file,const char* full_path,HANDLE log_file)
{
	if(!mb_complete){
		TRACE("FILE TRANSFER SERVICE FileBufferFile::ManifestOutOfBuffer() failed because to mainifest file %s because the file is not yet fully downloaded\n",full_path);
		return false;
	}
	CString tmp=full_path;

	//create the path for the file
	if(tmp.Trim().GetLength()<1){  //do a few miscellaneous checks to make sure this system will remain stable.
		//TRACE("FILE TRANSFER SERVICE:  Ignoring request from plugin %d to create a file with a blank name.\n",m_from_app);
		return false;
	}

	//create the path for the file.  This is bs in my opinion, CreateFile should create the path for you.
	int index=0;
	int dcount=0;
	while((index=tmp.Find("\\",index))!=-1){
		CString ltmp=tmp.Left(index);
		index++;
		dcount++;
		if(dcount==1)
			continue;
		//skip the first one, we don't want to create "c:\\" etc.
		CreateDirectory(ltmp,NULL);//we have to retardly build this path up one directory at a time.
	}

	HANDLE file = CreateFile(full_path,			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		CString log_msg;
		int error=GetLastError();
		//log_msg.Format("FAILED:  Couldn't create the file %s to received it from %s, system error code = %d.\n",m_local_path.c_str(),m_source.c_str(),GetLastError());
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
		TRACE("FILE TRANSFER SERVICE FileBufferFile::ManifestOutOfBuffer() failed because it couldn't create the file %s.  Error code = %d\\n",full_path,error);
		return false;
	}

	UINT start_pos=m_position;
	DWORD num_read=0;
	byte buffer[(1<<13)];  //an 8k buffer

	while(num_read<m_total_length){
		DWORD tmp1=0;
		DWORD tmp2=0;

		LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
		LONG low_move=start_pos+num_read;  
		DWORD dwPtr=SetFilePointer(buffer_file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

		DWORD error=-1;
		if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
			error=GetLastError();
			CString log_str;
			log_str.Format("FileBufferFile::ManifestOutOfBuffer() ERROR writing out p2p file %s, size %d.  Error was %d in SetFilePointer win32 call.",m_file_hash.c_str(),m_total_length,error);
			WriteToLogFile(log_file,log_str);
			CloseHandle(file);
			ASSERT(0);
			return false;
		}
		if(!ReadFile(buffer_file,buffer,min((1<<13),m_total_length-num_read),&tmp1,NULL)){
			error=GetLastError();
			CString log_str;
			log_str.Format("FileBufferFile::ManifestOutOfBuffer() ERROR reading p2p file from buffer: file %s, size %d.  Error was %d in ReadFile win32 call.",m_file_hash.c_str(),m_total_length,error);
			WriteToLogFile(log_file,log_str);
			CloseHandle(file);
			ASSERT(0);
			return false;
		}
		if(tmp1==0 && m_total_length!=0){
			CloseHandle(file);
			CString log_str;
			log_str.Format("FileBufferFile::ManifestOutOfBuffer() ERROR writing out p2p file, amount read from buffer was not correct: %s, size %d, actually read %d bytes.  Error was %d in WriteFile[1] win32 call.",m_file_hash.c_str(),m_total_length,tmp1,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		
		num_read+=tmp1;
		if(!WriteFile(file,buffer,tmp1,&tmp2,NULL)){
			error=GetLastError();
			CString log_str;
			log_str.Format("FileBufferFile::ManifestOutOfBuffer() ERROR writing out p2p file, amount written was not correct: %s, size %d, actually wrote %d bytes, supposed to write %d bytes.  Error was %d in WriteFile win32 call.",m_file_hash.c_str(),m_total_length,tmp2,tmp1,error);
			WriteToLogFile(log_file,log_str);
			CloseHandle(file);
			ASSERT(0);
			return false;
		}
		if(tmp2!=tmp1){  //we better of written as much as we read or this operation is a failure
			CString log_str;
			log_str.Format("FileBufferFile::ManifestOutOfBuffer() ERROR writing out p2p file, amount written was not correct: %s, size %d, actually wrote %d bytes, supposed to write %d bytes.  Error was %d in WriteFile[1] win32 call.",m_file_hash.c_str(),m_total_length,tmp2,tmp1,error);
			WriteToLogFile(log_file,log_str);
			CloseHandle(file);
			ASSERT(0);
			return false;
		}
	}

	CloseHandle(file);
	return true;
}

bool FileBufferFile::IsBad(void)
{
	if(!mb_complete && m_creation_time<(CTime::GetCurrentTime()-CTimeSpan(2,0,0,0))){  //if a file hasn't complete after 2 days, the file probably isn't available on the network anymore
		return true;
	}
	else{
		return false;
	}
}

void FileBufferFile::WriteToLogFile(HANDLE file, const char* str)
{
	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%m/%d %I:%M %p"),str);
	DWORD tn;
	WriteFile(file,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}

string FileBufferFile::GetSources(void)
{
	string str;
	for(int i=0;i<(int)mv_sources.Size() && i<40;i++){
		FileBufferFile::FileSource *fs = (FileBufferFile::FileSource *)mv_sources.Get(i);
		if(i!=0)
			str+=", ";
		str+=fs->GetIP();
	}
	return str;
}

//call to see if we have at least one part of a file.
bool FileBufferFile::HasMinimumPartsForIndexServer(void)
{
	vector <int> v_incomplete_parts;
	GetIncompleteParts(v_incomplete_parts);

	int num_parts=m_total_length/PARTSIZE;
	if(num_parts==0 || (m_total_length%PARTSIZE)!=0){
		num_parts++;
	}
	
	float ratio=(float)v_incomplete_parts.size()/(float)max(1,num_parts);
	if(ratio>.90f)  //do we have at least 10% of the file?
		return false;
	else return true;
}

void FileBufferFile::ReceivedPart(int part)
{
	for(int i=0;i<(int)mv_previous_requests.size();i++){
		if(mv_previous_requests[i]==part){
			mv_previous_requests.erase(mv_previous_requests.begin()+i);
			return;
		}
	}
}

//the unfortunate and expensive process of verifying that the file we downloaded is in fact the file we think it is.  to prevent any corruption from entering the network
bool FileBufferFile::ValidateFile(HANDLE buffer_file,HANDLE log_file)
{
	SHA1 sha1;

	unsigned long ulFileSize = 0, ulRest = 0, ulBlocks = 0;
	unsigned long i = 0;
	byte uData[MAX_FILE_READ_BUFFER];

	LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
	LONG low_move=m_position;  
	DWORD dwPtr=SetFilePointer(buffer_file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

	DWORD error=-1;
	if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
		error=GetLastError();
		CString log_str;
		log_str.Format("FileBufferFile::ValidateFile() ERROR validating p2p file %s, size %d.  Error was %d in SetFilePointer win32 call.",m_file_hash.c_str(),m_total_length,error);
		WriteToLogFile(log_file,log_str);
		ASSERT(0);
		return false;
	}

	//FILE *fIn = NULL;

	ulRest = m_total_length % MAX_FILE_READ_BUFFER;
	ulBlocks = m_total_length / MAX_FILE_READ_BUFFER;

	DWORD tmp=NULL;
	for(i = 0; i < ulBlocks ; i++)
	{
		if(!ReadFile(buffer_file,uData,MAX_FILE_READ_BUFFER,&tmp,NULL)){
			CString log_str;
			DWORD error=GetLastError();
			log_str.Format("FileBufferFile::ValidateFile() ERROR reading buffer file to validate p2p file %s, size %d.  Error was %d in SetFilePointer win32 call.",m_file_hash.c_str(),m_total_length,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		//fread(uData, 1, MAX_FILE_READ_BUFFER, fIn);
		sha1.Update(uData, MAX_FILE_READ_BUFFER);
	}

	if(ulRest != 0)
	{
		if(!ReadFile(buffer_file,uData,ulRest,&tmp,NULL)){
			CString log_str;
			DWORD error=GetLastError();
			log_str.Format("FileBufferFile::ValidateFile() ERROR reading buffer file to validate p2p file %s, size %d.  Error was %d in SetFilePointer win32 call.",m_file_hash.c_str(),m_total_length,error);
			WriteToLogFile(log_file,log_str);
			ASSERT(0);
			return false;
		}
		sha1.Update(uData, ulRest);
	}
	
	sha1.Final();

	char ba[33];
	ba[0]='\0';
	sha1.Base32Encode(ba);

	if(stricmp(ba,m_file_hash.c_str())!=0){  
		//oh damn
		CString log_str;
		DWORD error=GetLastError();
		log_str.Format("FileBufferFile::ValidateFile() ERROR failed to validate (hash does not match) received p2p file %s, size %d.",m_file_hash.c_str(),m_total_length);
		WriteToLogFile(log_file,log_str);
		return false;
	}
	return true;
}

//just retrieves the mb_complete flag, instead of verifying a file or trying to determine if its complete
bool FileBufferFile::IsComplete2(void)
{
	return mb_complete;
}
