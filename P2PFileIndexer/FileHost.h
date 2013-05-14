#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"
#include "Timer.h"
#include "FileObject.h"

class FileHost : public Object
{
private:
	string m_ip;
	Vector mv_files;
	Timer m_last_update;
public:
	FileHost(const char *ip);
	~FileHost(void);
	bool IsHost(const char* ip);
	void RemoveFile(FileObject* file);
	const char* GetIP(void);
	void UpdateFiles(vector<string>& v_file_hashes, vector<int>& v_file_sizes);
	bool FindFile(const char* hash,UINT &size);
	bool IsExpired(void);

	inline int CompareTo(Object *object){
		FileHost *host=(FileHost*)object;

		if(stricmp(host->m_ip.c_str(),this->m_ip.c_str())<0){
			return 1;
		}
		else if(stricmp(host->m_ip.c_str(),this->m_ip.c_str())>0){
			return -1;
		}
		else{
			return 0;
		}
	}

};
