#pragma once
#include "..\tkcom\vector.h"

class SNKeyword : public Object
{
public:
	string m_keyword;
	Vector mv_files;

	SNKeyword(const char* keyword);
	~SNKeyword(void);

	bool IsKeyword(const char* keyword){
		if(stricmp(m_keyword.c_str(),keyword)==0) 
			return true; 
		else return false;
	}

	inline int CompareTo(Object *object){
		SNKeyword *kw=(SNKeyword*)object;
		return stricmp(m_keyword.c_str(),kw->m_keyword.c_str());
	}

};
