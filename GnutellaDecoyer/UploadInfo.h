#pragma once

class UploadInfo
{
public:
	UploadInfo(void);
	~UploadInfo(void);

	void GetValueQuery(CString& query);  // get partial sql statment for insert.. 
	CString SQLFormat(const char* str); // get escaped string to insert

	CString m_filename;
	CString m_hash;
	CString m_ip;
	CTime m_timestamp;
};
