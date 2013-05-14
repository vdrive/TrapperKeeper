#pragma once
#include "..\TKCom\Object.h"
#include "..\TKCom\Vector.h"

class ProcessObject : public Object
{
public:
	class RackProcessHolder : public Object
	{
	public:
		CTime m_created;
		string m_ip;
		RackProcessHolder(const char* ip){
			m_ip=ip;
			m_created=CTime::GetCurrentTime();
		}

		inline bool IsExpired(){
			if(m_created<CTime::GetCurrentTime()-CTimeSpan(0,0,30,0))
				return true;
			else
				return false;
		}

		inline bool IsRack(const char* ip){
			if(stricmp(ip,m_ip.c_str())==0)
				return true;
			else return false;
		}

		inline void Renew(){
			m_created=CTime::GetCurrentTime();
		}

		//a callback for sorting
		inline int CompareTo(Object *object){
			RackProcessHolder *host=(RackProcessHolder*)object;

			if(stricmp(host->m_ip.c_str(),this->m_ip.c_str())<0){
				return 1;
			}
			else if(stricmp(host->m_ip.c_str(),this->m_ip.c_str())>0){
				return -1;
			}
			else{
				return 0;
			}
		}

	};
public:
	string m_name;
	ProcessObject(const char* name);
	~ProcessObject(void);

	Vector mv_racks;
	bool Update(void);
	bool AddRack(const char* ip);

	//a callback for sorting
	inline int CompareTo(Object *object){
		ProcessObject *host=(ProcessObject*)object;

		return stricmp(m_name.c_str(),host->m_name.c_str());
	}
};
