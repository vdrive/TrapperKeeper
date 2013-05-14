//AUTHOR:  Ty Heath
//Last Modified:  4.8.2003

#pragma once
#include "object.h"
#include "Timer.h"
#include "ComLink.h"


//This class encapsulates a clients request for a file from a source.  It handles creating a file and writing any data out that it receives in regard to a file
class GetFileRequest :
	public Object
{
private:
	//a static counter that increments with each new GetFileRequest and gives this object a unique ID
	static UINT m_job_counter;

	//called internally when things go badly and its
	void CleanUpFailure(void);

	//the m_from_app and m_session_id can uniquely identify a TKFileTransferInterface object in any plugin for notification purposes
	UINT m_session_id;
	UINT m_from_app;
	UINT m_retry;
	UINT m_written;

	UINT m_log_id;  //an id used for the dialogs tracking purposed;

	//a unique ID for this object.  a source responds to this trapper keeper with this ID
	UINT m_job_index;

	//file handle to write data to
	HANDLE m_file;

	//local path of where the file will be saves
	string m_local_path;
	string m_tmp_path;

	//source path of the file we want
	string m_remote_path;

	//The IP of the source
	string m_source;

	//Time we last requested a file
	Timer m_last_update;

public: //inline access functions for the above attributes.  There is no function overhead this way.  It is good for functions with little code in them.
	inline char* GetLocalPath(){return (char*)m_local_path.c_str();}
	inline char* GetRemotePath(){return (char*)m_remote_path.c_str();}
	inline char* GetSource(){return (char*)m_source.c_str();}
	inline UINT GetPluginID(){return m_from_app;}
	inline UINT GetInterfaceSessionID(){return m_session_id;}
	inline UINT GetJobIndex(){return m_job_index;}
	inline UINT GetLogID(){ return m_log_id;}

	//constructor creates an empty file specified by local path and assigned the m_job_index
	GetFileRequest(UINT from_app, UINT session_id, char* source, char* local_path, char* remote_path);

	//closes a file and cleans
	~GetFileRequest(void);

	//called to determine if this job hasn't had any activity in x minutes
	bool IsDead(ComLink *p_com_link);

	//call to write data into a file.  called from FileTransferService::ReceivedComData
	bool Process(byte* data, UINT data_size);

	//call to indicate that the source says it is done sending a file to us.
	void Finished(void);
};
