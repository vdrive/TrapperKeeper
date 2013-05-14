#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"

class FileObject :
	public Object
{
private:
	string m_hash;
	UINT m_size;
	//Vector mv_hosts;
public:
	FileObject(const char* hash, UINT size);
	~FileObject(void);
	bool IsFile(const char* hash, UINT size);
	inline UINT GetSize(){return m_size;}

	inline int CompareTo(Object *object){
		FileObject *file=(FileObject*)object;

		if(stricmp(file->m_hash.c_str(),this->m_hash.c_str())<0){
			return 1;
		}
		else if(stricmp(file->m_hash.c_str(),this->m_hash.c_str())>0){
			return -1;
		}
		else{
			return 0;
		}
	}

	const char* GetHash(void);
};
