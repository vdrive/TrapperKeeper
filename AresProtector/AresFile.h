#pragma once
#include "..\tkcom\object.h"

class AresFile : public Object
{
private:
	void CalculateExtension();
public:
	bool mb_valid;
	string m_file_name;
	string m_meta_title;
	string m_meta_artist;
	string m_file_extension;

	byte m_hash[20];
	string m_project;
	unsigned int m_size;
	unsigned int m_media_length;
	unsigned int m_keywords_length;
	byte m_type;

	inline int CompareTo(Object *object){
		AresFile *af=(AresFile*)object;
		return -memcmp(m_hash,af->m_hash,20);
	}

	vector <string> mv_keyword_artist;
	vector <string> mv_keyword_title;
public:
	void Init(byte* hash,const char* artist, const char* title, const char* file_name, const char* project, unsigned int size, int media_length,bool b_trace,bool b_swarm);
	AresFile();
	AresFile(byte* hash,const char* artist, const char* title, const char* file_name, const char* project, unsigned int size, int media_length,bool b_trace,bool b_swarm);
	~AresFile(void);

private:
	void CreateKeywords(bool b_trace);
	string GetExtensionlessFileName(void);
public:
	void ExtractKeywords(const char* str,vector <string> &v_keywords,bool b_trace);
private:
	bool IsValidSpecialChar(char ch);
	void AddKeyword(vector <string>&v, string& str);
	
};
