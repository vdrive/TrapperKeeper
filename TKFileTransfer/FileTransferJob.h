//AUTHOR:  Ty Heath
//Last Modified:  4.8.2003

#pragma once
#include "..\tkcom\threadedobject.h"
#include "ComLink.h"
#include <Mmsystem.h>
#include "Timer.h"
class ComLink;

#define FILECHUNKSIZE (1000000)  //size of each peice of file we want to send.
class FileTransferJob : public ThreadedObject
{

	//INTERNAL STATE ATTRIBUTES
private:
	UINT m_log_id; //an id used for the dialog's logging purposed
	UINT m_job_index; //a unique job index (this is the job index of a destination GetFileRequest object)
	UINT m_transferred; //bytes that have been sent
	UINT m_total; //total bytes to be sent
	HANDLE m_file; //handle to our file that we are sending data from
	string m_dest;	//destination we are sending data to
	string m_local_path; //path of the file we are sending, we keep this for logging
	bool mb_inited;  //A call to init has been made. 
	bool mb_done;  //a flag that gets set when the transfer has been completed
	bool mb_done_notified;
	bool mb_data_ready;
	bool mb_data_loading;
	bool mb_data_error;
	Vector mv_data_buffer;
	//byte *m_data_buffer;
	//DWORD m_data_buffer_length;
	Timer m_last_used; //time we last sent data to the destination.  (to track timeouts)
public:

	//inline access operators.  There is no function overhead this way.  It is good for functions with little code in them.
	inline const char* GetDest(){return (char*)m_dest.c_str();}
	inline const char* GetLocalPath(){return (char*)m_local_path.c_str();}
	inline UINT GetLogID(){ return m_log_id;}
	//call to see if we are done sending and this object can be cleaned up
	inline bool IsDone(void){return mb_done_notified;}

	FileTransferJob(void);
	~FileTransferJob(void);

	//called to begin a send
	char* Init(char *dest,UINT job_index, string local_path,UINT offset=0,const char *pcstr_sha1=NULL);

	//called to send more data
	void Process(ComLink* plink);

	//called to see if this is a specific job
	bool IsJob(UINT job_index , char* peer);

	virtual UINT Run();
	
	//call to see if there has been any activity
	bool IsDead(void);

	
	void BeginLoad(void);
	bool IsLoading(void);
	bool IsDoneLoading(void);
	void Reset(void);
	void ResetLoad(void);
};
