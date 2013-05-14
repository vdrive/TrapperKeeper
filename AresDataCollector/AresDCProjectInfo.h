#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Vector.h"
#include "AresSearchResult.h"
#include "AresDCTrackInfo.h"

class AresDCProjectInfo : public Object
{
public:
	string m_name;
	string m_owner;
	string m_artist;
	string m_search_string;
	string m_album;
	vector <string>mv_search_keywords;
	UINT m_size_threashold;
	bool mb_swarming;
	bool mb_decoying;
	UINT m_id;

	vector <string> mv_killwords;
	Vector mv_tracks;

	Vector mv_search_results;
	
	AresDCProjectInfo(const char* name, const char* search_str,const char* owner,const char* artist,UINT id,UINT size_threashold,const char* album);
	~AresDCProjectInfo(void);
	void AddTrack(const char* track_name, int track_number, bool single);
	void AddTrackKeyword(const char* keyword, int track_number);
	void AddKillword(const char* killword);

	// Compares this object to another.  Classes should override this function if they want sorting capability etc.
	int CompareTo(Object* object)
	{
		AresDCProjectInfo* p=(AresDCProjectInfo*)object;
		return stricmp(m_name.c_str(),p->m_name.c_str());
	}
	void AddSearchResult(AresSearchResult* result);

private:
	void ExtractSearchKeywords(void);
public:
	AresDCTrackInfo* DoesMatchSingle(const char* file_name, const char* artist, const char* title);
	bool DoesMatchKillwords(const char* file_name, const char* artist, const char* title);
};
