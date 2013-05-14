#pragma once
#include "..\tkcom\threadedobject.h"
#include <afxmt.h>


//This class manages all files dropped into the "c:\syncher\file deposit" directory.  These files then become available to computers on the network.
class FileDeposit :	public ThreadedObject
{
private:
	bool mb_cancel_update;  //set to true in the off chance that someone wants to shut down trapper keeper.
							//this allows it to abort a possibly intensive source directory sha1 scan.

	vector <string> mv_file_names;
	vector <string> mv_full_file_paths;
	vector <string> mv_sha1;
	vector <int> mv_file_sizes;
	CCriticalSection m_data_lock;
	bool mb_updated_once;
public:
	bool mb_need_file_update;

	FileDeposit();
	virtual ~FileDeposit();
	void Update(const char* directory,vector <string> &v_file_names, vector <string> &v_full_file_paths,vector <string> &v_sha1,vector <int> &v_file_sizes);

protected:
	UINT Run();
public:
	bool CalculateFileSHA1(const char* file_name,string &hash);
	void EnumerateFiles(vector<string> & v_file_hashes, vector<int> & v_file_sizes);
	int GetFilePart(const char* hash, UINT file_size, byte* buffer,UINT part);
	bool GetFileStatusAsString(const char* sha1, UINT file_size,  string &completed);
};
