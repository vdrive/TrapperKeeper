#include "StdAfx.h"
#include "filetransferjob.h"
#include "TKFileTransferInterface.h"
#include "FileTransferDialog.h"
#include "..\tkcom\Sha1.h"
using namespace filetransferspace;

UINT log_counter=0;

FileTransferJob::FileTransferJob()
{
	m_log_id=log_counter++;
	Reset();
}

FileTransferJob::~FileTransferJob(void)
{
	if(m_file!=NULL)
		CloseHandle(m_file);
}

//called to begin a send to the dest with a specific job index
//returns a failure reason, or NULL if successfull
char* FileTransferJob::Init(char *dest,UINT job_index, string local_path,UINT offset,const char *pcstr_sha1)
{
	
	if(m_file!=NULL){
		CloseHandle(m_file);
		m_file=NULL;
	}
	
	Reset();
	m_dest=dest;
	m_job_index=job_index;
	m_local_path=local_path;

	if(m_dest.size()<1){
		return "no destination address specified";
	}

	//check path integrity
	CString path=local_path.c_str();
	path.Trim();
	path.MakeLower();
	if(path.GetLength()<1 || path.Find("c:\\windows")!=-1){  //A little bit of security.
		TRACE("A plugin at %s requested that a file with no name be sent to it or the requested file violates security.\n",m_dest.c_str());
		return "We discriminate against improper requests on this fileserver.  No blank requests or request to c:\\windows allowed.\n";
	}

	if( pcstr_sha1 != NULL ){  //if this is a resume job, we need to make sure that our file is still the same file that the requester thinks that it is.
		char ba[33];
		ba[0]='\0';
		SHA1 sha1;
		if(sha1.HashPartOfFile((char*)local_path.c_str(),0,offset)){
			sha1.Final();
			sha1.Base32Encode(ba);
		}
		else{ //else the file isn't even as long as the requesters chunk anymore, because it must have changed.
			return "The file requester is trying to resume a file that has changed.  Ignoring request.";
		}
		
		if(stricmp(pcstr_sha1,ba)!=0){  //do the sha1's of what the requester has already received, and the corresponding part of the file on this server match?
			return "The file requester is trying to resume a file that has changed.  Ignoring request.";
		}
	}



	//open the file
	m_file = CreateFile(local_path.c_str(), // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		TRACE("A plugin at %s requested that the file %s be sent to it.  This file could not be opened.\n",m_dest.c_str(),local_path.c_str());
		//CString msg;
		//msg.Format("Unable to open file %s because of %d.",local_path.c_str(),GetLastError());
		//MessageBox(NULL,msg,"ERROR",MB_OK);
		m_file=NULL;
		return "Unable to open the requested file";
	}
	m_transferred=offset;
	//DEL //init our transferred amount to 0
	//DEL //m_transferred=0;
	//resume ability
	if(offset>0){
		SetFilePointer(m_file,offset,0,FILE_BEGIN);
	}


	DWORD fsize=0;
	DWORD hsize=0;
	fsize=GetFileSize(m_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.

	//init our file size
	m_total=fsize;
	
	//this is read to send
	mb_inited=true;
	m_last_used.Refresh();
	return NULL; //return no error
}

//call to send more data to the destination
void FileTransferJob::Process(ComLink* plink){
	if(!mb_inited || m_file==NULL || mb_done_notified || mb_data_error)
		return;

	TRY{
		TKFileTransferInterface::IPInterface message;

		if(mb_done){
			message.m_type=message.FILETRANSFERDONE;
			message.v_ints.push_back(m_job_index);
			byte buf[2048];
			int nw=message.WriteToBuffer(buf);
			plink->SendReliableData((char*)m_dest.c_str(),buf,nw);

			g_file_transfer_dialog.SendDone(m_log_id);

			CloseHandle(m_file);
			m_file=NULL;
			m_last_used.Refresh();
			mb_done_notified=true;
			mv_data_buffer.Clear();
			return;
		}
		
		//we are not done, lets send a chunk
		//byte *temp_buf=new byte[FILECHUNKSIZE+2048];
		
		//DWORD nread=0;
		//ReadFile(m_file,temp_buf,FILECHUNKSIZE,&nread,NULL);
		//m_transferred+=nread;
		//if(nread==0)  //This is wierd, let it do a timeout and request it all over again.
		//	return;	

		message.m_type=message.FILEDATA;
		message.v_ints.push_back(m_job_index);
		message.v_ints.push_back(m_transferred);
		message.v_ints.push_back(m_total);
		if(mv_data_buffer.Size()<1){
			mb_data_error=true;
			return;
		}
		FileCacher::FileSegment *fs=(FileCacher::FileSegment *)mv_data_buffer.Get(0);
		message.SetData(fs->m_data,fs->m_length);
		

		byte *response_buf=new byte[FILECHUNKSIZE+2048];
		int nw=message.WriteToBuffer(response_buf);

		int retry_count = 0;
		//try to send the data a few times.  We may have too much queued up.
		while(!plink->SendReliableData((char*)m_dest.c_str(),response_buf,nw) && (retry_count++<5)){
			Sleep(200); //sleep for a moment and then try again..
		}

		delete []response_buf;
		//delete []temp_buf;

		g_file_transfer_dialog.SendProgress(m_log_id,(float)((double)m_transferred/(double)m_total));
		m_last_used.Refresh();
	}
	CATCH_ALL( e ){  //MFC EXCEPTION HANDLING IS "SPECIAL"
		TRACE("Exception caught in FileTransferService::FileTransferJob::SendMore()\n");
	}
	END_CATCH_ALL

	mv_data_buffer.Clear();
}

//called to see if this is a specific job
bool FileTransferJob::IsJob(UINT job_index , char* peer)
{
	if(job_index==m_job_index && stricmp(peer,m_dest.c_str())==0)
		return true;
	else return false;
	return 0;
}

//called to see if there has been any activity
bool FileTransferJob::IsDead(void)
{
	if(m_last_used.HasTimedOut(21*60)){  //no action for 21 mins = dead
		g_file_transfer_dialog.SendFailed(m_log_id);
		return true;
	}
	else
		return false;
}
void FileTransferJob::BeginLoad(void)
{
	mb_data_loading=true;
	this->StartThread();
}

bool FileTransferJob::IsLoading(void)
{
	return mb_data_loading;
}

UINT FileTransferJob::Run(){
	if(mb_done){
		mb_data_ready=true;
		mb_data_loading=false;
		return 0;
	}
	if(!mb_inited || m_file==NULL){
		mb_data_ready=true;
		mb_data_loading=false;
		mb_data_error=true;
		return 0;
	}

	
	if(m_transferred>=m_total){
		mb_done=true;
		mb_data_ready=true;
		mb_data_loading=false;
		return 0;
	}	

	mv_data_buffer.Clear();
	bool b_got_cache=g_file_cacher.GetSegment(m_local_path.c_str(),m_transferred,mv_data_buffer);

	if(!b_got_cache){
		byte *data_buffer=new byte[FILECHUNKSIZE];
		DWORD data_length=0;
		BOOL stat=ReadFile(m_file,data_buffer,FILECHUNKSIZE,&data_length,NULL);
		FileCacher::FileSegment *ns=new FileCacher::FileSegment(m_local_path.c_str(),m_transferred,data_buffer,data_length);
		mv_data_buffer.Add(ns);
		if(!stat){  //This is wierd, let it do a timeout and request it all over again.
			mb_data_ready=true;
			mb_data_loading=false;
			mb_data_error=true;
			return 0;	
		}
		else{
			g_file_cacher.AddSegment(ns);
			m_transferred+=ns->m_length;
		}
	}
	else{
		FileCacher::FileSegment *fs=(FileCacher::FileSegment*)mv_data_buffer.Get(0);
		SetFilePointer(m_file,m_transferred+fs->m_length,0,FILE_BEGIN);
		g_file_transfer_dialog.AddCacheHit();
		m_transferred+=fs->m_length;
	}

	mb_data_ready=true;
	mb_data_loading=false;
	return 0;
}

bool FileTransferJob::IsDoneLoading(void)
{
	return mb_data_ready;
}

void FileTransferJob::Reset(void)
{
	m_transferred=0;
	m_total=0;
	mb_done=false;
	mb_inited=false;
	m_file=NULL;

	mb_data_ready=false;
	mb_data_loading=false;
	mb_data_error=false;
	mb_done_notified=false;
	mv_data_buffer.Clear();
}

void FileTransferJob::ResetLoad(void)
{
	mb_data_ready=false;
	mb_data_loading=false;
	mv_data_buffer.Clear();
}
