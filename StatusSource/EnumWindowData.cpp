#include "StdAfx.h"
#include "enumwindowdata.h"

EnumWindowData::EnumWindowData(void)
{
	m_uploads = 0;
	m_hwnd = NULL;
	memset(&m_source,0,sizeof(m_source));
}

//
//
//
EnumWindowData::~EnumWindowData(void)
{
}
