#pragma once
#include "..\tkcom\object.h"

class AresFTBanClient : public Object
{
public:
	string m_ip;    
	byte m_hash[20];
	CTime m_create_time;
	AresFTBanClient(const char* ip,byte *hash);
	~AresFTBanClient(void);
};
