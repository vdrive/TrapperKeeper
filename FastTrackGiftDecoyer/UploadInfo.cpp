#include "StdAfx.h"
#include "uploadinfo.h"

UploadInfo::UploadInfo(void)
{
}

//
//
//
UploadInfo::~UploadInfo(void)
{
}

//
//
//
void UploadInfo::GetValueQuery(CString& query)
{
	query.AppendFormat( 
		"('%s','%s','%s')",
		SQLFormat(m_filename), m_hash,m_ip);
}

CString UploadInfo::SQLFormat(const char* in) 
{
	CString temp = in;
	temp.Replace("\\", "\\\\");
	temp.Replace("'", "\\'");
	temp.Replace("\"", "\\\"");
	return temp;
}