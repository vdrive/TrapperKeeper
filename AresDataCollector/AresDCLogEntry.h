#pragma once
#include "..\tkcom\Object.h"


class AresDCLogEntry : public Object
{
public:
	string m_log;
	AresDCLogEntry(const char *str);
	~AresDCLogEntry(void);
};
