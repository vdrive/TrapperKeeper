#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Timer.h"

class P2PGetFileJob :	public Object
{

private:
	//FileDeposit *mp_file_deposit;  //a pointer to the file deposit
	//FileBuffer *mp_file_buffer;  //a pointer to the file buffer
	string m_local_path;
	string m_file_hash;  //the hash that this get job is trying to get
	UINT m_file_size;  //corresponding size to ensure a match
	UINT m_session_id;
	UINT m_app_id;
	bool mb_inited;  //have we received size information about this file yet?
	Timer m_created;
	//Timer m_last_request;
public:
	inline bool IsOld(){
		if(m_created.HasTimedOut(60*60*24))
		{
			return true;
		}
		else return false;
	}
	inline UINT GetSessionID(){return m_session_id;}
	inline UINT GetRequesterID(){return m_app_id;}
	inline const char* GetFileHash(){return m_file_hash.c_str();}
	inline const char* GetLocalPath(){return m_local_path.c_str();}
	inline UINT GetFileSize(){return m_file_size;}
	P2PGetFileJob(UINT session_id,UINT app_id,const char* hash,const char* local_path);
	~P2PGetFileJob(void);
	void Inited(UINT file_size);
	bool IsInited(void);
};
