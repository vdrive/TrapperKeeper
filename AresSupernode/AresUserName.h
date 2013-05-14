#pragma once
#include "..\tkcom\object.h"

class AresUserName : public Object
{
public:
	AresUserName(const char* name);
	~AresUserName(void);

	string m_name;
};
