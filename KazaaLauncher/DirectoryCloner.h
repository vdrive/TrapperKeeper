#pragma once

class DirectoryCloner
{
public:
	DirectoryCloner(void);
	~DirectoryCloner(void);
	void CloneDirectory(const char* source_directory, const char* dest_directory);

private:
	void CreateDirectoryStructure(const char* directory);
};
