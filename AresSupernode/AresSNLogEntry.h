#pragma once
#include "..\tkcom\Object.h"

class AresSNLogEntry : public Object
{
public:
	string m_log;
	AresSNLogEntry(const char* str,UINT entry_num);
	~AresSNLogEntry(void);
};
