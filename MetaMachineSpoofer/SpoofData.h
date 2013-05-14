#pragma once
#include <afxmt.h>
#include "..\tkcom\Timer.h"
#include "..\tkcom\Vector.h"

class SpoofData
{
private:
	bool mb_inited;
	Timer m_last_update;

public:
	SpoofData(void);
	~SpoofData(void);

	CCriticalSection m_data_lock;

	Vector mv_spoofs;

	void Update(void);
};
