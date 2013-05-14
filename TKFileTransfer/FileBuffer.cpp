#include "StdAfx.h"
#include "filebuffer.h"
#include "TKFileTransferInterface.h"
#include <mmsystem.h>

//remember to stay below the 4GB limit for this stuff
#define MAXBUFFERSIZE		((1<<32)-800000000)//maximum size of allocated buffer - this is less than the total buffer size.  The reason is that it makes it much more efficient to find a spot for a file if the size is much less.
#define MAXFILESIZE			(1<<31)		//maximum size of any one file
#define TOTALBUFFERSIZE		((1<<32)-1)  //total theoretical capacity of buffer

const char* the_buffer_name="c:\\syncher\\internal file buffer\\buffer.dat";
const char* the_buffer_index_name="c:\\syncher\\internal file buffer\\buffer_index.txt";

FileBuffer::FileBuffer(void)
{
	CreateDirectory("c:\\syncher",NULL);
	CreateDirectory("c:\\syncher\\internal file buffer",NULL);

	mb_needs_index_save=false;
//	m_buffer_file=NULL;
	mb_need_file_update=false;
	m_used_space=0;

	//InitBufferFile();
	LoadIndex();
	m_log_file=NULL;
}

FileBuffer::~FileBuffer(void)
{
}

//UINT FileBuffer::Run(){
	//while(!this->b_killThread){

	//}
//}

//wish to add a file to the file buffer system
FileBufferFile* FileBuffer::AddFile(const char* hash, UINT file_size,HANDLE log_file)
{	
	CString log_str;
	log_str.Format("FileBuffer::AddFile() Add file %s, size %d BEGIN.",hash,file_size);
	WriteToLogFile(log_file,log_str);
	CSingleLock lock(&m_file_lock,TRUE);
	if(file_size==-1){
		log_str.Format("FileBuffer::AddFile() Add file FAILED, END %s, size %d.",hash,file_size);
		WriteToLogFile(log_file,log_str);
		ASSERT(0);
		return NULL;
	}
	TRACE("TKFileTransfer FileBuffer::AddFile() Attempting to add file to buffer hash=%s, size=%d.\n",hash,file_size);
	if(file_size>MAXFILESIZE){  //is this file too big?  then ditch it
		log_str.Format("FileBuffer::AddFile() Add file FAILED %s, size %d too big END.",hash,file_size);
		WriteToLogFile(log_file,log_str);
		return NULL;
	}

	if((MAXBUFFERSIZE-m_used_space)<file_size || mv_files.Size()>2000){  //do we need to free up space in our buffer to add this file?
		if(mv_files.Size()>2000){
			MakeMoreSpace(1,log_file);	
		}
		if((MAXBUFFERSIZE-m_used_space)<file_size){
			log_str.Format("FileBuffer::AddFile() Add file %s, size %d MakeMoreSpace BEGIN.",hash,file_size);
			WriteToLogFile(log_file,log_str);
			MakeMoreSpace(file_size-(MAXBUFFERSIZE-m_used_space),log_file);
			log_str.Format("FileBuffer::AddFile() Add file %s, size %d MakeMoreSpace END.",hash,file_size);
			WriteToLogFile(log_file,log_str);
		}
	}

	if((MAXBUFFERSIZE-m_used_space)<file_size){  //do we need to free up space in our buffer to add this file?
		log_str.Format("FileBuffer::AddFile() Add file FAILED %s, size %d.  IMPOSSIBLE END.",hash,file_size);
		WriteToLogFile(log_file,log_str);
		ASSERT(0);  //this should be impossible, but just in case.
		return NULL;
	}

	log_str.Format("FileBuffer::AddFile() Add file %s, size %d Sort file buffer BEGIN.",hash,file_size);
	WriteToLogFile(log_file,log_str);
	FileBufferFile::SortByPosition();
	mv_files.Sort(true);  //sort in ascending manner by position
	log_str.Format("FileBuffer::AddFile() Add file %s, size %d Sort file buffer END.",hash,file_size);
	WriteToLogFile(log_file,log_str);

	//peruse through and try to find a spot where we can insert our file
	log_str.Format("FileBuffer::AddFile() Add file %s, size %d INSERT FILE BEGIN.",hash,file_size);
	WriteToLogFile(log_file,log_str);
	UINT mark=0;

	while(mark<=TOTALBUFFERSIZE-file_size){
		bool b_avail=true;
		//we need to find a spot to insert the file
		//our approach will be to iterate through the files, and see if any of them conflict with our proposed spot.
		//if they do conflict, then we will advance the start of our proposed spot to the end of the conflicting file

		for(int i=0;i<(int)mv_files.Size();i++){
			FileBufferFile* file=(FileBufferFile*)mv_files.Get(i);
			UINT test_start=file->GetFilePosition();
			UINT test_end=test_start+file->GetFileSize();
			if(test_start==test_end)
				continue;  //don't test against zero length files that may be in the buffer.

			//test if file in question is inside our proposed spot
			if(test_start>=mark && test_start<mark+file_size){
				mark=test_end;  //advance mark forward past the end of this file
				b_avail=false;
				break;
			}
			else if(test_end>mark && test_end<mark+file_size){
				mark=test_end;  //advance mark forward past the end of this file
				b_avail=false;
				break;
			}
		}
		if(b_avail){  //sweet we found a spot.
			CTime cur_time=CTime::GetCurrentTime();
			FileBufferFile *nf=new FileBufferFile(hash,mark,file_size,cur_time,cur_time);
			TRACE("TKFileTransfer FileBuffer::AddFile() File Successfully added to buffer hash=%s, size=%d, position=%u. Allocated buffer space = %u.\n",hash,file_size,mark,m_used_space);
			log_str.Format("FileBuffer::AddFile() Added file %s, size %d, position=%u, allocated buffer=%u.",hash,file_size,mark,m_used_space);
			WriteToLogFile(log_file,log_str);
			mb_need_file_update=true;
			mv_files.Add(nf);
			log_str.Format("FileBuffer::AddFile() Added file [1] %s, size %d, position=%u, allocated buffer=%u.",hash,file_size,mark,m_used_space);
			WriteToLogFile(log_file,log_str);
			m_used_space+=file_size;
			SaveIndex(); 
			log_str.Format("FileBuffer::AddFile() Added file [2] %s, size %d, position=%u, allocated buffer=%u.",hash,file_size,mark,m_used_space);
			WriteToLogFile(log_file,log_str);
			mv_incomplete_files.Add(nf);  //this file is definitely incomplete also
			log_str.Format("FileBuffer::AddFile() Add file %s, size %d INSERT FILE SUCCESSFUL END.",hash,file_size);
			WriteToLogFile(log_file,log_str);
			return nf;
		}
	}
	
	log_str.Format("FileBuffer::AddFile() Add file %s, size %d Failed to insert, END INSERT BEGIN Optimize (defrag).",hash,file_size);
	WriteToLogFile(log_file,log_str);
	//it should be rather rare that it needs to find
	HANDLE buffer_file=GetBufferFile();
	bool did_optimize=OptimizeBuffer(buffer_file,log_file);

	log_str.Format("FileBuffer::AddFile() Add file %s, size %d Failed to insert, END Optimize (defrag).",hash,file_size);
	WriteToLogFile(log_file,log_str);

	CloseHandle(buffer_file);
	if(!did_optimize){  //rearrange our files within the buffer to make a gap large enough to add a file
		log_str.Format("FileBuffer::AddFile() Add file %s, size %d Failed to insert ABNORMAL COULD NOT OPTIMIZE END.",hash,file_size);
		WriteToLogFile(log_file,log_str);
		return NULL;
	}
	
	log_str.Format("FileBuffer::AddFile() Add file %s, size %d Failed to insert, but optimized END, recalling via recursion.",hash,file_size);
	WriteToLogFile(log_file,log_str);
	return AddFile(hash,file_size,log_file);  //recursively call this function again
}

void FileBuffer::MakeMoreSpace(int amount,HANDLE log_file)
{
	WriteToLogFile(log_file,"FileBuffer::MakeMoreSpace() BEGIN.");
	CSingleLock lock(&m_file_lock,TRUE);
	TRACE("TKFileTransfer Service FileBuffer::MakeMoreSpace(%d).\n",amount);
	FileBufferFile::SortByTime();
	mv_files.Sort(false);
	//the vector of files is sorted by last access time (oldest is at the end of the vector)
	int cleared=0;


	for(int i=mv_files.Size()-1;i>=0 && cleared<amount;i--){
		FileBufferFile *file=(FileBufferFile*)mv_files.Get(i);
		if(file->IsComplete2()){  //if we find one that is complete lets remove it.
			m_used_space-=file->GetFileSize();
			cleared+=file->GetFileSize(); //increase the amount of memory that we have cleared
			TRACE("TKFileTransfer Service FileBuffer::MakeMoreSpace() Removing hash %s with access time %s and size %u.  Buffer allocation is now %u\n",file->GetFileHash(),file->GetLastAccessTime(),m_used_space);
			mv_files.Remove(i);
			i--;
		}
	}


	//if we failed to remove enough completed files...  we have no choice but to remove incomplete files if we are going to add a new file.

	//none of them were complete (this trapper keeper never finished downloading a single file).  This situation sucks ass and should never happen (but is possible).  we must drop the oldest one, the system will recover.		FileBufferFile *file=(FileBufferFile*)mv_files.Get(i);
	while(mv_files.Size()>0 && cleared<amount){
		FileBufferFile *file=(FileBufferFile*)mv_files.Get(mv_files.Size()-1);
		m_used_space-=file->GetFileSize();  //reduce our total used space
		cleared+=file->GetFileSize();
		mv_incomplete_files.RemoveByReference(mv_files.Get(mv_files.Size()-1));
		mv_files.Remove(mv_files.Size()-1);
	}	

	SaveIndex();
	WriteToLogFile(log_file,"FileBuffer::MakeMoreSpace() END.");
}

//call to save the list of files
bool FileBuffer::SaveIndex(void)
{
	CString log_str;
	log_str.Format("FileBuffer::SaveIndex() BEGIN.");
	WriteToLogFile(m_log_file,log_str);
	CSingleLock lock(&m_file_lock,TRUE);
	mb_needs_index_save=false;
	//TRACE("TKFileTransfer Service FileBuffer::SaveIndex().\n");
	CreateDirectory("c:\\syncher",NULL);
	CreateDirectory("c:\\syncher\\internal file buffer",NULL);

	//open or create the file
	HANDLE index_file = CreateFile(the_buffer_index_name, // open file at local_path 
			GENERIC_WRITE,              // open for writing
			FILE_SHARE_READ,           // share for reading 
			NULL,                      // no security 
			OPEN_ALWAYS,             // open file if exists, create if it doesn't
			FILE_ATTRIBUTE_NORMAL,     // normal file 
			NULL);                     // no attr. template 

	if(index_file==NULL || index_file==INVALID_HANDLE_VALUE){
		log_str.Format("FileBuffer::SaveIndex() END [0].");
		WriteToLogFile(m_log_file,log_str);
		return false;
	}

	//DWORD fsize=0;  //our file size
	//DWORD hsize=0;  //high size (useless to us)
	//fsize=GetFileSize(index_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	//byte buffer[1500000];
	//DWORD num_read=0;
	TKFileTransferInterface::IPInterfaceDataSet data;
	
	//ReadFile(index_file,buffer,1500000,&num_read,NULL);
	data.v_ints.push_back(1);  //version 1, we will track version info so we know how to read in the file if we ever need to change our system
	data.v_ints.push_back(mv_files.Size());  //x number of files

//	log_str.Format("FileBuffer::SaveIndex() Iterating through files.");
//	WriteToLogFile(m_log_file,log_str);

	int save_size=8;
	for(int i=0;i<(int)mv_files.Size();i++){
//		log_str.Format("FileBuffer::SaveIndex() Iteration %d/%d",i,mv_files.Size());
//		WriteToLogFile(m_log_file,log_str);

//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [0].");
		FileBufferFile* file=(FileBufferFile*)mv_files.Get(i);

//		log_str.Format("FileBuffer::SaveIndex() iterating [1] - file size=%u.",file->GetFileSize());
//		WriteToLogFile(m_log_file,log_str);
		data.v_ints.push_back(file->GetFileSize());			save_size+=4;

//		log_str.Format("FileBuffer::SaveIndex() iterating [2] - file position=%u.",file->GetFilePosition());
//		WriteToLogFile(m_log_file,log_str);
		data.v_ints.push_back(file->GetFilePosition());		save_size+=4;

//		log_str.Format("FileBuffer::SaveIndex() iterating [3] - created year=%u.",file->GetCreatedYear());
//		WriteToLogFile(m_log_file,log_str);
		data.v_ints.push_back(file->GetCreatedYear());		save_size+=4;

//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [4].");
		data.v_ints.push_back(file->GetCreatedMonth());		save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [5].");
		data.v_ints.push_back(file->GetCreatedDay());		save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [6].");
		data.v_ints.push_back(file->GetCreatedHour());		save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [7].");
		data.v_ints.push_back(file->GetCreatedMinute());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [8].");
		data.v_ints.push_back(file->GetCreatedSecond());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [9].");
		data.v_ints.push_back(file->GetLastAccessYear());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [10].");
		data.v_ints.push_back(file->GetLastAccessMonth());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [11].");
		data.v_ints.push_back(file->GetLastAccessDay());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [12].");
		data.v_ints.push_back(file->GetLastAccessHour());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [13].");
		data.v_ints.push_back(file->GetLastAccessMinute());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [14].");
		data.v_ints.push_back(file->GetLastAccessSecond());	save_size+=4;
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [15].");

//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [16].");
		string the_hash=file->GetFileHash();
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [17].");
		data.v_strings.push_back(the_hash);					save_size+=(int)(the_hash.size()+1);
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [18].");
		string completed=file->EncodeCompletionToString();
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [19].");
		data.v_strings.push_back(completed);				save_size+=(int)(completed.size()+1);
//		WriteToLogFile(m_log_file,"FileBuffer::SaveIndex() iterating [20].");
	}

//	log_str.Format("FileBuffer::SaveIndex() Writing data structure to byte buffer.",save_size);
//	WriteToLogFile(m_log_file,log_str);

	byte *buffer=new byte[(1<<17)+save_size];  //allocate a buffer of the approximately correct size plus a little padding
	int len=data.WriteToBuffer(buffer);

	DWORD tmp;

//	log_str.Format("FileBuffer::SaveIndex() Writing byte buffer to hard disk.",save_size);
//	WriteToLogFile(m_log_file,log_str);
	BOOL stat=WriteFile(index_file,buffer,len,&tmp,NULL);
	
	CloseHandle(index_file);
	delete []buffer;

	if(stat){
		log_str.Format("FileBuffer::SaveIndex() END[1].");
		WriteToLogFile(m_log_file,log_str);
		return true;
	}
	else{
		log_str.Format("FileBuffer::SaveIndex() END[2].");
		WriteToLogFile(m_log_file,log_str);
		return false;
	}
}

//adjusts the position of various files inside the buffer to open up necessary gaps
bool FileBuffer::OptimizeBuffer(HANDLE buffer,HANDLE log_file)
{
	TRACE("TKFileTransfer Service FileBuffer::OptimizeBuffer().\n");
	//CString log_str;
	//log_str.Format("FileBuffer::OptimizeBuffer().");
	WriteToLogFile(log_file,"FileBuffer::OptimizeBuffer() BEGIN.");
	CSingleLock lock(&m_file_lock,TRUE);
	//if(m_buffer_file==NULL)
	//	return false;
	//FileBufferFile::SortByPosition();
	//mv_files.Sort(true);  //sort in ascending manner by position
	//do this 10 times

	UINT optimized_size=0;
	//optimize up to 256 megs worth of files, this shouldn't take too long, unless it finds a 2 gig file before the 256 megs has been moved

	for( int i=0 ; i<(int)mv_files.Size()-1 && optimized_size<(1<<28); i++ ){
		FileBufferFile *file=(FileBufferFile*)mv_files.Get(i);
		FileBufferFile *next_file=(FileBufferFile*)mv_files.Get(i+1);
		if(i==0){  //make sure the first file starts at the beginning.
			if(file->GetFilePosition()!=0){
				bool stat=file->MoveFile(buffer,0,log_file);
				optimized_size+=file->GetFileSize();
				if(!stat){
					SaveIndex(); //we have probably changed the index, it must be saved
					WriteToLogFile(log_file,"FileBuffer::OptimizeBuffer() END [1].");
					return false;
				}
			}
		}
		if(next_file->GetFilePosition()>(file->GetFilePosition()+file->GetFileSize())){
			bool stat=next_file->MoveFile(buffer,file->GetFilePosition()+file->GetFileSize(),log_file);
			optimized_size+=next_file->GetFileSize();
			if(!stat){
				SaveIndex(); //we have probably changed the index, it must be saved
				WriteToLogFile(log_file,"FileBuffer::OptimizeBuffer() END [2].");
				return false;
			}
		}
	}

	SaveIndex(); //we have changed the index, it must be saved
	WriteToLogFile(log_file,"FileBuffer::OptimizeBuffer() END [3].");
	return true;
}

HANDLE FileBuffer::GetBufferFile(void)
{
	CSingleLock lock(&m_file_lock,TRUE);
	//TRACE("TKFileTransfer Service FileBuffer::GetBufferFile().\n");

	//open or create the file
	HANDLE buffer_file = CreateFile(the_buffer_name, // open file at local_path 
			GENERIC_READ|GENERIC_WRITE,              // open for reading and writing
			FILE_SHARE_READ,           // share for reading 
			NULL,                      // no security 
			OPEN_ALWAYS,				// open existing or create if it doesn't exist
			FILE_ATTRIBUTE_NORMAL,     // normal file 
			NULL);                     // no attr. template 

	if(buffer_file==NULL || buffer_file==INVALID_HANDLE_VALUE){
		buffer_file=NULL;

		return NULL;
	}

	//get its current size
	DWORD fsize=0;  //our file size
	DWORD hsize=0;  //high size (useless to us)
	fsize=GetFileSize(buffer_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(hsize!=0){
		ASSERT(0);
	}

	BOOL stat=SetFilePointer(buffer_file,0,0,FILE_END);  //move to the end of the file
	byte *buffer=new byte[(1<<20)];
	ZeroMemory(buffer,(1<<20));  //zero this buffer so we write zero's as padding into the file
	UINT attempt=0;
	int write_attempt=0;
	while(fsize<TOTALBUFFERSIZE && attempt<1000){  //move to where the file will be moved to
		DWORD tmp;
		BOOL stat2=FALSE;
		if(TOTALBUFFERSIZE-fsize<(UINT)(1<<20)){
			stat2=WriteFile(buffer_file,buffer,TOTALBUFFERSIZE-fsize,&tmp,NULL);
		}
		else{
			stat2=WriteFile(buffer_file,buffer,(1<<20),&tmp,NULL);
		}
		fsize+=tmp;

		if((write_attempt++%50)==0){
			TRACE("TKFileTransfer Service FileBuffer::Update() Buffer File Allocation in progress... %u bytes allocated\n",fsize);
		}

		if(tmp==0){
			attempt++;
		}
	}

	delete []buffer;
	//load the files

	return buffer_file;
}

bool FileBuffer::LoadIndex(void)
{
	CSingleLock lock(&m_file_lock,TRUE);
	TRACE("TKFileTransfer Service FileBuffer::LoadIndex().\n");
	CreateDirectory("c:\\syncher",NULL);
	CreateDirectory("c:\\syncher\\internal file buffer",NULL);

	mv_incomplete_files.Clear();
	mv_files.Clear();
	m_used_space=0;

	if(!DoesBufferFileExist()){  //check to see if the main buffer file is missing, if it is then we truely have no files.
		return false;
	}

	//open or create the file
	HANDLE index_file = CreateFile(the_buffer_index_name, // open file at local_path 
			GENERIC_READ,              // open for reading
			FILE_SHARE_READ,           // share for reading 
			NULL,                      // no security 
			OPEN_ALWAYS,             // open file if exists, create if it doesn't
			FILE_ATTRIBUTE_NORMAL,     // normal file 
			NULL);                     // no attr. template 

	if(index_file==NULL || index_file==INVALID_HANDLE_VALUE){
		return false;
	}

	DWORD fsize=0;  //our file size
	DWORD hsize=0;  //high size (useless to us)
	fsize=GetFileSize(index_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	byte *buffer=new byte[4000000];
	DWORD num_read=0;
	TKFileTransferInterface::IPInterfaceDataSet data;
	
	ReadFile(index_file,buffer,4000000,&num_read,NULL);


	//create a special log file for this event
	CreateDirectory("c:\\TKFileTransfer Logs",NULL);
	HANDLE log_file= CreateFile("c:\\TKFileTransfer Logs\\log_file_buffer_index_load.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	CString log_str;

	log_str.Format("FileBuffer::LoadIndex(void) Loading Index File... %u bytes read",num_read);
	WriteToLogFile(log_file,log_str);

	if(num_read>0){
		data.ReadFromBuffer(buffer);
		int version=data.v_ints[0];
		int num_files=data.v_ints[1];

		log_str.Format("FileBuffer::LoadIndex(void) Loading Index File... verion=%d, num_files=%d",version,num_files);
		WriteToLogFile(log_file,log_str);
		//data.v_ints.erase(data.v_ints.begin(),data.v_ints.begin()+1);  //pop our variables off the vector
		//data.v_ints.erase(data.v_ints.begin()+1);  //pop our variables off the vector
		if(version==1){  //check to make sure we know how to read this version
			for(int i=0;i<num_files && i<2000;i++){
				string sha=data.v_strings[2*i];
				string complete=data.v_strings[2*i+1];
				UINT size=(UINT)data.v_ints[14*i+2];			//the +2 offset is from having the version and num_files pushed onto the int vector
				UINT position=(UINT)data.v_ints[14*i+1+2];
				UINT year=(UINT)data.v_ints[14*i+2+2];
				UINT month=(UINT)data.v_ints[14*i+3+2];
				UINT day=(UINT)data.v_ints[14*i+4+2];
				UINT hour=(UINT)data.v_ints[14*i+5+2];
				UINT minute=(UINT)data.v_ints[14*i+6+2];
				UINT second=(UINT)data.v_ints[14*i+7+2];

				UINT year2=(UINT)data.v_ints[14*i+8+2];
				UINT month2=(UINT)data.v_ints[14*i+9+2];
				UINT day2=(UINT)data.v_ints[14*i+10+2];
				UINT hour2=(UINT)data.v_ints[14*i+11+2];
				UINT minute2=(UINT)data.v_ints[14*i+12+2];
				UINT second2=(UINT)data.v_ints[14*i+13+2];

				log_str.Format("FileBuffer::LoadIndex(void) File %d/%d has properties sha='%s' , complete='%s' , size=%u , position=%u , year=%d , month=%d , day=%d , hour=%d , minute=%d, second=%d",
					i,
					num_files,
					sha.c_str(),
					complete.c_str(),
					size,
					position,
					year,
					month,
					day,
					hour,
					minute,
					second);
				WriteToLogFile(log_file,log_str);				

				log_str.Format("FileBuffer::LoadIndex(void) File %d/%d has more properties year2=%d , month2=%d , day2=%d , hour2=%d , minute2=%d , second2=%d",
					i,
					num_files,
					year2,
					month2,
					day2,
					hour2,
					minute2,
					second2);
				WriteToLogFile(log_file,log_str);	

				//check for corruption in the index buffer and jump out if its screwy for whatever reason
				if(year<2003 || year>2036 || year2>2036 || year2<2003 || month2>13 || day2>32 || minute2>61 || second2>61 || month >13 || minute>61 || second>61 || size==0 || day>32)
					break;  //abandon the rest of this index file, it got corrupted somehow

				FileBufferFile *nf=new FileBufferFile(sha.c_str(),position,size,CTime(year,month,day,hour,minute,second),CTime(year2,month2,day2,hour2,minute2,second2));
				if(!nf->DecodeCompletionFromString(complete)){  //determine completion of the file
					mv_incomplete_files.Add(nf);  //if it wasn't complete, then add it to our special list of incomplete files.  A vast majority of the files in our buffer should be complete at any one time
				}
				mv_files.Add(nf);
				m_used_space+=size;
			}
		}
	}

	CloseHandle(index_file);
	CloseHandle(log_file);
	delete[] buffer;


	return true;
}

int FileBuffer::GetFilePart(const char* hash, UINT file_size, byte* buffer,UINT part)
{
	WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() BEGIN.");
	CSingleLock lock(&m_file_lock,TRUE);
	for(UINT i=0;i<mv_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_files.Get(i);
		if(fbf->IsFile(hash,file_size)){
			if(!DoesBufferFileExist()){
				//someone has deleted the buffer file, lets clear any knownledge of our files
				mv_files.Clear();
				mv_incomplete_files.Clear();
				m_used_space=0;
				SaveIndex();
				mb_need_file_update=true;
				WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() END[0].");
				return -1;
			}
			HANDLE buffer_file=GetBufferFile();
			int stat= fbf->GetFilePart(buffer_file,buffer,part);
			CloseHandle(buffer_file);
			WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() END[1].");
			return stat;
		}
	}
	WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() END[2].");
	return -1;
}

void FileBuffer::EnumerateFilesForIndexServer(vector<string> & v_file_hashes, vector<int> & v_file_sizes)
{
	CSingleLock lock(&m_file_lock,TRUE);
//	WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() BEGIN.");
	for(int i=0;i<(int)mv_files.Size();i++){
		FileBufferFile* file=(FileBufferFile*)mv_files.Get(i);
		if(file->HasMinimumPartsForIndexServer()){
			v_file_hashes.push_back(string(file->GetFileHash()));
			v_file_sizes.push_back(file->GetFileSize());
		}
	}
//	WriteToLogFile(m_log_file,"FileBuffer::GetFilePart() END.");
}

bool FileBuffer::WriteFilePart(const char* hash, UINT file_size, UINT part, byte* buffer, UINT data_length,HANDLE log_file)
{
	CString log_str;
	log_str.Format("FileBuffer::WriteFilePart() BEGIN.");
	WriteToLogFile(log_file,log_str);
	CSingleLock lock(&m_file_lock,TRUE);
	//bool b_done=false;
	FileBufferFile* file=NULL;
	for(UINT i=0;i<mv_incomplete_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
		if(fbf->IsFile(hash,file_size)){
			file=fbf;
			break;
		}
	}
	if(file){
		bool stat=false;
		HANDLE buffer_file=GetBufferFile();
		if((stat=file->WriteFilePart(buffer_file,part,buffer,data_length,log_file))){
			mv_incomplete_files.RemoveByReference(file);  //this file is now complete so lets remove it from our list of incomplete files
		}
		CloseHandle(buffer_file);
		mb_needs_index_save=true; 
		log_str.Format("FileBuffer::WriteFilePart() END [1].");
		WriteToLogFile(log_file,log_str);
		return stat;
	}
	else{  //for some reason we have no record of this file, lets ignore the data being sent for this file.
		log_str.Format("FileBuffer::WriteFilePart() END [2].");
		WriteToLogFile(log_file,log_str);
		return false;
	}

	//we have made changes, we should save our new file index for the next time the program starts
	//if we don't save it right now (like trying to optimize it) there is a chance that some other dll could crash and our entire 3 GB buffer would be corrupt (well it wouldn't be corrupt but it would be effectively lost).  This must not be allowed to happen.				
}

bool FileBuffer::GetFileStatusAsString(const char* sha1, UINT file_size, string& completed)
{
	CSingleLock lock(&m_file_lock,TRUE);
	for(UINT i=0;i<mv_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_files.Get(i);
		if(fbf->IsFile(sha1,file_size)){
			completed=fbf->EncodeCompletionToString();
			return true;
		}
	}
	return false;
}

/*
void FileBuffer::GetListOfCompleteFiles(vector<string>& v_hashes, vector<UINT>& v_sizes)
{
	//build a list of all files that are complete
	for(UINT i=0;i<mv_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_files.Get(i);
		if(fbf->IsComplete()){
			v_hashes.push_back(string(fbf->GetFileHash()));
			v_sizes.push_back(fbf->GetFileSize());
		}
	}
}*/

//call to see if a file is in the system, and get a pointer to it if it is
FileBufferFile* FileBuffer::GetFile(const char* hash, UINT size)
{
	CSingleLock lock(&m_file_lock,TRUE);
	for(UINT i=0;i<mv_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_files.Get(i);
		if(fbf->IsFile(hash,size)){
			return fbf;
		}
	}
	return NULL;
}

//a source has sent us a file status update
void FileBuffer::UpdateSources(const char* hash, UINT file_size ,vector <string> &v_source_ips)
{
	CSingleLock lock(&m_file_lock,TRUE);
	for(UINT i=0;i<mv_incomplete_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
		if(fbf->IsFile(hash,file_size)){
			for(int j=0;j<(int)v_source_ips.size();j++){
				fbf->UpdateSource(v_source_ips[j].c_str());
			}
			break;
		}
	}
}

bool FileBuffer::ManifestOutOfBuffer(FileBufferFile *file,const char* full_path,HANDLE log_file){
	CString log_str;
	log_str.Format("FileBuffer::ManifestOutOfBuffer() Add file %s, size %d BEGIN.",file->GetFileHash(),file->GetFileSize());
	WriteToLogFile(log_file,log_str);
	CSingleLock lock(&m_file_lock,TRUE);

	TRACE("TKFileTransfer Service FileBuffer::ManifestOutOfBuffer(%s).\n",full_path);
	HANDLE buffer_file=GetBufferFile();
	bool stat= file->ManifestOutOfBuffer(buffer_file,full_path,log_file);
	CloseHandle(buffer_file);

	log_str.Format("FileBuffer::ManifestOutOfBuffer() Add file %s, size %d END.",file->GetFileHash(),file->GetFileSize());
	WriteToLogFile(log_file,log_str);
	return stat;
}

void FileBuffer::Update(TKComInterface *p_com, HANDLE log_file)
{
	CSingleLock lock(&m_file_lock,TRUE);
	m_log_file=log_file;
	//we only care about incomplete files, so we will be working with the incomplete vector here
	static Timer last_part_request;
	static Timer last_age_check;
	//TRACE("TKFileTransfer Service FileBuffer::Update().\n");

	if(last_age_check.HasTimedOut(60*60*3)){
		CString log_str;
		log_str.Format("FileBuffer::Update() AGECHECK BEGIN.");
		WriteToLogFile(log_file,log_str);
		//WriteToLogFile(log_file,"FileBuffer::Update() BEGIN Doing incomplete file check.");
		TRACE("TKFileTransfer Service FileBuffer::Update() Checking for old files.\n");
		bool b_changed=false;
		for(UINT i=0;i<mv_incomplete_files.Size();i++){
			FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
			if(fbf->IsBad()){  //if this file is incomplete, and has been for a long time, then lets discard it.
				TRACE("TKFileTransfer Service FileBuffer::Update() removing old file %s.\n",fbf->GetFileHash());
				mv_files.RemoveByReference(fbf);
				mv_incomplete_files.Remove(i);
				b_changed=true;
				i--;
			}
		}
		if(b_changed){
			SaveIndex();
		}
		last_age_check.Refresh();
		log_str.Format("FileBuffer::Update() AGECHECK END.");
		WriteToLogFile(log_file,log_str);
		//WriteToLogFile(log_file,"FileBuffer::Update() END Doing incomplete file check.");
	}

	srand(timeGetTime());
	for(UINT i=0;i<mv_incomplete_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
		//WriteToLogFile(log_file,"FileBuffer::Update() BEGIN processing incomplete file.");
		fbf->Update();  //let each of the files update themselves

		fbf->MakePartRequest(p_com,log_file);  //let each of the files request another part if they are able to do so
		//WriteToLogFile(log_file,"FileBuffer::Update() END processing incomplete file.");
	}

	if(mb_needs_index_save){
		SaveIndex();
	}
	last_part_request.Refresh();
}

void FileBuffer::EnumerateIncompleteFiles(vector<string>& v_file_hashes, vector<UINT>& v_file_sizes)
{
	CSingleLock lock(&m_file_lock,TRUE);
	//build a list of all files that are not complete
	for(UINT i=0;i<mv_incomplete_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
		v_file_hashes.push_back(string(fbf->GetFileHash()));
		v_file_sizes.push_back(fbf->GetFileSize());
	}
}

void FileBuffer::EnumerateCompleteFiles(Vector &v_files)
{
	//build a list of all files that are complete
	CSingleLock lock(&m_file_lock,TRUE);
	for(UINT i=0;i<mv_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_files.Get(i);
		if(fbf->IsComplete2()){
			v_files.Add(fbf);
		}
	}
}

void FileBuffer::QueryIncompleteP2PJobs(vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_num_parts, vector <int> &v_completed_parts, vector<string> &v_sources){
	CSingleLock lock(&m_file_lock,TRUE);
	//build a list of all files that are not complete
	for(UINT i=0;i<mv_incomplete_files.Size();i++){
		FileBufferFile* fbf=(FileBufferFile*)mv_incomplete_files.Get(i);
		vector <int> v_tmp;
		fbf->GetIncompleteParts(v_tmp);
		v_hashes.push_back(string(fbf->GetFileHash()));
		v_size.push_back((int)fbf->GetFileSize());
		int num_parts=(int)((fbf->GetFileSize()/PARTSIZE)+1);
		v_num_parts.push_back(num_parts);
		v_completed_parts.push_back(num_parts-(int)v_tmp.size());
		v_sources.push_back(fbf->GetSources());
	}
}

UINT FileBuffer::GetAllocatedBuffer(void)
{
	return m_used_space;
}

UINT FileBuffer::GetFileCount(void)
{
	return mv_files.Size();
}

void FileBuffer::WriteToLogFile(HANDLE file, const char* str)
{
	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%m/%d %I:%M %p"),str);
	DWORD tn;
	WriteFile(file,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}

bool FileBuffer::DoesBufferFileExist(void)
{
	CString log_str;
	log_str.Format("FileBuffer::GetBufferFile() STARTED.");
	WriteToLogFile(m_log_file,log_str);	
	CSingleLock lock(&m_file_lock,TRUE);
	//open or create the file
	HANDLE buffer_file = CreateFile(the_buffer_name, // open file at local_path 
			GENERIC_READ|GENERIC_WRITE,              // open for reading and writing
			FILE_SHARE_READ,           // share for reading 
			NULL,                      // no security 
			OPEN_ALWAYS,				// open existing or create if it doesn't exist
			FILE_ATTRIBUTE_NORMAL,     // normal file 
			NULL);                     // no attr. template 

	if(buffer_file==NULL || buffer_file==INVALID_HANDLE_VALUE){
		log_str.Format("FileBuffer::GetBufferFile() FINISHED [0].");
		WriteToLogFile(m_log_file,log_str);	
		return false;
	}

	//get its current size
	DWORD fsize=0;  //our file size
	DWORD hsize=0;  //high size (useless to us)
	fsize=GetFileSize(buffer_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	CloseHandle(buffer_file);
	if(hsize!=0){
		ASSERT(0);
		DeleteFile(the_buffer_name);  //fix it for later
		log_str.Format("FileBuffer::GetBufferFile() FINISHED [1].");
		WriteToLogFile(m_log_file,log_str);	
		return false;
	}


	if(fsize<TOTALBUFFERSIZE){
		log_str.Format("FileBuffer::GetBufferFile() FINISHED [2].");
		WriteToLogFile(m_log_file,log_str);	
		return false;
	}
	log_str.Format("FileBuffer::GetBufferFile() FINISHED [3].");
	WriteToLogFile(m_log_file,log_str);	
	return true;
}
