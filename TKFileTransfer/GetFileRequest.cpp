#include "StdAfx.h"
#include "getfilerequest.h"
#include <Mmsystem.h>
#include "FileTransferDialog.h"
#include "..\tkcom\sha1.h"
using namespace filetransferspace;

UINT GetFileRequest::m_job_counter=1;

UINT log_id_counter=0;

GetFileRequest::GetFileRequest(UINT from_app, UINT session_id, char* source, char* local_path, char* remote_path)
{
	m_retry=0;
	m_written=0;
	m_log_id=log_id_counter++;
	m_source=source;
	m_from_app=from_app;
	m_session_id=session_id;
	CString tstr=local_path;
	m_local_path=local_path;
	m_tmp_path=local_path;
	tstr.MakeLower();
	if(tstr.Find("c:\\syncher\\rcv")!=-1){  //we will store it in a tmp directory until we are done with the copy.
		tstr.Replace("c:\\syncher\\rcv","c:\\syncher\\tmp");
		m_tmp_path=(LPCSTR)tstr;
	}
	CString orig_path=local_path;
	
	m_remote_path=remote_path;
	m_job_index=m_job_counter++; //each GetFileRequest gets a unique id
	m_file=NULL;

	g_file_transfer_dialog.NewGet(local_path,source,m_log_id);

	CString tmp=m_tmp_path.c_str();
	if(tmp.Trim().GetLength()<1){  //do a few miscellaneous checks to make sure this system will remain stable.
		TRACE("FILE TRANSFER SERVICE:  Ignoring request from plugin %d to create a file with a blank name.\n",m_from_app);
		return;
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

	//we need to create the directory structure for the original path as well
	index=0;
	dcount=0;
	while((index=orig_path.Find("\\",index))!=-1){
		CString ltmp=orig_path.Left(index);
		index++;
		dcount++;
		if(dcount==1)
			continue;
		//skip the first one, we don't want to create "c:\\" etc.
		CreateDirectory(ltmp,NULL);//we have to retardly build this path up one directory at a time.
	}

	m_file = CreateFile(m_tmp_path.c_str(),			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		CString log_msg;
		int error=GetLastError();
		//log_msg.Format("FAILED:  Couldn't create the file %s to received it from %s, system error code = %d.\n",m_local_path.c_str(),m_source.c_str(),GetLastError());
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
		g_file_transfer_dialog.GetFailed(m_log_id);
		TRACE("FILE TRANSFER SERVICE:  Couldn't Create File %s for plugin %d from %s.\n",m_local_path.c_str(),m_from_app,source);
		m_file=NULL;
	}
	else{
		CString log_msg;
		log_msg.Format("Successfully initiated a GET FILE request from %s for the FILE %s to SAVE AS %s, system error code = %d.\n",m_source.c_str(),remote_path,m_local_path.c_str(),GetLastError());
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
	}
}

GetFileRequest::~GetFileRequest(void)
{
	CleanUpFailure();  //for when the program quits, deletes incomplete downloads.
}

bool GetFileRequest::IsDead(ComLink *p_com_link)
{
	if(m_file==NULL || m_last_update.HasTimedOut(6*60)){ //if we haven't heard anything in 30 minutes, this request has failed
		if(m_retry<3 && m_file!=NULL){
			m_last_update.Refresh();
			m_retry++;
			//begin resume
			//send a request via com to this destination to initiate the transfer
			TKFileTransferInterface::IPInterface remote_request;
			//reassign this job_index so there are no confusions (i.e. the source happens to be able to send us a very very delayed chunk, which we will now just ignore)
			UINT old_index=m_job_index;
			m_job_index=m_job_counter++; //each GetFileRequest gets a unique id
			remote_request.v_ints.push_back(old_index);  //let the source reference the old index so we don't see a lot of failure stack up on the source side
			remote_request.v_ints.push_back(m_job_index);  //the new job index
			remote_request.v_ints.push_back(m_written);  //how much data we did receive
			remote_request.v_strings.push_back(m_remote_path);

			//get a sha1 for the file that we have written so far
			SHA1 sha1;
			if(!sha1.HashPartOfFile((char*)m_local_path.c_str(),0,m_written))
				return true;

			char ba[33];
			ba[0]='\0';
			sha1.Final();
			sha1.Base32Encode(ba);
			remote_request.v_strings.push_back(string(ba));  //send our sha too, so that the source can compare to see if it should bother sending more data.  (in case the source file changed for some wierd reason, we shouldn't corrupt this destinations file)

			remote_request.m_type=remote_request.RESUME;

			byte buf[4096];
			int nw=remote_request.WriteToBuffer(buf); //put this request into a byte buffer
			//send our request for initiating a send!
			ASSERT(nw<600);
			p_com_link->SendReliableData((char*)m_source.c_str(),buf,nw);
			return false;  //we aren't done yet.
		}
		g_file_transfer_dialog.GetFailed(m_log_id);
		CleanUpFailure();
		return true;
	}
	else
		return false;
}

bool GetFileRequest::Process(byte* data, UINT data_size)
{
	if(m_file==NULL)
		return false;

	UINT num_written=0;

	for(int i=0;i<20 && num_written<data_size;i++){ //try for a moment to write all this data out.  
		int old_num=num_written;
		WriteFile(m_file,data+num_written,data_size-num_written,(DWORD*)&num_written,NULL);
		if(num_written==0){
			TRACE("FILE TRANFER SERVICE: WriteFile FAILED because of %d:\n",GetLastError());
		}
		num_written=old_num+num_written;
		if(num_written<data_size)
			Sleep(10);
	}

	if(num_written<data_size){
		//for whatever reason, we can't write to it. 
		//CString log_msg;
		//log_msg.Format("FAILED to write to FILE %s from SOURCE %s with windows error code %d.\n",m_local_path.c_str(),m_source.c_str(),GetLastError());
		//g_file_transfer_dialog.GetFailed(m_log_id);
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
		CleanUpFailure();
		return false; //master we have failed
	}
	else{ 
		m_last_update.Refresh();
		m_written+=data_size;
		//m_current_sha1.Update(data,data_size);  //track the sha1 of the data that we have read in, just in case we need to do a resume.
		return true;
	}
}

void GetFileRequest::CleanUpFailure(void)
{
	if(m_file!=NULL){  //lets remove this incomplete file that we made
		CloseHandle(m_file);
		DeleteFile(m_tmp_path.c_str());
		m_file=NULL;
	}
}

void GetFileRequest::Finished(void)
{
	CloseHandle(m_file);
	m_file=NULL;

	CString str=m_tmp_path.c_str();
	str.MakeLower();
	CString orig_path=str;
	
	if(str.Find("c:\\syncher\\tmp")!=-1){  //we were storing it in a tmp directory, now we will copy it over.
		str.Replace("c:\\syncher\\tmp","c:\\syncher\\rcv");
		CopyFile(orig_path,str,FALSE);
		DeleteFile(orig_path);
		CString tmp=orig_path;
		vector <CString> v_directories;
		int index=0;
		while((index=tmp.Find("\\",index))!=-1){  //enumerate the directories, so we can remove them in reverse order
			CString ltmp=tmp.Left(index);
			index++;
			v_directories.push_back(ltmp);			
		}
		for(UINT i=(UINT)v_directories.size()-1;i>2;i--){
			RemoveDirectory((LPCSTR)v_directories[i]);
		}
	}
}
