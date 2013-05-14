#pragma once
#include "..\TKCom\Vector.h"

class Network : public Object
{
public:
	Vector mv_racks;
	string m_name;
	Network(const char* name);
	~Network(void);
	int GetTotalRacks(void);
	int GetAliveRacks(void);
	int GetCrashedRacks(void);
	bool HasRack(const char* ip);
};
