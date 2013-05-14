#pragma once

class SpoofStruct
{
public:
	SpoofStruct(void);
	~SpoofStruct(void); 

	CString ip;
	CString m_user;
	CString m_filename;
	int m_filesize;		
};
