#pragma once
#include "..\tkcom\tkcominterface.h"
#include "..\tkcom\Timer.h"
#include "FileIndexSystem.h"
#include "..\tkcom\threadedobject.h"
#include <afxmt.h>

class P2PCom : public TKComInterface, public ThreadedObject
{
private:
	class SendJob : public Object
	{
	public:
		string m_hash;
		string m_dest;
		UINT m_size;
	};

	class CachedSource : public Object
	{
	public:
		string m_hash;
		UINT m_file_size;
		vector <string> mv_ips;
		Timer m_age;
		CachedSource(const char* hash,UINT file_size){
			m_file_size=file_size;
			m_hash=hash;
		}

		inline int CompareTo(Object *object){
			CachedSource *file=(CachedSource*)object;

			if(stricmp(file->m_hash.c_str(),this->m_hash.c_str())<0){
				return 1;
			}
			else if(stricmp(file->m_hash.c_str(),this->m_hash.c_str())>0){
				return -1;
			}
			else{
				return 0;
			}
		}

	};
	FileIndexSystem m_file_index_system;
	Vector mv_cached_sources;
	Vector mv_send_jobs;
	UINT Run();
	CCriticalSection m_lock;
public:
	P2PCom(void);
	~P2PCom(void);
	void DataReceived(char *source_ip, void *data, UINT data_length);
	void Update(void);
	P2PCom::CachedSource* GetCachedSource(const char* hash,UINT size);
};
