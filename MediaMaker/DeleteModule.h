#pragma once
#include "FileAssociation.h"

class MediaManager;

class DeleteModule
{
public:
	DeleteModule(void);
	~DeleteModule(void);

	void InitParent(MediaManager * parent);

	bool DeleteNormalFile(char * path);
	bool DeleteMappedFile(char * filename);
	bool DeleteMappedFileList(vector<FileAssociation> vfa);
	void GetDirectoryList(char * folder, vector<string> * dirnames);
	bool DeleteExtraFolders();
	bool ClearAndRemoveDirectory(string directory);

private:
	MediaManager * p_parent;
};
