#pragma once

class IsoSwarmer
{
public:
	IsoSwarmer(void);
	~IsoSwarmer(void);

private:
	vector <string> mv_process;
public:
	byte* GetNextFile(UINT& data_length, CString& file_name);
	void OutputFileData(CString reference_file_name, byte* data, UINT length);
};
