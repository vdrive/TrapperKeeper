#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include <afxmt.h>

class AresDCProjectInfoCache : public ThreadedObject
{
	class TrackHashCollection : public Object{
	public:
		UINT m_track;
		vector<string> v_hashes;
		vector<UINT> v_amounts;
	};
private:
	void PrepareStringForDatabase(CString &str)
	{
		str.Replace("\\","\\\\");
		str.Replace("'","\\'");
	}


	string GetProjectTableName(const char* project_name)
	{
		int len=(int)strlen(project_name);
		string new_name;
		for(int i=0;i<len && i<60;i++){
			if(isdigit(project_name[i]) || isalpha(project_name[i])){
				new_name+=project_name[i];
			}
			else{
				new_name+="_";
			}
		}
		return new_name;
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

	vector <string> mv_exception_hashes;
	Vector mv_projects;
	CCriticalSection m_lock;
	bool mb_ready;
	int m_search_index;
public:
	AresDCProjectInfoCache(void);
	~AresDCProjectInfoCache(void);
	UINT Run();
	void GetProjects(Vector& v);
	bool IsReady(void);
	void GetNextProjectForSearch(Vector& v_tmp);
private:
	void ProcessSearchResults(void);
public:
	void BuildSwarms(void);
	void BuildExceptionHashes(void);
	void GetExceptionHashes(vector<string> &v_exception_hashes);
	void BuildDecoys(void);
	void CleanTables(void);
};
