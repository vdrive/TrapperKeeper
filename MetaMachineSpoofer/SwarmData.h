#pragma once
#include "..\tkcom\timer.h"
#include "..\tkcom\vector.h"

class SwarmData
{
private:
	bool mb_inited;
	Timer m_last_update;

public:
	SwarmData(void);
	~SwarmData(void);
	void Update(void);
	byte ConvertCharToInt(char ch);

	CCriticalSection m_data_lock;

	Vector mv_swarms;
	void Reset(void);
	void AddSwarm(const char* project, const char *hash, const char* hash_set,const char* file_name, UINT file_size);
};
