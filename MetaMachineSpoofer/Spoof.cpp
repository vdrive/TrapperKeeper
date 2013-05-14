#include "StdAfx.h"
#include "spoof.h"

Spoof::Spoof(const char* file_name,int file_size)
{
	m_file_name=file_name;
	m_file_size=file_size;
}

Spoof::~Spoof(void)
{
}
