#pragma once
#include "..\tkcom\object.h"

class AresSNSpoof : public Object
{
public:
	string m_project;
	UINT m_track;
	UINT m_popularity;
	string m_filename;
	string m_title;
	string m_artist;
	string m_album;
	string m_genre;
	string m_comments;
	UINT m_bitrate;
	UINT m_media_length;
	UINT m_resolution1;
	UINT m_resolution2;
	UINT m_search_type;
	UINT m_size;

	byte m_clump_hash[20];
	vector<string> mv_keywords;
	
	string m_file_extension;
	string m_language;
	string m_category;
	bool mb_valid;

private:
	void AddKeyword(vector <string>&v, string& str);
	void ExtractKeywords(const char* str,vector <string> &v_keywords);
	void CreateKeywords();
	void CalculateExtension();
	bool IsValidSpecialChar(char ch);
public:

	//big contructor
	AresSNSpoof(const char* project, UINT track, const char* filename, const char* title, const char* artist, 
								   const char* album, const char* genre, const char* comments, UINT bitrate, UINT media_length, UINT size,UINT popularity);
	~AresSNSpoof(void);


	inline int CompareTo(Object *object){
		AresSNSpoof *as=(AresSNSpoof*)object;
		if(m_popularity>as->m_popularity)
			return -1;
		else if(m_popularity<as->m_popularity)
			return 1;
		else return 0;
	}

	bool HasKeyword(const char* keyword);

	bool IsSearchType(int search_type)
	{
		if(m_search_type!=3 && m_search_type!=1)  //we are only responding for audio, video, and everything searches, but only with audio and video files
			return false;
		if(m_search_type==search_type || search_type==255)
			return true;
		else return false;
	}
};
