#include "StdAfx.h"
#include "aresdclogentry.h"

AresDCLogEntry::AresDCLogEntry(const char *str)
{
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s:  %s\r\n",cur_time.Format("%A %I:%M %p"),str);
	m_log=(LPCSTR)output;
}

AresDCLogEntry::~AresDCLogEntry(void)
{
}
