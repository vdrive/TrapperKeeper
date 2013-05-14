#include "StdAfx.h"
#include "dllobject.h"

DLLObject::DLLObject(const char* name)
{
	m_name=name;
	m_created=CTime::GetCurrentTime();
}

DLLObject::~DLLObject(void)
{
}
