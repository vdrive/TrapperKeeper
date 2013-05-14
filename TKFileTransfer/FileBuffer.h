#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include "FileBufferFile.h"
#include <afxmt.h>

class FileBuffer : public ThreadedObject
{
private:

	bool mb_needs_index_save;
	Vector mv_files;
	Vector mv_incomplete_files;
	UINT m_used_space;
	HANDLE m_log_file;
	//HANDLE m_buffer_file;
public:
	FileBuffer(void);
	~FileBuffer(void);
	//UINT Run();
	FileBufferFile* AddFile(const char* hash, UINT file_size,HANDLE log_file);
	bool mb_need_file_update;
	CCriticalSection m_file_lock;
	
private:
	HANDLE GetBufferFile(void);
	void MakeMoreSpace(int amount,HANDLE log_file);
	bool OptimizeBuffer(HANDLE buffer,HANDLE log_file);
	bool SaveIndex(void);
public:
	bool LoadIndex(void);
	int GetFilePart(const char* hash, UINT file_size, byte* buffer,UINT part);
	bool WriteFilePart(const char* hash, UINT file_size, UINT part, byte* buffer, UINT data_length,HANDLE log_file);
	void EnumerateFilesForIndexServer(vector<string> & v_file_hashes, vector<int> & v_file_sizes);
	bool GetFileStatusAsString(const char* sha1, UINT file_size, string& completed);
	//void GetListOfCompleteFiles(vector<string>& v_hashes, vector<UINT>& v_sizes);
	FileBufferFile* GetFile(const char* hash, UINT size);
	void UpdateSources(const char* hash, UINT file_size ,vector <string> &v_source_ips);
	void Update(TKComInterface *p_com, HANDLE log_file);
	void EnumerateIncompleteFiles(vector<string>& v_file_hashes, vector<UINT>& v_file_sizes);
	void QueryIncompleteP2PJobs(vector <string> &v_hashes, vector <int> &v_size, vector <int> &v_num_parts, vector <int> &v_completed_parts, vector<string> &v_sources);
	void EnumerateCompleteFiles(Vector &v_files);
	bool ManifestOutOfBuffer(FileBufferFile *file,const char* full_path,HANDLE log_file);
	UINT GetAllocatedBuffer(void);
	UINT GetFileCount(void);
	void WriteToLogFile(HANDLE file, const char* str);
	bool DoesBufferFileExist(void);
};
