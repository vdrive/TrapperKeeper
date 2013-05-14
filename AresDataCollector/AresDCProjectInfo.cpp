#include "StdAfx.h"
#include "aresdcprojectinfo.h"
#include "AresDCTrackInfo.h"

AresDCProjectInfo::AresDCProjectInfo(const char* name, const char* search_str,const char* owner,const char* artist,UINT id,UINT size_threashold,const char* album)
{
	m_size_threashold=size_threashold;
	m_owner=owner;
	m_artist=artist;
	m_name=name;
	m_search_string=search_str;
	m_id=id;
	m_album=album;

	mb_swarming=false;
	mb_decoying=false;

	ExtractSearchKeywords();
}

AresDCProjectInfo::~AresDCProjectInfo(void)
{
}

void AresDCProjectInfo::AddTrack(const char* track_name, int track_number, bool single)
{
	AresDCTrackInfo *new_track=new AresDCTrackInfo(track_name,track_number,single);
	mv_tracks.Add(new_track);
}

void AresDCProjectInfo::AddTrackKeyword(const char* keyword, int track_number)
{
	CString tmp=keyword;
	tmp=tmp.MakeLower();
	for(int i=0;i<(int)mv_tracks.Size();i++){
		AresDCTrackInfo* track=(AresDCTrackInfo*)mv_tracks.Get(i);
		if(track->m_track_number==track_number){
			track->mv_keywords.push_back((LPCSTR)tmp);
			break;
		}
	}
}

void AresDCProjectInfo::AddKillword(const char* killword)
{
	CString tmp=killword;
	tmp=tmp.MakeLower();
	mv_killwords.push_back((LPCSTR)tmp);
}

void AresDCProjectInfo::AddSearchResult(AresSearchResult* result)
{
	mv_search_results.Add(result);
}

void AresDCProjectInfo::ExtractSearchKeywords(void)
{
	CString tmp=m_search_string.c_str();
	tmp=tmp.Trim();
	const char *str=m_search_string.c_str();
	int len=(int)m_search_string.size();

	mv_search_keywords.clear();

	string cur_keyword;
	for(int i=0;i<len;i++){
		if(str[i]<0 || (!isalpha((int)str[i]) && !isdigit(str[i]))){  //word delimiter
			if(cur_keyword.size()>1){
				mv_search_keywords.push_back(cur_keyword);
			}
			cur_keyword.clear();
		}
		else{ 
			cur_keyword+=tolower(str[i]);
		}
	}

	if(cur_keyword.size()>1){
		mv_search_keywords.push_back(cur_keyword);
	}
}

AresDCTrackInfo* AresDCProjectInfo::DoesMatchSingle(const char* file_name, const char* artist, const char* title)
{
	CString cfile_name=file_name;
	cfile_name=cfile_name.MakeLower();

	CString cartist=artist;
	cartist=cartist.MakeLower();

	CString ctitle=title;
	ctitle=ctitle.MakeLower();

	for(int i=0;i<(int)mv_tracks.Size();i++){
		AresDCTrackInfo *track=(AresDCTrackInfo*)mv_tracks.Get(i);
		if(!track->mb_single)
			continue;

		bool b_valid=true;
		for(int j=0;j<(int)track->mv_keywords.size();j++){
			bool b_found=false;
			if(strstr(cfile_name,track->mv_keywords[j].c_str())!=NULL)
				b_found=true;
			else if(strstr(ctitle,track->mv_keywords[j].c_str())!=NULL)
				b_found=true;
			else if(strstr(cartist,track->mv_keywords[j].c_str())!=NULL)
				b_found=true;

			if(!b_found){
				b_valid=false;
				break;
			}
		}

		if(b_valid)
			return track;
	}

	return NULL;
}

bool AresDCProjectInfo::DoesMatchKillwords(const char* file_name, const char* artist, const char* title)
{
	CString cfile_name=file_name;
	cfile_name=cfile_name.MakeLower();

	CString cartist=artist;
	cartist=cartist.MakeLower();

	CString ctitle=title;
	ctitle=ctitle.MakeLower();

	for(int i=0;i<(int)mv_killwords.size();i++){
		if(strstr(cartist,mv_killwords[i].c_str())!=NULL)
			return true;
		if(strstr(ctitle,mv_killwords[i].c_str())!=NULL)
			return true;
		if(strstr(cfile_name,mv_killwords[i].c_str())!=NULL)
			return true;
	}

	return false;
}
