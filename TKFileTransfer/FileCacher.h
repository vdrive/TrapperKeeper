#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\timer.h"
#include <afxmt.h>

class FileCacher :	public ThreadedObject
{
public:
	class FileSegment : public Object
	{
	public:
		Timer m_cache_time;
		Timer m_last_hit_time;
		string m_file_name;
		byte *m_data;
		UINT m_length;
		UINT m_file_offset;
		FileSegment(const char *name, UINT offset, byte* data, UINT length){
			m_data=data;
			m_length=length;
			//m_data=new byte[length];
			//memcpy(m_data,data,length);
			//m_length=length;
			m_file_name=name;
			m_file_offset=offset;
		}
		inline bool IsOld(){
			if(m_last_hit_time.HasTimedOut(5*60)){  //if noone has requested this file for 5 minutes
				return true;
			}	
			else if(m_cache_time.HasTimedOut(30*60))  //cache stuff for 30 minutes no matter the activity, then let it go.
				return true;
			else return false;
		}
		~FileSegment(){
			if(m_data){
				delete m_data;
				m_data=NULL;
			}
		}

		inline bool IsSegment(const char *name,UINT offset){
			if(stricmp(m_file_name.c_str(),name)!=0)
				return false;
			if(m_file_offset!=offset)
				return false;
			return true;
		}

		inline void Hit(){
			m_last_hit_time.Refresh();
		}	
		
	};
private:
	Vector mv_file_segments;
	CCriticalSection m_data_lock;
	UINT Run();
public:
	FileCacher(void);
	~FileCacher(void);
	// Caches a piece of a file
	void AddSegment(FileCacher::FileSegment *file_segment);
	bool GetSegment(const char* file_name, UINT offset,Vector &v_buffer);
	UINT GetCount(void);
};
