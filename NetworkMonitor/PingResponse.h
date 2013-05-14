#pragma once
#include "..\tkcom\Object.h"

class PingResponse : public Object
{
public:
	PingResponse(bool b_tk_on);
	~PingResponse(void);

public:
	CTime m_response_time;
	bool mb_tk_on;
};
