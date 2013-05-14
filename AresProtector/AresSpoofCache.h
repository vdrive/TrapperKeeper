#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Timer.h"
#include <afxmt.h>

class AresSpoofCache : public ThreadedObject
{
	class DecoyObject: public Object{
	public:
		string m_project;
		int m_track;
		string m_file_name;
		int m_size;
		DecoyObject(const char* project, const char* file_name,int track,int size){
			m_project=project;
			m_file_name=file_name;
			m_track=track;
			m_size=size;
		}
	};
	/*
	class FileHashObject : public Object{
	public:
		byte m_hash[20];
		FileHashObject(){
			for(int i=0;i<20;i++){m_hash[i]=rand()&255;}
		}
	};*/

private:

	bool mb_ready;
	//Vector mv_ares_files;
	int SumString(const char* str);

	byte ConvertCharToInt(char ch)
	{
		switch(ch){
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'A': return 10;
			case 'a': return 10;
			case 'B': return 11;
			case 'b': return 11;
			case 'C': return 12;
			case 'c': return 12;
			case 'D': return 13;
			case 'd': return 13;
			case 'E': return 14;
			case 'e': return 14;
			case 'F': return 15;
			case 'f': return 15;
		}
		return 0;
	}

public:
	
	Vector mv_swarm_file_array[256];
	Vector mv_file_groups;
	CCriticalSection m_lock;

	AresSpoofCache(void);
	~AresSpoofCache(void);
	UINT Run();
	bool IsReady(void);
	unsigned int GetSpoofCount(void);
	int GetFileLength(byte* hash);
	void AddDecoyToList(Vector& v_tmp, const char* file_name, const char* project,int file_size,int hash_offset,int set_modifier);
	void DoSpecialMovie(const char* project, vector<string>& v_file_names, vector<string>& v_titles, vector<string>& v_authors,int seed,Vector &v_tmp_vector);
};
