#pragma once

class IsoSwarmer
{
public:
	IsoSwarmer(void);
	~IsoSwarmer(void);

private:
	vector <string> mv_extended_file_names;
	vector <string> mv_process;
	void Mirror(CString syncher_path , CString swarmer_path );
	void CleanDirectory(const char* directory_name);
public:
	byte* GetNextFile(UINT& data_length, CString& file_name);
	//void OutputFileData(CString reference_file_name, byte* data, UINT length);
private:
	void ScanInExtendedFileNames(void);
};

