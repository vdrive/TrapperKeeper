#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\TKComInterface.h"
#include "Utility.h"

#define PARTSIZE 500000

class FileBufferFile :	public Object
{
private:
	class FileSource : public Object{
		friend class P2PGetFileJob;
	private:
		string m_ip;

		//we used to query each source for its status, but it seems to be more efficient just to assume that it has a part, since its extremely likely that it will given the rate things spread here
		//vector <byte> mv_has_parts;
		//CTime m_last_part_update;
		CTime m_creation_time;
		
	public:
		FileSource(const char* ip){
			m_creation_time=CTime::GetCurrentTime();
			//m_last_part_update=CTime(1975,5,5,5,5,5);
			m_ip=ip;
		}

		//inline void UpdateParts(string &completed){
		//	Utility::DecodeCompletionFromString(completed,mv_has_parts);
		//	m_last_part_update=CTime::GetCurrentTime();
		//}

		//inline bool NeedsPartUpdate(){
		//	CTime cur_time=CTime::GetCurrentTime();
		//	if(m_last_part_update<(cur_time-CTimeSpan(0,1,0,0))){  //get a list of parts once an hour
		//		m_last_part_update=CTime::GetCurrentTime();
		//		return true;
		//	}
		//	else return false;
		//}

		//inline bool SourceHasPart(int part){
		//	//create our status reference info
		//	int byte_offset=part/8;
		//	int bit_offset=part%8;
		//	if(byte_offset<(int)mv_has_parts.size()){
		//		byte the_byte=mv_has_parts[byte_offset];
		//		int has_part=( the_byte & Utility::PowerOf2(bit_offset) );  //see if we have this part yet
		//		if(has_part)
		//			return true;
		//		else return false;
		//	}
		//	else
		//		return false;
		//}

		inline bool IsSource(const char* ip){
			if(stricmp(m_ip.c_str(),ip)==0)
				return true;
			else
				return false;
		}

		inline const char* GetIP(){
			return m_ip.c_str();
		}

		inline bool IsOld(){
			//if we haven't received a file parts update from this source lately, we can assume this source no longer has the file
			CTime cur_time=CTime::GetCurrentTime();
			if(m_creation_time<(cur_time-CTimeSpan(0,1,0,0)))
				return true;
			else return false;
		}

		inline void Refresh(){
			m_creation_time=CTime::GetCurrentTime();
		}
	};

	enum SORTMODE {BYPOSITION,BYTIME};
	UINT m_position;
	UINT m_total_length;
	CTime m_last_access_time;
	CTime m_creation_time;
//	CTime m_last_source_update;
//	int m_request_count;
	Timer m_request_count_timer;

	Timer m_previous_request_timer;
	vector <int>mv_previous_requests;

	string m_file_hash;
	vector <byte> mv_completed_parts;  //1 bit of each byte represents a 100k chunk
	int m_last_part_requested;
	bool mb_complete;
	static SORTMODE sm_sort_mode;
	Vector mv_sources;


public:
	FileBufferFile(const char* hash,UINT position,UINT file_length,CTime creation_time, CTime last_access_time);
	~FileBufferFile(void);
	bool IsComplete(bool b_initial,HANDLE buffer_file, HANDLE log_file);

	inline int CompareTo(Object *object){
		FileBufferFile *file=(FileBufferFile*)object;

		if(sm_sort_mode==FileBufferFile::BYTIME){  //sort by time last accessed
			if(file->m_last_access_time>this->m_last_access_time){
				return -1;
			}
			else if(file->m_last_access_time<this->m_last_access_time){
				return 1;
			}
			else{
				return 0;
			}
		}
		else{  //must be sorted by position
			if(file->m_position>m_position){
				return -1;
			}
			else if(file->m_position<m_position){
				return 1;
			}
			else{
				return 0;
			}
		}
	}

	inline UINT GetFileSize(){
		return m_total_length;
	}

	inline const char* GetFileHash(){
		return m_file_hash.c_str();
	}

	inline UINT GetFilePosition(void)
	{
		return m_position;
	}

	inline UINT GetCreatedYear(){return m_creation_time.GetYear();}
	inline UINT GetCreatedMonth(){return m_creation_time.GetMonth();}
	inline UINT GetCreatedDay(){return m_creation_time.GetDay();}
	inline UINT GetCreatedHour(){return m_creation_time.GetHour();}
	inline UINT GetCreatedMinute(){return m_creation_time.GetMinute();}
	inline UINT GetCreatedSecond(){return m_creation_time.GetSecond();}

	inline UINT GetLastAccessYear(){return m_last_access_time.GetYear();}
	inline UINT GetLastAccessMonth(){return m_last_access_time.GetMonth();}
	inline UINT GetLastAccessDay(){return m_last_access_time.GetDay();}
	inline UINT GetLastAccessHour(){return m_last_access_time.GetHour();}
	inline UINT GetLastAccessMinute(){return m_last_access_time.GetMinute();}
	inline UINT GetLastAccessSecond(){return m_last_access_time.GetSecond();}
	inline CString GetLastAccessTime(){return m_last_access_time.Format("%Y.%m.%d.%H.%M.%s");}

	static void SortByPosition(void);
	static void SortByTime(void);
	bool MoveFile(HANDLE file, UINT new_position,HANDLE log_file);
	bool DecodeCompletionFromString(string str);
	string EncodeCompletionToString(void);
	int GetFilePart(HANDLE file,byte* buffer,UINT part);

	inline void Hit(){
		m_last_access_time=CTime::GetCurrentTime();
	}

	inline bool IsFile(const char* hash,UINT file_size)
	{
		if(stricmp(m_file_hash.c_str(),hash)==0 && file_size==m_total_length)
			return true;
		else
			return false;
	}
	bool WriteFilePart(HANDLE file,UINT part, byte* buffer, UINT data_length,HANDLE log_file);
	void Update(void);
	void UpdateSource(const char* source_ip);
	void MakePartRequest(TKComInterface* p_com, HANDLE log_file);
	void GetIncompleteParts(vector<int> &v_parts);
	bool ManifestOutOfBuffer(HANDLE buffer_file,const char* full_path,HANDLE log_file);
	bool IsBad(void);
	void WriteToLogFile(HANDLE file, const char* str);
	string GetSources(void);
	bool HasMinimumPartsForIndexServer(void);
private:
	void ReceivedPart(int part);
public:
	bool ValidateFile(HANDLE buffer_file,HANDLE log_file);
	bool IsComplete2(void);
};
