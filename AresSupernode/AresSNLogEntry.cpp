#include "StdAfx.h"
#include "aressnlogentry.h"

AresSNLogEntry::AresSNLogEntry(const char* str,UINT entry_num)
{
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	CString cur_time_str=cur_time.Format("%A %I:%M %p");
	if(entry_num>0)
		output.Format("[%u] %s:  %s\r\n",entry_num,cur_time_str,str);
	else
		output.Format("%s:  %s\r\n",cur_time_str,str);
	m_log=(LPCSTR)output;
}

AresSNLogEntry::~AresSNLogEntry(void)
{
}
