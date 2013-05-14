#pragma once
#include "..\TKCom\Object.h"

class DLLObject : public Object
{
public:
	string m_name;
	CTime m_created;
	DLLObject(const char* name);
	~DLLObject(void);

	inline bool IsExpired(){
		if(m_created<CTime::GetCurrentTime()-CTimeSpan(0,0,30,0))
			return true;
		else
			return false;
	}

	inline void Renew(){
		m_created=CTime::GetCurrentTime();
	}

	//a callback for sorting
	inline int CompareTo(Object *object){
		DLLObject *host=(DLLObject*)object;

		return stricmp(m_name.c_str(),host->m_name.c_str());
	}

};
