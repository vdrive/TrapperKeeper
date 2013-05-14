#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"

class SearchJob : public Object
{
protected:
	inline bool IsValidSpecialChar(char ch)
	{
		if(ch!='.' && ch!='&' && ch!=',' && ch!='(' && ch!=')' && ch!='[' && ch!=']' && ch!='_' && ch!=' ' && ch!='.' && ch!='\'' && ch!='-'){
			return false;
		}
		else return true;
	}
	void AddKeyword(vector <string>&v, string& str)
	{
		if(str.size()==0)
			return;
		for(int i=0;i<(int)v.size();i++){  //check if it is already a keyword
			if(stricmp(v[i].c_str(),str.c_str())==0){
				return;
			}
		}

		v.push_back(str);
	}


	void ExtractKeywords(const char* str,vector <string> &v_keywords);

public:
	bool mb_valid;
	UINT m_search_id;
	UINT m_project_id;
	UINT m_size_threashold;
	string m_artist;
	string m_album;
	UINT m_track_num;
	string m_track_name;
	string m_search_str;
	vector<string> mv_search_keywords;
	vector<string> mv_killwords;

	vector<string> mv_supply_ips;
	vector<string> mv_supply_hashes;
	
	SearchJob(UINT proj_id,const char* artist, const char* album, UINT track_num, const char* track_name, const char* search_str,UINT size_threashold);
	~SearchJob(void);
	
	void AddKillword(const char* killword);
};
