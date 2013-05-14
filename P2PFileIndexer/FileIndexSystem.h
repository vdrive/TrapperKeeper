#pragma once
#include "..\tkcom\vector.h"
#include "FileObject.h"
#include "FileHost.h"
#include <afxmt.h>

class FileIndexSystem
{
	//Vector mv_files;
	Vector mv_hosts;
public:
	FileIndexSystem(void);
	~FileIndexSystem(void);
	UINT GetHostsForFile(const char* hash, UINT size,vector<string> &v_ips);
	void UpdateHost(const char* ip, vector<string>& v_file_hashes, vector<int>& v_file_sizes);
	//FileObject* AddFile(const char* hash, UINT size);
	//bool FindFile(int& location,const char* hash,UINT size);
	void Update(void);
private:
	CCriticalSection m_lock;
	FileHost* GetHost(const char* ip);
};
