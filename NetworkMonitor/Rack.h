#pragma once
#include "..\TKCom\Vector.h"
#include "PingResponse.h"
#include "Network.h"
#include <afxmt.h>

class Rack : public Object
{
private:
	Vector mv_dlls;
	Vector mv_ping_responses;
public:
	string m_ip;
	
	bool mb_exist_check;  //a flag used by the network system to mark whether this ip showed up in the nameserver in the last check
	Rack(const char* ip);
	~Rack(void);
	void Update(void);
	
	CTime m_next_thin_time;
	string m_comment;
	Network* mp_owner_network;
	CTime m_next_dll_request;
	CTime m_next_process_request;
	CCriticalSection m_ping_vector_lock;
	CCriticalSection m_dll_vector_lock;

	//a callback for sorting
	inline int CompareTo(Object *object){
		Rack *host=(Rack*)object;

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

	void EnumerateRackHistory(int range,vector <bool> &vb_on);
	void EnumerateTKHistory(int range,vector <bool> &vb_on);
	void GotPingResponse(bool b_trapper_on);
	bool IsAlive(void);
	bool IsTKOn(void);
	bool SetDLLs(Vector& v_dlls);
	bool HasDll(const char* name);

	bool mb_restart;
	int m_version;
	void GetDLLs(Vector& v_dlls);
	void GetPingResponseVector(Vector& v_pings);
	void SavePingResponse(PingResponse* pr);
};
