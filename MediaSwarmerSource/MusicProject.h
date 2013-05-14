#pragma once
#include "fileentry.h"


class MusicProject
{
public:
	MusicProject(void);
	~MusicProject(void);

	char projectname[256]; 
	int minswarms;
	int maxswarms;
	vector<FileEntry> files;
};
