#pragma once
#include "..\tkcom\object.h"

class P2PSendFileJob :	public Object
{
private:
	string m_file_hash;
	UINT m_file_part;
	UINT m_file_size;
	string m_dest;
public:
	inline const char* GetHash(){return m_file_hash.c_str();}
	inline UINT GetFilePart(){return m_file_part;}
	inline UINT GetFileSize(){return m_file_size;}
	inline const char* GetDest(){return m_dest.c_str();}
	P2PSendFileJob(const char* hash, UINT file_size, const char* dest,UINT file_part);
	~P2PSendFileJob(void);
};
