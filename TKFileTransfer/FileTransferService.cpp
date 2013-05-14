#include "StdAfx.h"
#include "filetransferservice.h"
#include "GetFileRequest.h"
#include "FileTransferJob.h"
#include "DllInterface.h"
#include "FileTransferDialog.h"
#include "p2pgetfilejob.h"
#include "p2psendfilejob.h"
#include <Winsock2.h>
using namespace filetransferspace;

#define GETFILERESPONSE 10
//#define INDEXSERVERADDRESS "38.119.64.66"

FileTransferService::FileTransferService(void)
{
	mb_new_get_request=false;
	mb_completed_file=false;
	p_com_link=NULL;
	CreateDirectory("c:\\syncher\\tmp",NULL);
	CreateDirectory("c:\\TKFileTransfer Logs",NULL);
	m_log_file_service= CreateFile("c:\\TKFileTransfer Logs\\log_file_main_service.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 



	/*
	WSADATA wsaData;
	int retval=0;
    if ((retval = WSAStartup(0x202,&wsaData)) != 0) {  //tell winsock that this process requires its services
        TRACE("COM SERVICE:  ERROR!  WSAStartup failed with error %d\n",retval);  //this should be output to a gui log as well, later
    }	

	char hostname[255];
	
	if( gethostname ( hostname, sizeof(hostname)) == 0)
	{
		TRACE("Host name: %s\n", hostname);
		hostent* hostinfo=NULL;
		if((hostinfo = gethostbyname(hostname)) != NULL)
		{
			if(hostinfo->h_addr_list[0])
			{
				m_local_ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]);  //pull off the first ip, hopefully it is the only one and the one the user may be interested in.
			}
		}
	}
	*/
}

FileTransferService::~FileTransferService(void)
{
	for(UINT i=0;i<mv_send_jobs.Size();i++){
		FileTransferJob *ftj=(FileTransferJob *)mv_send_jobs.Get(i);
		ftj->StopThread();
	}

	CloseHandle(m_log_file_service);
}

//called to set the reference to the com link
void FileTransferService::SetComLink(ComLink* link)
{
	p_com_link=link;
}

//Called when a DLL sends data to this plugin.
//An app should only be making a Get Request
bool FileTransferService::ReceivedAppData( UINT from_app , byte* data )
{
	TKFileTransferInterface::IPInterface plugin_request;
	int stat=plugin_request.ReadFromBuffer(data);
	if(stat==0) //decided not to read any data because something is wrong with the request
		return false;

	//are they checking too see if this is a p2p compatible file transfer?
	if(plugin_request.m_type==plugin_request.P2PEXISTS)  //yes this is a p2p capable file transfer service
		return true;

	//the request must be either a p2p request or a p2pgetfile request
	if(plugin_request.m_type!=plugin_request.GETFILE && plugin_request.m_type!=plugin_request.P2PGETFILE)
		return false;

	//make sure the app gave us the required types of data
	if(plugin_request.v_ints.size()<1 || plugin_request.v_strings.size()<3)
		return false;

	//session id of the interface
	UINT session_id=(UINT)plugin_request.v_ints[0];

	//paths
	char* source=(char*)plugin_request.v_strings[0].c_str();
	char* local_path=(char*)plugin_request.v_strings[1].c_str();
	char* remote_path=(char*)plugin_request.v_strings[2].c_str();

	CString tmp=remote_path;
	tmp.MakeLower();

	if(stricmp(source,"p2p")==0){
		//create a new get request for this app.  This will put a new file into our file buffer.
		//the file buffer will then attempt to retrieve the file for all it is worth
		TRACE("TKFileTransfer FileTransferService::ReceivedAppData() App %d requested P2P file %s to save as %s.\n",from_app,remote_path,local_path);
		P2PGetFileJob *p2pgfr=new P2PGetFileJob(session_id,from_app,remote_path,local_path);
		mv_p2p_get_requests.Add(p2pgfr);
		mb_new_get_request=true;
		CString log_str;
		//log_str.Format("FileTransferService::ReceivedAppData() Received p2p get request for file %s with hash %s.",local_path,remote_path);
		//WriteToLogFile(log_str);
	}
	else{  //are they requesting a file??
		//create a new get request for this app.  (this causes an empty file to be requested etc.)
		GetFileRequest *gfr=new GetFileRequest(from_app,session_id,source,local_path,remote_path);
		mv_get_requests.Add(gfr);

		//send a request via com to this destination to initiate the transfer
		TKFileTransferInterface::IPInterface remote_request;

		remote_request.v_ints.push_back(gfr->GetJobIndex());
		remote_request.v_strings.push_back(string(remote_path));
		remote_request.m_type=remote_request.GETFILE;

		byte buf[4096];
		int nw=remote_request.WriteToBuffer(buf); //put this request into a byte buffer
		//send our request for initiating a send!
		ASSERT(nw<600);
		p_com_link->SendReliableData(source,buf,nw);
	}

	return true;
}

//Called by the com interface to notify us that we have a remote message
bool FileTransferService::ReceivedComData( char* source, byte* data , UINT data_size )
{
	if( data_size<6 || data[0]!=TKFILETRANSFERCODE ) //better be a message meant for us
		return false;
	TKFileTransferInterface::IPInterface request;

	ASSERT(data_size<(FILECHUNKSIZE+4096));  //we shouldn't be receiving data much larger than 32k
	int stat=request.ReadFromBuffer(data);
	if(stat==0)
		return false;

	CSingleLock lock(&m_data_lock,TRUE);  //do the standard delete lock.

	//someone sent us file data that needs to be written to file (for a GetRequest that we have)
	if(request.m_type==request.FILEDATA){
		if(request.v_ints.size()<3 )
			return false;
		UINT job_index=request.v_ints[0]; //a job index we must have gave the other trapper keeper
		UINT data_transferred=request.v_ints[1];  //data that has been transferred so far
		UINT data_total=request.v_ints[2];  //size of the file
		
		UINT nlen;
		byte *file_data=request.GetData(nlen);  //get a pointer to the file data

		//figure out which GetRequest this thing is referencing
		for(UINT i=0;i<mv_get_requests.Size();i++){
			GetFileRequest* gfr=(GetFileRequest*)mv_get_requests.Get(i);
			if(gfr->GetJobIndex()==job_index){  //is this the right job?

				if(!gfr->Process(file_data,nlen)){  //if it is the right job, write the data to file
					//we must have failed to write the data to file
					//if we aren't able to write this out, then we shouldn't even respond to the source to send us more.  Just let it timeout and die on the sources side, its more efficient than wasting the bandwidth and connections for a failed response.
					//We should however notify the app that it failed and why.

					//CString log_msg;
					//log_msg.Format("FAILED:  RECEIVED FILE DATA for file %s from %s, but couldn't write the data to the file.  %.2f percent complete.\n",gfr->GetLocalPath(),source,100.0f*(float)((float)data_transferred/(float)max(1,data_total)));
					g_file_transfer_dialog.GetFailed(gfr->GetLogID());
					//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);

					string reason="unable to write data to file";

					TKFileTransferInterface::IPInterface failed_notification;
					failed_notification.m_type=failed_notification.FAILED;
					failed_notification.v_strings.push_back(string(gfr->GetSource()));
					failed_notification.v_strings.push_back(string(gfr->GetLocalPath()));
					failed_notification.v_strings.push_back(string(gfr->GetRemotePath()));
					failed_notification.v_strings.push_back(reason);
					failed_notification.v_ints.push_back(gfr->GetInterfaceSessionID());

					byte buf[4096];
					failed_notification.WriteToBuffer(buf);

					AppID app;
					app.m_app_id=gfr->GetPluginID();
					DllInterface::SendData(app,buf);
		
					//remove it from existence.
					TRACE("FILE TRANSFER SERVICE:  Unable to write data to file %s from %s.\n",gfr->GetLocalPath(),gfr->GetSource());
					mv_get_requests.Remove(i);
					break;
				} 

				//at this point we have successfully written data to file
				//SEND back a response saying we got it
				TKFileTransferInterface::IPInterface com_response;
				com_response.m_type=com_response.FILEDATARECEIVED;
				com_response.v_ints.push_back(job_index);

				byte buf[4096];
				int nw=com_response.WriteToBuffer(buf);
				ASSERT(nw<4096);
				p_com_link->SendReliableData(source,buf,nw);
				
				//do a progress notification to the client
				TKFileTransferInterface::IPInterface progress;
				progress.m_type=progress.PROGRESS;
				float fprogress=(float)((float)data_transferred/(float)max(1,data_total));
				progress.v_floats.push_back(fprogress);
				progress.v_ints.push_back(gfr->GetInterfaceSessionID());
				progress.v_strings.push_back(string(gfr->GetSource()));
				progress.v_strings.push_back(string(gfr->GetLocalPath()));
				progress.v_strings.push_back(string(gfr->GetRemotePath()));

				//CString log_msg;
				//log_msg.Format("RECEIVED FILE DATA for file %s from %s.  %.2f percent complete.\n",gfr->GetLocalPath(),source,100.0f*(float)((float)data_transferred/(float)max(1,data_total)));
				//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
				g_file_transfer_dialog.GetProgress(gfr->GetLogID(),fprogress);
				nw=progress.WriteToBuffer(buf);
				AppID app;
				app.m_app_id=gfr->GetPluginID();
				DllInterface::SendData(app,buf);
				break;
			}
		}
		return true;
	}

	//Does someone wants us to send them a file?
	if(request.m_type==request.GETFILE){ 
		if(request.v_ints.size()<1 || request.v_strings.size()<1){
			return false;
		}
		
		int job_index=request.v_ints[0];
		string local_path=request.v_strings[0];
		
		//did this dest restart???
		for(UINT i=0;i<mv_send_jobs.Size();i++){
			FileTransferJob *job=(FileTransferJob*)mv_send_jobs.Get(i);
			if(job->IsJob(job_index,source)){
				mv_send_jobs.Remove(i);
				i--;
				break;
			}
		}

		if(this->mv_send_jobs.Size()>30){  //if too many requests, then ignore.
			return false;
		}

		//create a new job to handle sending then the file
		FileTransferJob *new_job=new FileTransferJob();
		char *error=new_job->Init(source,job_index,local_path);
		g_file_transfer_dialog.NewSend(local_path.c_str(),source,new_job->GetLogID());
		if(!error){
			g_file_transfer_dialog.AddSendJob(new_job,p_com_link); //queue up a send.  the sender will send data when resources are available
			mv_send_jobs.Add(new_job);  
			//CString log_msg;
			//log_msg.Format("Responding to a GET FILE Request from %s for file %s.\n",source,local_path.c_str());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			
		}
		else{
			//CString log_msg;
			//log_msg.Format("FAILED:  Couldn't process a GET FILE Request from %s for file %s.\n",source,local_path.c_str());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			g_file_transfer_dialog.SendFailed(new_job->GetLogID());
			delete new_job; //we didn't add this to a vector so we need to delete it
		}

		return true;
	}

	//Does someone want us to send them a file?
	if(request.m_type==request.RESUME){ 

		static Timer last_resume;  //a timer that only allows so many resumes so often as they are very hard on the system.
		if(request.v_ints.size()<3 || request.v_strings.size()<2){
			return false;
		}
		int old_index=request.v_ints[0];
		int new_index=request.v_ints[1];
		int offset=request.v_ints[2];
		string local_path=request.v_strings[0];
		string sha1=request.v_strings[1];

		if(offset==0)
			return false;

		if(this->mv_send_jobs.Size()>25){  //only allow resumes if there are open send slots (don't allow resume requests to block out requests for new files)
			return false;
		}

		if(!last_resume.HasTimedOut(60)){  //if there are too many requests, only allow resumes that have made progress to continue.
			return false;
		}

		
			
		//try to find this job
		for(UINT i=0;i<mv_send_jobs.Size();i++){
			FileTransferJob *job=(FileTransferJob*)mv_send_jobs.Get(i);
			if(job->IsJob(old_index,source)){
				char* error=job->Init(source,new_index,local_path,offset,sha1.c_str());
				if(error)
					return false;
				last_resume.Refresh();
				g_file_transfer_dialog.AddSendJob(job,p_com_link); //queue up a send.  the sender will send data when resources are available
				return true;
			}
		}

		//create a new job to handle sending the file
		FileTransferJob *new_job=new FileTransferJob();
		char *error=new_job->Init(source,new_index,local_path,offset,sha1.c_str());
		g_file_transfer_dialog.NewSend(local_path.c_str(),source,new_job->GetLogID());
		if(!error){
			last_resume.Refresh();
			g_file_transfer_dialog.AddSendJob(new_job,p_com_link); //queue up a send.  the sender will send data when resources are available
			mv_send_jobs.Add(new_job);  
			//CString log_msg;
			//log_msg.Format("Responding to a GET FILE Request from %s for file %s.\n",source,local_path.c_str());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			
		}
		else{
			//CString log_msg;
			//log_msg.Format("FAILED:  Couldn't process a GET FILE Request from %s for file %s.\n",source,local_path.c_str());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			g_file_transfer_dialog.SendFailed(new_job->GetLogID());
			delete new_job; //we didn't add this to a vector so we need to delete it
		}

		return true;
	}

	//Is a remote client ready for more data? (we must have sent them some)
	if(request.m_type==request.FILEDATARECEIVED){
		//CString log_msg;
		//log_msg.Format("Got file data received notification from %s.\n",source);
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
		if(request.v_ints.size()<1)
			return false;

		int job_index=request.v_ints[0];
		for(UINT i=0;i<mv_send_jobs.Size();i++){
			FileTransferJob* ftj=(FileTransferJob*)mv_send_jobs.Get(i);
			if(ftj->IsJob(job_index,source)){
				g_file_transfer_dialog.AddSendJob(ftj,p_com_link); //queue up a send.
				break;
			}
		}
	}

	//a remote source said they have finished sending us a file.  trigger a receivedfile event for the corresponding plugin.
	if(request.m_type==request.FILETRANSFERDONE){ 
		//TRACE("FILE TRANSFER SERVICE:  File Transfer Done.\n");
		//CString log_msg;
		//log_msg.Format("The source %s says that it is done sending a file to this trapper keeper.\n",source);
		//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);

		if(request.v_ints.size()<1)
			return false;
		UINT job_index=(UINT)request.v_ints[0];

		for(UINT i=0;i<mv_get_requests.Size();i++){  //find the correct job
			GetFileRequest* gfr=(GetFileRequest*)mv_get_requests.Get(i);
			if(gfr->GetJobIndex()==job_index){
				gfr->Finished(); //tell the job that it is finished
				TKFileTransferInterface::IPInterface notify;
				notify.m_type=notify.RECEIVEDFILE;
				notify.v_strings.push_back(string(gfr->GetSource()));
				notify.v_strings.push_back(string(gfr->GetLocalPath()));
				notify.v_strings.push_back(string(gfr->GetRemotePath()));
				notify.v_ints.push_back(gfr->GetInterfaceSessionID());	

				byte buf[4096];
				int nw=notify.WriteToBuffer(buf);

				AppID app;
				app.m_app_id=gfr->GetPluginID();
				DllInterface::SendData(app,buf);

				g_file_transfer_dialog.GetDone(gfr->GetLogID());
				mv_get_requests.Remove(i);  //this get request shall cease to exist.
				return true;
			}
		}
		return true;
	}
	
	//NEW P2P FILE TRANSFER SYSTEM

	if(request.m_type==request.P2PFILEPART){  //they sent us a file part
		//completed
		string hash=request.v_strings[0];
		int file_size=request.v_ints[0];
		int part=request.v_ints[1];
		int data_length=request.v_ints[2];
		//TRACE("TKFileTransfer FileTransferService::ReceivedComData() P2P %s sent part %d for file %s.\n",source,part,hash.c_str());
		UINT len;
		byte* buf=request.GetData(len);

		CString log_str;

		log_str.Format("FileTransferService::ReceivedComData() Recieved p2p file part for file %s, size %d, part %d, length %d from %s.",hash.c_str(),file_size,part,data_length,source);
		WriteToLogFile(log_str);

		if(m_file_buffer.WriteFilePart(hash.c_str(),file_size,part,buf,data_length,m_log_file_service)){
			TRACE("TKFileTransfer FileTransferServer::ReceivedComData() Successful received file part %d for file %s from %s.\n",part,hash.c_str(),source);
			mb_completed_file=true;
		}
	}
	else if(request.m_type==request.P2PNOSUCHFILE){  //they say they don't or no longer have a file part we asked for
		//string sha1=request.v_strings[0];

		//a host said they don't have a file with the specified sha1 (it probably got deleted along the way)
	}
	else if(request.m_type==request.P2PREQUESTFILESTATUS){  //they want to know how much of a file we have.
		//completed
		string sha1=request.v_strings[0];
		int size=request.v_ints[0];
		string completed;

		CString log_str;
		log_str.Format("FileTransferService::ReceivedComData() Received P2PREQUESTFILESTATUS for file %s with size %d.",sha1.c_str(),size);
		WriteToLogFile(log_str);

		//a host wants to know how much of a file we have, find it in our cache or deposit directory and respond here
		if(m_file_deposit.GetFileStatusAsString(sha1.c_str(),size,completed) || m_file_buffer.GetFileStatusAsString(sha1.c_str(),size,completed)){
			TKFileTransferInterface::IPInterface file_status;
			file_status.m_type=file_status.P2PFILESTATUS;
			file_status.v_ints.push_back(size);
			file_status.v_strings.push_back(sha1);
			file_status.v_strings.push_back(completed);

			byte *buf=new byte[8096];
			int nw=file_status.WriteToBuffer(buf);
			p_com_link->SendReliableData(source,buf,nw);  //let the com system worry about getting this data out to our destinations
			delete []buf;						
		}
		else{
			log_str.Format("FileTransferService::ReceivedComData() Failed to respond to P2PREQUESTFILESTATUS for file %s with size %d because we don't have that file.",sha1.c_str(),size);
			WriteToLogFile(log_str);
		}
	}
	else if(request.m_type==request.P2PFILEHOSTS){
		string sha1=request.v_strings[0];
		UINT file_size=(UINT)request.v_ints[0];
		TRACE("TKFileTransfer FileTransferService::ReceivedComData() %s sent %d hosts for file %s.\n",source,request.v_strings.size()-1,request.v_strings[0].c_str());

		//see if this is a source list for an uninited file job
		for(int i=0;i<(int)mv_p2p_get_requests.Size();i++){
			P2PGetFileJob *p2pgfj=(P2PGetFileJob*)mv_p2p_get_requests.Get(i);
			if(!p2pgfj->IsInited()){
				if(stricmp(p2pgfj->GetFileHash(),sha1.c_str())==0){
					TRACE("TKFileTransfer FileTransferService::ReceivedComData() %s p2p file is now inited.\n",p2pgfj->GetLocalPath());
					p2pgfj->Inited(file_size);
					if(!m_file_buffer.GetFile(sha1.c_str(),file_size)){
						if(!m_file_buffer.AddFile(sha1.c_str(),file_size,m_log_file_service)){
							CString log_str;

							log_str.Format("FileTransferService::ReceivedComData() Couldn't add file %s, size %d.",sha1.c_str(),file_size);
							WriteToLogFile(log_str);
							ASSERT(0);
						}
					}
					else{  //we already have the file in our buffer.  we need to signal a processing event
						mb_completed_file=true;
					}
				}
			}
		}

		TKFileTransferInterface::IPInterface status_request;
		status_request.m_type=status_request.P2PREQUESTFILESTATUS;
		status_request.v_strings.push_back(sha1);
		status_request.v_ints.push_back(file_size);
		byte buf[4096];
		int nw=status_request.WriteToBuffer(buf);
		request.v_strings.erase(request.v_strings.begin()+0);
		m_file_buffer.UpdateSources(sha1.c_str(),file_size,request.v_strings);
		//go through these hosts and request their status for this particular hash
		//for(int i=1;i<(int)request.v_strings.size();i++){
			//m_file_buffer.UpdateSource(
			//TRACE("TKFileTransfer FileTransferService::ReceivedComData() Asking %s for its P2P status of file %s.\n",(char*)request.v_strings[i].c_str(),request.v_strings[0].c_str());
		//	p_com_link->SendReliableData((char*)request.v_strings[i].c_str(),buf,nw);  //dispatch a status request to each of these sources
		//}
	}
	else if(request.m_type==request.P2PFILESTATUS){  //a host is responding with how much of a file they have
		//string sha1=request.v_strings[0];
		//int file_length=request.v_ints[0];  //the length of the file in case we don't know it yet.
		//TRACE("TKFileTransfer FileTransferService::ReceivedComData() Received P2P status on file %s with length %d from %s.\n",sha1.c_str(),file_length,source);
		//string completed=request.v_strings[1];
		
		//m_file_buffer.UpdateSource(sha1.c_str(),file_length,source,completed);

		//CString log_str;
		//log_str.Format("FileTransferService::ReceivedComData() Received file status from %s for file %s with size %d status was %s.",source,sha1.c_str(),file_length,completed.c_str());
		//WriteToLogFile(log_str);
	}
	else if(request.m_type==request.P2PREQUESTPART){  //they want a file part
		//completed
		string sha1=request.v_strings[0];
		int file_size=request.v_ints[0];
		int part=request.v_ints[1];  

		CString log_str;

		//TRACE("TKFileTransfer FileTransferService::ReceivedComData() P2P %s requested part %d for file %s.\n",source,part,sha1.c_str());
		if(mv_p2p_send_jobs.Size()<90){  //only enqueue this request if we are handling less than 20 requests already
			log_str.Format("FileTransferService::ReceivedComData() Got p2p file part request for file %s, part %d.",sha1.c_str(),part);
			//WriteToLogFile(log_str);

			P2PSendFileJob *p2psfj=new P2PSendFileJob(sha1.c_str(),file_size,source,part);
			mv_p2p_send_jobs.Add(p2psfj);  //queue up this send request
		}
		else{
			//log_str.Format("FileTransferService::ReceivedComData() Unable to process p2p part request for file %s, part %d.  We have too many send requests going already.",sha1.c_str(),part);
			//WriteToLogFile(log_str);
		}
	}
	else if(request.m_type==request.P2PSERVERADDRESS){  //an indexing server pinged us, flag it as the server to ask and upload file info to
		//completed
		m_server_address=source;
		//TRACE("TKFileTransfer Service FileTransferService::ReceivedComData() p2p file index server address = %s.\n",m_server_address.c_str());
	}
	
	return true;
}

void FileTransferService::Update(void)
{
//	WriteToLog("FileTransferService::Update() Phase 1.");
	CSingleLock lock(&m_data_lock,TRUE);  //synch because other threads are removing objects possibly, and so is this thread
	static Timer last_index_server_update;
	static Timer last_file_buffer_update;
	static Timer last_file_source_update;
	static Timer last_file_init_update;
	static Timer last_get_update;  //a timer to track how often we check on the get jobs
	static Timer last_send_update;  //a timer to track how often we check on the send jobs
	static bool has_done_source_update=false;

	srand(timeGetTime());
	//WriteToLog("FileTransferService::Update() Phase 2.");

	// ************** LEGACY FILE TRANSFER SERVICE CODE *********************
	//remove any send jobs that are dead or done
	for(UINT i=0;i<mv_send_jobs.Size();i++){
		FileTransferJob* ftj=(FileTransferJob*)mv_send_jobs.Get(i);
		if(ftj->IsDead() || ftj->IsDone()){  //check for any old transfers that timedout.
			if(ftj->IsDead()){
				TRACE("FILE TRANFSER SERVICE:  A file %s being sent to %s timed out.\n",ftj->GetLocalPath(),ftj->GetDest());
			}
			mv_send_jobs.Remove(i);
			i--;
		}
	}	
	//WriteToLog("FileTransferService::Update() Phase 3.");

	//remove any get jobs that are dead or done
	for(UINT i=0;i<mv_get_requests.Size();i++){
		GetFileRequest* gfr=(GetFileRequest*)mv_get_requests.Get(i);
		if(gfr->IsDead(p_com_link)){
			//WriteToLog("FileTransferService::Update() BEGIN removing old legacy get job because it is dead.");
			//notify the app that it failed.
			string reason="no response from source";

			TKFileTransferInterface::IPInterface notify;
			notify.m_type=notify.FAILED;
			notify.v_strings.push_back(string(gfr->GetSource()));
			notify.v_strings.push_back(string(gfr->GetLocalPath()));
			notify.v_strings.push_back(string(gfr->GetRemotePath()));
			notify.v_strings.push_back(reason);
			notify.v_ints.push_back(gfr->GetInterfaceSessionID());

			byte buf[4096];
			notify.WriteToBuffer(buf);

			AppID app;
			app.m_app_id=gfr->GetPluginID();  //app that this was for
			DllInterface::SendData(app,buf);

			//remove this get request from existence.
			TRACE("FILE TRANFSER SERVICE:  A file %s being sent from %s timed out, deleting the incomplete file.\n",gfr->GetLocalPath(),gfr->GetSource());
			mv_get_requests.Remove(i);
			//WriteToLog("FileTransferService::Update() END removing old legacy get job because it is dead.");
		}
	}
	// ***************** END LEGACY FILE TRANSFER SERVICE CODE *********************

	//upload file list to the index server
	if((m_server_address.length()>0) && (last_index_server_update.HasTimedOut(1000) || ((m_file_deposit.mb_need_file_update || m_file_buffer.mb_need_file_update) && last_index_server_update.HasTimedOut(110)))){
		//WriteToLog("FileTransferService::Update() BEGIN updating index server with filelist.");
		TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s STARTING.\n",m_server_address.c_str());
		m_file_deposit.mb_need_file_update=false;
		m_file_buffer.mb_need_file_update=false;

		vector<string> v_file_hashes;
		vector<int> v_file_sizes;
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s ENUMERATING FILE DEPOSIT.\n",m_server_address.c_str());
		m_file_deposit.EnumerateFiles(v_file_hashes,v_file_sizes);
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s ENUMERATING FILE BUFFER.\n",m_server_address.c_str());
		m_file_buffer.EnumerateFilesForIndexServer(v_file_hashes,v_file_sizes);

		//remove duplicates
		for(int file_index=0;file_index<(int)v_file_hashes.size();file_index++){
			for(int check_index=file_index+1;check_index<(int)v_file_hashes.size();check_index++){
				if(stricmp(v_file_hashes[file_index].c_str(),v_file_hashes[check_index].c_str())==0){
					v_file_hashes.erase(v_file_hashes.begin()+check_index);
					v_file_sizes.erase(v_file_sizes.begin()+check_index);
					check_index--;
					continue;
				}
			}
		}

		srand(timeGetTime());

		while((int)v_file_hashes.size()>9000){  //we need to prune this to a reasonable size
			int remove_index=rand()%(int)v_file_hashes.size();
			v_file_hashes.erase(v_file_hashes.begin()+remove_index);
			v_file_sizes.erase(v_file_sizes.begin()+remove_index);
		}

		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s DONE ENUMERATING.\n",m_server_address.c_str());
		TKFileTransferInterface::IPInterface file_update;  //a structure for the message
		TKFileTransferInterface::IPInterfaceDataSet file_list;  //a dataset capable of holding our list of files
		file_update.m_type=file_update.P2PFILELIST;

		int buffer_size=0;  //a variable to track how much room we need to allocate for our send buffer
		
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s BUILDING COM MESSAGE.\n",m_server_address.c_str());
		for(int i=0;i<(int)v_file_hashes.size();i++){
			file_list.v_strings.push_back(v_file_hashes[i]);
			buffer_size+=(int)(v_file_hashes[i].size()+1);
			file_list.v_ints.push_back(v_file_sizes[i]);
			buffer_size+=4;
		}
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s ALLOCATING COM MESSAGE OF LENGTH %d.\n",m_server_address.c_str(),buffer_size);

		byte *buf=new byte[buffer_size+16384];
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s WRITING MESSAGE TO SEND BUFFER.\n",m_server_address.c_str());
		int written_len=file_list.WriteToBuffer(buf);  //write the filelist out into a byte array

		file_update.SetData(buf,written_len);  //write that byte array into our message structure's data system
		byte *com_buf=new byte[written_len+32384];
		int com_buf_len=file_update.WriteToBuffer(com_buf);  //write our outgoing message into another byte array for the com system

		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s SENDING DATA.\n",m_server_address.c_str());
		p_com_link->SendReliableData((char*)m_server_address.c_str(),com_buf,com_buf_len);  //let the com system worry about getting this data out to our destinations
		//TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s DELETING SEND BUFFER.\n",m_server_address.c_str());
		delete []buf;
		delete []com_buf;
		TRACE("TKFileTransfer Service FileTransferService::Update() updating files on p2p index server with address %s FINISHED.\n",m_server_address.c_str());

		last_index_server_update.Refresh();
		//WriteToLog("FileTransferService::Update() END updating index server with filelist.");
	}

	//give the file buffer a chance to make requests etc.
	m_file_buffer.Update(p_com_link,m_log_file_service);
	last_file_buffer_update.Refresh();

	//every 10 minutes, or if a new get request has been added, see if we can't try to init it
	if(m_server_address.size()>0 && (last_file_init_update.HasTimedOut(400) || mb_new_get_request)){
		//WriteToLog("FileTransferService::Update() BEGIN processing uninited get requests.");
		mb_new_get_request=false;
		byte buf[4096];

		//ask for sources for our uninited get jobs
		for(int i=0;i<(int)mv_p2p_get_requests.Size();i++){
			P2PGetFileJob *p2pgfj=(P2PGetFileJob*)mv_p2p_get_requests.Get(i);
			if(!p2pgfj->IsInited()){
				TRACE("TKFileTransfer FileTransferService::Update() Requesting hosts for file %s.\n",p2pgfj->GetLocalPath());
				TKFileTransferInterface::IPInterface host_request;
				host_request.m_type=host_request.P2PGETFILEHOSTS;
				host_request.v_strings.push_back(string(p2pgfj->GetFileHash()));
				host_request.v_ints.push_back(-1);  //a special size that indicates that we don't know the size
				
				int nw=host_request.WriteToBuffer(buf);
				p_com_link->SendReliableData((char*)m_server_address.c_str(),buf,nw);  //let the com system worry about talking to the server
			}
		}

		last_file_init_update.Refresh();
		//WriteToLog("FileTransferService::Update() END processing uninited get requests.");
	}


	//every 10 minutes we will request sources for our incomplete files
	if(m_server_address.size()>0 && (last_file_source_update.HasTimedOut(600) || !has_done_source_update)){
		//WriteToLog("FileTransferService::Update() BEGIN processing inited get requests.");
		byte buf[4096];
		vector <string> v_file_hashes;
		vector <UINT> v_file_sizes;
		m_file_buffer.EnumerateIncompleteFiles(v_file_hashes,v_file_sizes);
		//choose up to 15 random files to get sources for
		int len=(int)v_file_hashes.size();

		//ask for sources for each of 15 random files
		for(int i=0;i<len && i<15;i++){
			int index=rand()%((int)v_file_hashes.size());
			TKFileTransferInterface::IPInterface host_request;
			host_request.m_type=host_request.P2PGETFILEHOSTS;
			host_request.v_strings.push_back(v_file_hashes[index]);
			host_request.v_ints.push_back(v_file_sizes[index]);

			int nw=host_request.WriteToBuffer(buf);
			p_com_link->SendReliableData((char*)m_server_address.c_str(),buf,nw);  //let the com system worry about talking to the server

			v_file_hashes.erase(v_file_hashes.begin()+index);  // this vector should be very small so theres not a ton of extra work to delete from it
			v_file_sizes.erase(v_file_sizes.begin()+index);
		}

		has_done_source_update=true;
		last_file_source_update.Refresh();
		//WriteToLog("FileTransferService::Update() END processing inited get requests.");
	}

	//check on our get requests
	if(last_get_update.HasTimedOut(500) || mb_completed_file){
		//WriteToLog("FileTransferService::Update() BEGIN processing get requests for completion testing.");
		mb_completed_file=false;
		//check and see if any are really old
		for(int i=0;i<(int)mv_p2p_get_requests.Size();i++){
			P2PGetFileJob *p2pgfj=(P2PGetFileJob*)mv_p2p_get_requests.Get(i);
			if(p2pgfj->IsOld()){  //if this file has been transferring for way to long, just give up on it.
				WriteToLogFile("FileTransferService::Update() BEGIN removing old p2p get job, notifying application of a failed request.");
				//notify the app that it failed.
				string reason="no response from source";

				TKFileTransferInterface::IPInterface notify;
				notify.m_type=notify.FAILED;
				notify.v_strings.push_back(string("p2p"));
				notify.v_strings.push_back(string(p2pgfj->GetLocalPath()));
				notify.v_strings.push_back(string(p2pgfj->GetFileHash()));
				notify.v_strings.push_back(reason);
				notify.v_ints.push_back(p2pgfj->GetSessionID());

				byte buf[4096];
				notify.WriteToBuffer(buf);

				AppID app;
				app.m_app_id=p2pgfj->GetRequesterID();  //app that this was for
				DllInterface::SendData(app,buf);

				mv_p2p_get_requests.Remove(i);
				i--;
				WriteToLogFile("FileTransferService::Update() END removing old p2p get job, notifying application of a failed request.");
			}
		}

		if(mv_p2p_get_requests.Size()>0){
			Vector v_complete_files;  //a vector of FileBufferFile objects
			m_file_buffer.EnumerateCompleteFiles(v_complete_files);
			for(int i=0;i<(int)mv_p2p_get_requests.Size();i++){
				P2PGetFileJob *p2pgfr=(P2PGetFileJob *)mv_p2p_get_requests.Get(i);
				if(p2pgfr->IsInited()){
					for(int j=0;j<(int)v_complete_files.Size();j++){  //browse through the complete files for each get job
						FileBufferFile* file=(FileBufferFile*)v_complete_files.Get(j);
						if(file->IsFile(p2pgfr->GetFileHash(),p2pgfr->GetFileSize())){
							WriteToLogFile("FileTransferService::Update() BEGIN processing application get job, writing file out.");
							TRACE("TKFileTransfer FileTransferService::Update() P2P File Transfer completed for file %s as %s.\n",p2pgfr->GetFileHash(),p2pgfr->GetLocalPath());
							//make a real file out of this buffered file
							if(	!m_file_buffer.ManifestOutOfBuffer(file,p2pgfr->GetLocalPath(),m_log_file_service ) ){
								WriteToLogFile("FileTransferService::Update() FAILED to manifest file out of buffer.");
								WriteToLogFile("FileTransferService::Update() END processing application get job, writing file out.  FAILED.");
								TRACE("TKFileTransfer FileTransferService::Update() P2P FAILED to Manifest completed file %s as %s.\n",p2pgfr->GetFileHash(),p2pgfr->GetLocalPath());
								continue;
							}
							
							//notify the dll that requested the job
							TKFileTransferInterface::IPInterface notify;
							notify.m_type=notify.P2PRECEIVEDFILE;
							notify.v_strings.push_back(string("unknown"));  //uhh this file probably came from many sources
							notify.v_strings.push_back(string(p2pgfr->GetLocalPath()));
							notify.v_strings.push_back(string(p2pgfr->GetFileHash()));
							notify.v_ints.push_back(p2pgfr->GetSessionID());	
							notify.v_ints.push_back(p2pgfr->GetFileSize());	

							byte buf[4096];
							int nw=notify.WriteToBuffer(buf);

							AppID app;
							app.m_app_id=p2pgfr->GetRequesterID();  //GetAppID is used by the dll interface, so thus GetRequesterID
							DllInterface::SendData(app,buf);						

							//remove the get job
							mv_p2p_get_requests.Remove(i);
							i--;
							WriteToLogFile("FileTransferService::Update() END processing application get job, writing file out.");
							break;
						}
					}
				}
			}
		}

		last_get_update.Refresh();
		//WriteToLog("FileTransferService::Update() END processing get requests for completion testing.");
	}

	for(int send_counter=0;send_counter<4;send_counter++){  //do 4 sends per cycle.  increase to improve sending performance.
		//check to see if there are any send requests, and handle one if there is
		if(mv_p2p_send_jobs.Size()>0){  
			WriteToLogFile("FileTransferService::Update() BEGIN processing send job.");
			//WriteToLog("FileTransferService::Update() BEGIN processing send job.");
			P2PSendFileJob *job=(P2PSendFileJob *)mv_p2p_send_jobs.Get(0);
			CString log_str;
			byte *part_buf=new byte[PARTSIZE];
			int nread=m_file_deposit.GetFilePart(job->GetHash(),job->GetFileSize(),part_buf,job->GetFilePart());
			if(nread==-1){
				nread=m_file_buffer.GetFilePart(job->GetHash(),job->GetFileSize(),part_buf,job->GetFilePart());
			}
			if(nread>=0){
				//send this data to the requester
				TKFileTransferInterface::IPInterface file_part;
				file_part.m_type=file_part.P2PFILEPART;
				file_part.v_strings.push_back(job->GetHash());
				file_part.v_ints.push_back(job->GetFileSize());
				file_part.v_ints.push_back(job->GetFilePart());
				file_part.v_ints.push_back(nread);
				file_part.SetData(part_buf,nread);

				byte *buf=new byte[nread+1024];
				int nw=file_part.WriteToBuffer(buf);
				p_com_link->SendReliableData((char*)job->GetDest(),buf,nw);  //let the com system worry about talking to the server
				log_str.Format("FileTransferService::ReceivedComData() Succeeded in processing a send request for file %s, part %d to dest %s.  Sending the part.",job->GetHash(),job->GetFilePart(),job->GetDest());
				TRACE("FileTransferService::ReceivedComData() Succeeded in processing a send request for file %s, part %d to dest %s.  Sending the part.\n",job->GetHash(),job->GetFilePart(),job->GetDest());
				//WriteToLogFile(log_str);
				delete []buf;
			}
			else{
				CString log_str;

				log_str.Format("FileTransferService::ReceivedComData() Failed to process a send request for file %s, part %d to dest %s, we don't have the file.",job->GetHash(),job->GetFilePart(),job->GetDest());
				//WriteToLogFile(log_str);
			}
			delete[] part_buf;

			mv_p2p_send_jobs.Remove(0);  //pop this request off the queue
			last_send_update.Refresh();
			WriteToLogFile("FileTransferService::Update() END processing send job.");
		}
	}

}

void FileTransferService::StartFileTransferService(void)
{
	m_file_deposit.StartThread();
}

void FileTransferService::StopFileTransferService(void)
{
	m_file_deposit.StopThread();
}

void FileTransferService::QueryIncompleteP2PJobs(vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_num_parts, vector <int> &v_completed_parts, vector<string> &v_sources)
{
	m_file_buffer.QueryIncompleteP2PJobs(v_hashes,v_size,v_num_parts,v_completed_parts,v_sources);
}

void FileTransferService::QuerySendJobs(vector <string> &v_dest,vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_part)
{
	for(int i=0;i<(int)mv_p2p_send_jobs.Size();i++){
		P2PSendFileJob *job=(P2PSendFileJob *)mv_p2p_send_jobs.Get(i);
		v_dest.push_back(string(job->GetDest()));
		v_hashes.push_back(string(job->GetHash()));
		v_size.push_back((int)job->GetFileSize());
		v_part.push_back((int)job->GetFilePart());
	}
}

string FileTransferService::GetP2PIndexServer(void)
{
	return m_server_address;
}

UINT FileTransferService::GetAllocatedBuffer(void)
{
	return m_file_buffer.GetAllocatedBuffer();
}

UINT FileTransferService::GetBufferFileCount(void)
{
	return m_file_buffer.GetFileCount();
}

void FileTransferService::WriteToLogFile(const char* str)
{
	static CCriticalSection m_log_lock;

	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%m/%d %I:%M %p"),str);
	DWORD tn;
	WriteFile(m_log_file_service,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}
