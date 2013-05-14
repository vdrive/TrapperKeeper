#pragma once
#include "..\tkcom\object.h"

class Spoof : public Object
{
public:
	Spoof(const char* file_name,int file_size);
	~Spoof(void);
	string m_file_name;
	int m_file_size;
};
