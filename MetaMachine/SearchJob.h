#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"
#include "SearchResult.h"
#include "TinySQL.h"
#include <afxmt.h>

class SearchJob :	public Object
{
	//tracks belonging to this job
	class Track: public Object
	{
	private:
		friend class SearchJob;
		vector <string> mv_keywords;
		vector <string> mv_killwords;
		Vector	mv_search_results;	//results this job has found.
		string m_name;
		int m_track_index;
	public:

		bool CheckResult(const char* file_name)
		{
			if(mv_keywords.size()==0)
				return true;

			for(UINT i=0;i<mv_keywords.size();i++){
				if(mv_keywords[i].size()<1)
					continue;
				CString ref=mv_keywords[i].c_str();
				CString query=file_name;
				ref.MakeLower();
				query.MakeLower();
				if(query.Find(ref)==-1)  //does the file_name contain a keyword
					return false;
			}

			for(UINT i=0;i<mv_killwords.size();i++){
				if(mv_killwords[i].size()<1)
					continue;
				CString ref=mv_killwords[i].c_str();
				CString query=file_name;
				ref.MakeLower();
				query.MakeLower();
				if(query.Find(ref)!=-1)  //does the file_name contain a kill word?
					return false;
			}

			return true;
		}

		Track(){}
		Track(Track* track){
			m_name=track->m_name;
			m_track_index=track->m_track_index;
			for(UINT i=0;i<track->mv_keywords.size();i++){
				mv_keywords.push_back(track->mv_keywords[i]);
			}
			for(UINT i=0;i<track->mv_killwords.size();i++){
				mv_killwords.push_back(track->mv_killwords[i]);
			}
		}
	};

private:
	//UINT m_job_id;			//this jobs unique id
	string m_search_str;	//the string this job uses when searching
	string m_project;		//project this job is for
	UINT m_min_size;
	Vector mv_tracks;
	CCriticalSection m_lock;
	Timer m_start;

public:
	inline const char* GetProject(){return m_project.c_str();}
	//inline UINT GetJobID(){return m_job_id;}
	inline const char* GetSearchString(){return m_search_str.c_str();}
	//inline Vector* GetResults(){return &mv_search_results;}
public:
	
	//UINT CountSources();
	SearchJob(const char *search_str,string project,UINT min_size);
	~SearchJob(void);
	void AddIP(UINT ip, SearchResult *sr,int multiple);
	void AddKillwords(vector <char *> &v_killwords,const char* track_name);
	bool IsJob(const char* project_name, const char* search_str);
	void AddTrack(const char* name,int index, vector <char*>&v_keywords);
	bool AddResult(SearchResult *result);

	bool IsClosed(void)
	{
		if(m_start.HasTimedOut(60*60*3))
			return true;
		else
			return false;
	}
	SearchJob(SearchJob* job);
	void LogTracks(void);
};
