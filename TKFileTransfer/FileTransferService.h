//AUTHOR:  Ty Heath
//Last Modified:  4.8.2003

#pragma once
#include "threadedobject.h"
#include "Vector.h"
#include "ComLink.h"
#include "TKFileTransferInterface.h"
#include "FileBuffer.h"
#include "FileDeposit.h"
class ComLink;


//#include "TKFileTransferDll.h"

//TODO:
//1.  Change the GetFileRequest::Process() and the FileTransferJob::SendMore() to a threaded handler since disk i/o is slow and blocking
//2.  Make sure the service fault tolerance and duplicate/overlapping transfer request handling are coded properly
//3.  Add more logging to all aspects of the service.
//4.  Add a nice GUI to view current transfers requested by apps.
//5.  Add more useful failure notification reasons to apps.


//FUNCTIONALITY:
//1.  A trapper keeper must make a GetFile(...) request for a file to be able to be written to it.

class FileTransferService : protected  Object
{
private:
	//this object handles and throttles file requests
//	SendManager *p_send_manager;
	//string m_local_ip;
	bool mb_new_get_request;
	bool mb_completed_file;
	HANDLE m_log_file_service;

	string m_server_address;
	//string m_database_host;
	//string m_database_username;
	//string m_database_password;

	//thread safe
	CCriticalSection m_data_lock;
	CCriticalSection m_delayed_load_lock;
	FileBuffer m_file_buffer;
	FileDeposit m_file_deposit;

	Vector mv_get_requests; //files this service is requesting from foreign trapper keepers
	Vector mv_send_jobs; //files this service is sending to foreign trapper keepers
	
	Vector mv_p2p_get_requests; //files this service is requesting from foreign trapper keepers
	Vector mv_p2p_send_jobs; //files this service is sending to foreign trapper keepers

public:
	FileTransferService(void);
	~FileTransferService(void);
	//UINT Run(); //worker thread
	ComLink *p_com_link; //link to the com service
	void SetComLink(ComLink* link);  //called to set the com service link
	bool ReceivedAppData( UINT from_app , byte* data );  //called by a FileTransferInterface to get a file
	bool ReceivedComData( char *source , byte* data , UINT data_size );  //called by our com link when we have received com data
	void Update(void);
	void StartFileTransferService(void);
	void StopFileTransferService(void);
	void QueryIncompleteP2PJobs(vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_num_parts, vector <int> &v_completed_parts, vector<string> &v_sources);
	void QuerySendJobs(vector <string> &v_dest,vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_part);
	string GetP2PIndexServer(void);
	UINT GetAllocatedBuffer(void);
	UINT GetBufferFileCount(void);
	void WriteToLogFile(const char* str);
};
