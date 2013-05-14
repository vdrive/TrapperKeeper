#include "StdAfx.h"
#include "searchjob.h"
#include "MetaSystem.h"
#include "MetaMachineDLL.h"

//UINT job_index=0;

SearchJob::SearchJob(const char *search_str,string project,UINT min_size)
{
	//m_job_id=job_index++;
	m_search_str=search_str;

	m_project=project;
	m_min_size=min_size;
}

SearchJob::~SearchJob(void)
{
}

CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
{
	CString base16_buff;
	static byte base16_alphabet[] = "0123456789ABCDEF";

	for(unsigned int i = 0; i < bufLen; i++) {
		base16_buff += base16_alphabet[buffer[i] >> 4];
		base16_buff += base16_alphabet[buffer[i] & 0xf];
	}

    return base16_buff;
}


void SearchJob::AddIP(UINT ip, SearchResult *sr,int multiple)
{
	CSingleLock sl(&m_lock,TRUE);
	MetaSystemReference system;
	CString shash=EncodeBase16(sr->m_hash,16);
	CString file_name=sr->m_file_name.c_str();
	file_name.Replace("'","\\'");
	CString query="INSERT INTO rawsupply VALUES ";
	int insert_count=0;

	for(int mult_count=0;mult_count<multiple;mult_count++){
		for(UINT j=0;j<mv_tracks.Size();j++){
			SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
			int index=track->mv_search_results.GetIndex(sr);
			if(index>=0){
				//sr->AddSource(ip);  //enable this to enable ip recording.  some other restructuring needs to be done on the donkey server side.
				CString tmp;
				//CString track_name=track->m_name.c_str();
				//int track_index=track->m_index;
//				tmp.Format("Track Index=%d  track name= %s   search result index=%d", track->m_track_index,track->m_name.c_str(),index);
//				MetaMachineDll::GetDlg()->WriteToLogFile(tmp);
				//track_name.Replace("'","\\'");
				//tmp.Format("INSERT INTO RawSupply VALUES ('%s',%d,now(),%d,%d,'%s','%s')",this->m_project.c_str(),track_index,ip,sr->m_size,(LPCSTR)shash,(LPCSTR)file_name);
				if(insert_count>0){
					tmp.Format(",('%s',%d,now(),%d,%d,'%s','%s')",this->m_project.c_str(),track->m_track_index,ip,sr->m_size,(LPCSTR)shash,(LPCSTR)file_name);
				}
				else{
					tmp.Format("('%s',%d,now(),%d,%d,'%s','%s')",this->m_project.c_str(),track->m_track_index,ip,sr->m_size,(LPCSTR)shash,(LPCSTR)file_name);
				}
				query+=tmp;
				insert_count++;
			}
		}
	}

	if(insert_count>0){
		system.System()->AddQuery((LPCSTR)query);
	}
}

void SearchJob::AddKillwords(vector <char *> &v_killwords,const char* track_name)
{
	CSingleLock sl(&m_lock,TRUE);
	for(UINT j=0;j<mv_tracks.Size();j++){
		SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
		if(track_name==NULL || strlen(track_name)==0){
			for(UINT i=0;i<v_killwords.size();i++){
				track->mv_killwords.push_back(string(v_killwords[i]));
			}		
		}
		else if(stricmp(track->m_name.c_str(),track_name)==0){
			for(UINT i=0;i<v_killwords.size();i++){
				track->mv_killwords.push_back(string(v_killwords[i]));
			}
		}
	}
}

bool SearchJob::IsJob(const char* project_name, const char* search_str)
{
	if(stricmp(m_project.c_str(),project_name)!=0)
		return false;
//	bool b_has_track=false;
//	for(UINT j=0;j<mv_tracks.Size();j++){
//		SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
//		if(stricmp(track->m_name.c_str(),track_name)==0){
//			b_has_track=true;
//			break;
//		}
//	}
	if(stricmp(search_str,m_search_str.c_str())!=0)
		return false;
//	return b_has_track;
	return true;
}

void SearchJob::AddTrack(const char* name,int track_index, vector <char*> &v_keywords)
{
	CString tmp;
	tmp.Format("Adding track: %s - %s - %d keyword length=%d", m_project.c_str(),name,track_index,v_keywords.size());
	MetaMachineDll::GetDlg()->WriteToLogFile(tmp);
	CSingleLock sl(&m_lock,TRUE);
	SearchJob::Track *new_track=new SearchJob::Track();
	new_track->m_name=name;
	new_track->m_track_index=track_index;
	for(UINT i=0;i<v_keywords.size();i++){
		new_track->mv_keywords.push_back(string(v_keywords[i]));
	}
	mv_tracks.Add(new_track);
}

bool SearchJob::AddResult(SearchResult *result)
{
	CSingleLock sl(&m_lock,TRUE);
	bool stat=false;
	if(result->m_size<m_min_size){
		return false;
	}
	for(UINT j=0;j<mv_tracks.Size();j++){
		SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
		if(track->CheckResult(result->m_file_name.c_str())){
			stat=true;  //we were able to add this result to at least 1 track.
			track->mv_search_results.Add(result);
		}
	}
	return stat;
}

/*
UINT SearchJob::CountSources(){
	CSingleLock sl(&m_lock,TRUE);
	UINT sum=0;
	for(UINT j=0;j<mv_tracks.Size();j++){
		SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
		UINT len=track->mv_search_results.Size();
		for(UINT i=0;i<len;i++){
			SearchResult *sr=(SearchResult*)track->mv_search_results.Get(i);
			sum+=(UINT)sr->mv_ips.Size();
		}
	}
	return sum;
}*/

SearchJob::SearchJob(SearchJob* job)
{
	m_min_size=job->m_min_size;
	m_project=job->m_project;
	m_search_str=job->m_search_str;
	for(UINT i=0;i<job->mv_tracks.Size();i++){
		Track *nt=new Track((Track*)job->mv_tracks.Get(i));
		mv_tracks.Add(nt);
	}
}

void SearchJob::LogTracks(void)
{
	for(UINT j=0;j<mv_tracks.Size();j++){
		SearchJob::Track *track=(SearchJob::Track*)mv_tracks.Get(j);
		CString tmp;
		//CString track_name=track->m_name.c_str();
		//int track_index=track->m_index;
		tmp.Format("Track Listing:  Project %s Track %s - %d",this->m_project.c_str(), track->m_name.c_str(),track->m_track_index);
		MetaMachineDll::GetDlg()->WriteToLogFile(tmp);
	}
}
