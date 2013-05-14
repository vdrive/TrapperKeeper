#pragma once
#include "..\tkcom\object.h"


class DataSendEvent : public Object
{
public:
	UINT m_amount;
	CTime m_time;
	DataSendEvent(UINT amount);
	~DataSendEvent(void);
};
