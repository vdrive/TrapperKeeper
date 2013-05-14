#pragma once
#include "fileentry.h"

class FileManager
{
public:
	FileManager(void);
	~FileManager(void);

	void ReadMapsFile();
	bool CompareHashWithMaps(FileEntry entry);
	void RemoveLeftoverFiles();
	void GetHash(char * outputfilename, char * hashin);
	void ResetMap();
	int GetEntryFromHash(char * hash, FileEntry * entry);
	int GetEntryFromHashAndRemove(char * hash, FileEntry * entry);
	int ExploreFolder(char * folder, vector<string>  *filenames);
	void GetHashFromDeflated(char * filename, char * hashin);

private:
	//vector<FileEntry> v_curfile_list;
	vector<FileEntry> v_old_file_list;
};
